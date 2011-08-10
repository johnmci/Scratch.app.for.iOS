/*
	osxCameraOps.c

	Support for the Squeak camera plugin under OSX using Quicktime. Portions of this
	code are taken from Apple's "Mungrab.c" in the SGDataProcSample sample code
	
	John Maloney, May 1, 2005

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include "sqCamera.h"

#define BailErr(x) {err = x; if(err != noErr) goto bail;}

typedef struct {
    GWorldPtr 		 	pGWorld;	// offscreen
    Rect 				bounds;		// bounds rect
    SeqGrabComponent 	seqGrab;	// sequence grabber
	SGChannel			vChannel;   // video channel
    ImageSequence 	 	decomSeq;   // unique identifier for our decompression sequence
	Boolean				gotAFrame;
} CameraRecord, *CameraPtr;

// globals
static CameraPtr gCamera = NULL;

void InitCamera(int w, int h);
pascal OSErr GrabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon);

int CameraOpen(int cameraNum, int desiredWidth, int desiredHeight) {
	if (!gCamera) InitCamera(desiredWidth, desiredHeight);
	return true;
}

void CameraClose(int cameraNum) {
	const BitMap *srcBits;

	if (!gCamera) return;  /* not open */

	if (gCamera->seqGrab != NULL) {
		SGStop(gCamera->seqGrab);
		if (gCamera->vChannel != NULL) {
			SGDisposeChannel(gCamera->seqGrab, gCamera->vChannel);
			gCamera->vChannel = NULL;
		}
		CloseComponent(gCamera->seqGrab);
		gCamera->seqGrab = NULL;
	}

	if (gCamera->pGWorld != NULL) {
		/* ZNote: should dispose of gworld's pixmap, but not sure where it gets allocated... */
		srcBits = *((BitMap **) GetGWorldPixMap(gCamera->pGWorld));

		DisposePtr(srcBits->baseAddr);
		DisposeGWorld(gCamera->pGWorld);
		gCamera->pGWorld = NULL;
	}

	DisposePtr((Ptr) gCamera);
	gCamera = NULL;
}

int CameraExtent(int cameraNum) {
	const BitMap *srcBits;
	int w, h;

	if (!gCamera) return 0;

	srcBits = *((BitMap **) GetGWorldPixMap(gCamera->pGWorld));
	w = srcBits->bounds.right - srcBits->bounds.left;
	h = srcBits->bounds.bottom - srcBits->bounds.top;
	return (w << 16) + h;
}

int CameraGetFrame(int cameraNum, unsigned char* buf, int pixelCount) {
	const BitMap *srcBits;
	long *ptr, *dst = (long *) buf;
	long *end = dst + pixelCount;
	long pix;
	int y, i;

	if (!gCamera || !gCamera->seqGrab) return false;

	gCamera->gotAFrame = false;
	SGIdle(gCamera->seqGrab);
	if (!gCamera->gotAFrame) return false;  /* no frame ready */

	srcBits = *((BitMap **) GetGWorldPixMap(gCamera->pGWorld));
	ptr = (long *) srcBits->baseAddr;

	int w = srcBits->bounds.right - srcBits->bounds.left;
	int h = srcBits->bounds.bottom - srcBits->bounds.top;
	int bytesPerRow = srcBits->rowBytes & 0x3FFF;

	for (y = 0; y < h; y++) {
		ptr = (long *) ((int) srcBits->baseAddr + (y * bytesPerRow));
		for (i = 0; i < w; i++) {
			if (dst == end) return true;
//xxx			*dst++ = *ptr++;
			pix = *ptr++;
			*dst++ = 0xFF000000 | ((pix << 8) & 0xFF0000) | ((pix >> 8) & 0xFF00) | ((pix >> 24) & 0xFF);
		}
	}
	return true;
}

char* CameraName(int cameraNum) {
	if (!gCamera) return "camera not open";
	return "default camera";
}

int CameraGetParam(int cameraNum, int paramNum) { return 0; }

void InitCamera(int w, int h) {
	Rect 				portRect = {0, 0, 0, 0};
    SeqGrabComponent	seqGrab = NULL;
    SGChannel			sgchanVideo = NULL;
    OSErr				err = noErr;

    // allocate memory for the camera data
    gCamera = (CameraPtr) NewPtrClear(sizeof(CameraRecord));
    if (MemError() || !gCamera) {
		gCamera = NULL;
		return;
	}

    // create a GWorld
	portRect.right = w;
	portRect.bottom = h;
    err = QTNewGWorld(
		&(gCamera->pGWorld),	// returned GWorld
		k32ARGBPixelFormat,		// pixel format
		&portRect,				// bounds
		0,						// color table
		NULL,					// GDHandle
		keepLocal);				// allocate pixmap in main memory
	BailErr(err);

    // lock the GWorld pixmap and verify that it gets locked because
    // we can't grab frames into an unlocked pixmap
	err = !LockPixels(GetGWorldPixMap(gCamera->pGWorld));
	BailErr(err);

	gCamera->bounds = portRect;

    // open the default sequence grabber
    seqGrab = OpenDefaultComponent(SeqGrabComponentType, 0);
	BailErr(!seqGrab);

	// initialize it
	err = SGInitialize(seqGrab);
	BailErr(err);

	// set its graphics world to the specified window
	err = SGSetGWorld(seqGrab, NULL, NULL);
	BailErr(err);

	// specify the destination data reference for a record operation
	// when the flag seqGrabDontMakeMovie is used the sequence grabber still
	// calls your data function, but it does not writedata to the movie file
	// writeType will always be set to seqGrabWriteAppend
	err = SGSetDataRef(seqGrab, 0, 0, seqGrabDontMakeMovie);
	BailErr(err);

    // create the channel
    err = SGNewChannel(seqGrab, VideoMediaType, &sgchanVideo);
	BailErr(err);
	
	// set the channel bounds
	err = SGSetChannelBounds(sgchanVideo, &portRect);
	BailErr(err);

	// set usage for new video channel to avoid playthrough
	// note we don't set seqGrabPlayDuringRecord
	err = SGSetChannelUsage(sgchanVideo, seqGrabRecord);
	BailErr(err);

    // specify a data function
	err = SGSetDataProc(seqGrab, NewSGDataUPP(GrabDataProc), 0);
	BailErr(err);

	// lights...camera...
	err = SGPrepare(seqGrab, false, true);
	BailErr(err);

    // ...action
	err = SGStartRecord(seqGrab);
	BailErr(err);

	gCamera->seqGrab = seqGrab;
	gCamera->vChannel = sgchanVideo;

bail:
    if (err) { // clean up on failure
		if (sgchanVideo != NULL) {
			SGDisposeChannel(seqGrab, sgchanVideo);
			sgchanVideo = NULL;
		}
		if (seqGrab != NULL) {
			SGStop(seqGrab);
			CloseComponent(seqGrab);
			seqGrab = NULL;
		}
		if (gCamera != NULL) {
			DisposePtr((Ptr) gCamera);
			gCamera = NULL;
		}
	}
}

/* ---------------------------------------------------------------------- */
/* sequence grabber data procedure - this is where the work is done		  */
/* ---------------------------------------------------------------------- */
/* GrabDataProc - the sequence grabber calls the data function whenever
   any of the grabberÕs channels write digitized data to the destination movie file.

   NOTE: We really mean any, if you have an audio and video channel then the DataProc will
   		 be called for either channel whenever data has been captured. Be sure to check which
   		 channel is being passed in. In this example we never create an audio channel so we know
   		 we're always dealing with video.

   This data function does two things, it first decompresses captured video
   data into an offscreen GWorld, draws some status information onto the frame then
   transfers the frame to an onscreen window.

   For more information refer to Inside Macintosh: QuickTime Components, page 5-120
   c - the channel component that is writing the digitized data.
   p - a pointer to the digitized data.
   len - the number of bytes of digitized data.
   offset - a pointer to a field that may specify where you are to write the digitized data,
   			and that is to receive a value indicating where you wrote the data.
   chRefCon - per channel reference constant specified using SGSetChannelRefCon.
   time	- the starting time of the data, in the channelÕs time scale.
   writeType - the type of write operation being performed.
   		seqGrabWriteAppend - Append new data.
   		seqGrabWriteReserve - Do not write data. Instead, reserve space for the amount of data
   							  specified in the len parameter.
   		seqGrabWriteFill - Write data into the location specified by offset. Used to fill the space
   						   previously reserved with seqGrabWriteReserve. The Sequence Grabber may
   						   call the DataProc several times to fill a single reserved location.
   refCon - the reference constant you specified when you assigned your data function to the sequence grabber.
*/
pascal OSErr GrabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
	CodecFlags	ignore;
    ComponentResult err = noErr;

	BailErr(!gCamera->pGWorld);

	if (gCamera->decomSeq == 0) {
		// set up the sequence grabber to save a frame in the GWorld; this is done only the first time

		Rect				   sourceRect = {0, 0, 0, 0};
		MatrixRecord		   scaleMatrix;
		ImageDescriptionHandle imageDesc = (ImageDescriptionHandle) NewHandle(0);

		// retrieve a channelÕs current sample description, the channel returns a sample description that is
		// appropriate to the type of data being captured
		err = SGGetChannelSampleDescription(c, (Handle) imageDesc);
		BailErr(err);

		// make a scaling matrix for the sequence
		sourceRect.right = (**imageDesc).width;
		sourceRect.bottom = (**imageDesc).height;
		RectMatrix(&scaleMatrix, &sourceRect, &gCamera->bounds);

		// begin the process of decompressing a sequence of frames
		// this is a set-up call and is only called once for the sequence - the ICM will interrogate different codecs
		// and construct a suitable decompression chain, as this is a time consuming process we don't want to do this
		// once per frame (eg. by using DecompressImage)
		// for more information see Ice Floe #8 http://developer.apple.com/quicktime/icefloe/dispatch008.html
		// the destination is specified as the GWorld
		err = DecompressSequenceBegin(
			&gCamera->decomSeq,   // pointer to field to receive unique ID for sequence
			imageDesc,				// handle to image description structure
			gCamera->pGWorld,		// port for the DESTINATION image
			NULL,					// graphics device handle, if port is set, set to NULL
			NULL,					// source rectangle defining the portion of the image to decompress
			&scaleMatrix,			// transformation matrix
			srcCopy,				// transfer mode specifier
			(RgnHandle) NULL,		// clipping region in dest. coordinate system to use as a mask
			0,						// flags
			codecNormalQuality, 	// accuracy in decompression
			bestSpeedCodec);		// compressor identifier or special identifiers ie. bestSpeedCodec

		DisposeHandle((Handle) imageDesc);
		BailErr(err);
	}

	// decompress a frame into the GWorld - can queue a frame for async decompression when passed in a completion proc
	err = DecompressSequenceFrameS(
		gCamera->decomSeq,	// sequence ID returned by DecompressSequenceBegin
		p,						// pointer to compressed image data
		len,					// size of the buffer
		0,						// in flags
		&ignore,				// out flags
		NULL);					// async completion proc
	gCamera->gotAFrame = true;

bail:
	return err;
}
