#import "SqueakNoOGLIPhoneAppDelegate.h"
#import "sqSqueakIPhoneApplication+clipboard.h"
#import "sqSqueakMainApplication.h"

extern SqueakNoOGLIPhoneAppDelegate *gDelegateApp;

/* entry points */

int unicodeClipboardGet(unsigned short *utf16, int utf16Length);
void unicodeClipboardPut(unsigned short *utf16, int utf16Length);
int unicodeClipboardSize(void);
void unicodeDrawString(char *utf8, int utf8Length, int *wPtr, int *hPtr, unsigned int *bitmapPtr);
int unicodeGetFontList(char *str, int strLength);
int unicodeGetXRanges(char *utf8, int utf8Length, int *resultPtr, int resultLength);
void unicodeMeasureString(char *utf8, int utf8Length, int *wPtr, int *hPtr);
void unicodeSetColors(int fgRed, int fgGreen, int fgBlue, int bgRed, int bgGreen, int bgBlue, int mapBGToTransparent);
void unicodeSetFont(char *fontName, int fontSize, int boldFlag, int italicFlag, int antiAliasFlag);
extern sqInt vmEndianness(void);

/* globals */
NSString *gFontName;
UIFont *gFontToUse;
CGFloat	gFontSize = 18;
int	gBoldFlag = 0;
int	gItalicFlag = 0;
int	gAntialiasFlag = 1;

int	g_bgRed = 255, g_bgGreen = 255, g_bgBlue = 255;
int	g_fgRed = 0,   g_fgGreen = 0,   g_fgBlue = 0;
int	g_bgRGB = 0; // Squeak format
int	g_bgTransparent = 0;

#define MAX_FONTS 1000

#define UTF16_BUFSIZE 2000
unsigned short	g_utf16[UTF16_BUFSIZE];
int				g_utf16Length;

extern SqueakNoOGLIPhoneAppDelegate *gDelegateApp;
/* helper procedures */

/* entry points */

int unicodeClipboardGet(unsigned short *utf16, int utf16Length) {
	NSAutoreleasePool * pool = [NSAutoreleasePool new];
	[((sqSqueakIPhoneApplication*)(SqueakNoOGLIPhoneAppDelegate *)gDelegateApp.squeakApplication) clipboardRead16: utf16Length into: (char *)utf16  startingAt: 0];
	[pool drain];
	return utf16Length;
}

void unicodeClipboardPut(unsigned short *utf16, int utf16Length) {
	NSAutoreleasePool * pool = [NSAutoreleasePool new];
	[((sqSqueakIPhoneApplication*)(SqueakNoOGLIPhoneAppDelegate *)gDelegateApp.squeakApplication) clipboardWrite16: utf16Length from: (char *) utf16  startingAt: 0];
	[pool drain];
}

int unicodeClipboardSize(void) {
	NSAutoreleasePool * pool = [NSAutoreleasePool new];
	sqInt value = [((sqSqueakIPhoneApplication*)(SqueakNoOGLIPhoneAppDelegate *)gDelegateApp.squeakApplication) clipboardSize16];
	[pool drain];
	return value;
}

int unicodeGetFontList(char *str, int strLength) {
/*	for (id setObject in <#setaccess#>)
	{
		<#!loopcontents#>
	}
	
	OSStatus err;
	ItemCount i, numFonts;
	int dst = 0;

	err = ATSUFontCount(&numFonts);
	if (err != noErr) return 0;

	if (numFonts > MAX_FONTS) numFonts = MAX_FONTS;

	err = ATSUGetFontIDs(gfontIDList, numFonts, NULL);
	if (err != noErr) return 0;

	for (i = 0; i < numFonts; i++) {
		char fontName[500];
		ByteCount fontNameLength;
		ItemCount oNameIndex;
		int src, ch;

		err = ATSUFindFontName(
			gfontIDList[i], kFontFullName,
			kFontNoPlatformCode, kFontNoScriptCode, kFontNoLanguageCode,
			sizeof(fontName), fontName, &fontNameLength, &oNameIndex);
		if (err == noErr) {
			// append font name
			src = 0;
			while ((dst < strLength) && (src < fontNameLength)) {
				ch = fontName[src++];
				if (ch != 0) str[dst++] = ch;
			}
			if (dst < strLength) str[dst++] = '\n'; // new line
		}
	}

	// return the size of font list string
	return dst;
 */
	return 0;
}

void unicodeDrawString(char *utf8, int utf8Length, int *wPtr, int *hPtr, unsigned int *bitmapPtr) {

	int				w = *wPtr;
	int				h = *hPtr;
	int				pixelCount = w * h;
	unsigned int	*pixelPtr, *lastPtr;
	
	
	// create a Quartz graphics context
	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
	CGContextRef cgContext = CGBitmapContextCreate(bitmapPtr, w, h, 8, w * 4, colorspace, kCGImageAlphaPremultipliedLast);
	CGColorSpaceRelease(colorspace);
	
	if (cgContext == NULL) 
		return;

	NSString *stringToDraw = [[NSString alloc] initWithBytes: utf8 length: utf8Length encoding:  NSUTF8StringEncoding];

	CGContextSetShouldAntialias(cgContext, gAntialiasFlag);
	CGContextClipToRect(cgContext, CGRectMake(0, 0, w, h));
	
	// Note about pixel formats:
	// Quartz stores a pixel as four bytes in memory, RGBA, independent of the endianness
	// Squeak treats a 32-bit pixel as a word with A in the high bits and R in the low bits
	// on little endian computers (Intel), a Squeak pixel is stored in memory as BGRA
	//   thus, on little endian computers, we just swap blue and red
	// on big endian computers (PPC), a Squeak pixel is stored in memory as ARGB
	//   thus, on big endian computers we convert to Squeak pixels by shifting right
	
	// fill the background
	CGContextSetRGBFillColor(cgContext, g_bgBlue / 255.0, g_bgGreen / 255.0, g_bgRed / 255.0, 1);
	CGContextFillRect(cgContext, CGRectMake(0, 0, w, h));
	CGContextSetRGBFillColor(cgContext, g_fgBlue / 255.0, g_fgGreen / 255.0, g_fgRed / 255.0, 1);
	
	// draw the text
	
	CGContextTranslateCTM(cgContext, 0.0, h);
	CGContextScaleCTM(cgContext, 1.0, -1.0); //NOTE: NSString draws in UIKit referential i.e. renders upside-down compared to CGBitmapContext referential
	UIGraphicsPushContext(cgContext);
	[stringToDraw drawAtPoint:CGPointMake(0.0,0.0) withFont: gFontToUse];
	UIGraphicsPopContext();	
		
	// map bg color pixels to transparent if so desired
	if (g_bgTransparent) {
		pixelPtr = bitmapPtr;
		lastPtr = pixelPtr + pixelCount;
		while (pixelPtr < lastPtr) {
			if (*pixelPtr == g_bgRGB) *pixelPtr = 0;
			pixelPtr++;
		}
	}
	
	CGContextRelease(cgContext);
	[stringToDraw release];
}

int unicodeGetXRanges(char *utf8, int utf8Length, int *resultPtr, int resultLength) {
	
	NSString *stringToMangle = [[NSString alloc] initWithBytes: utf8 length: utf8Length encoding:  NSUTF8StringEncoding];
	NSUInteger stringSize = [stringToMangle length];
	NSUInteger i;
	int *dst = resultPtr;
	int leftEdge, rightEdge,estimatedWidth;
	
	if (gFontToUse == nil) {
		gFontSize = 12.0f;
		gFontToUse = [[UIFont systemFontOfSize: gFontSize] retain];
		gFontName = [[gFontToUse fontName] retain];
	}

	leftEdge = 0;
	rightEdge = 0;
	for (i=0;i<stringSize;i++) {
		unichar c = [stringToMangle characterAtIndex: i];
		NSString *stringToDraw = [[NSString alloc] initWithCharacters: &c length: 1];
		CGSize size = [stringToDraw sizeWithFont: gFontToUse];
		[stringToDraw release];
		estimatedWidth = size.width + 0.5f;
		rightEdge += estimatedWidth;
		*dst++ = leftEdge;
		*dst++ = rightEdge;
		leftEdge += estimatedWidth;
	}
	[stringToMangle release];
	
/*	ATSUCaret mainCaret;
	ATSUCaret secondaryCaret;
	Boolean isSplit;
	int leftToRight, prev, thisEdge, otherEdge, i;

	if (setText(utf8, utf8Length) != noErr) return 0;
	if (resultLength < (2 * g_utf16Length)) return 0;

	ATSUOffsetToPosition(g_layout, 0, TRUE, &mainCaret, &secondaryCaret, &isSplit);
	prev = mainCaret.fX >> 16;
	leftToRight = (mainCaret.fX == 0);

	for (i = 1; i <= g_utf16Length; i++) {
		ATSUOffsetToPosition(g_layout, i, TRUE, &mainCaret, &secondaryCaret, &isSplit);
		thisEdge = mainCaret.fX >> 16;
		otherEdge = prev;
		prev = thisEdge;
		if (isSplit) {  // direction change
			if (leftToRight) {
				leftToRight = false;
				thisEdge = secondaryCaret.fX >> 16;
				prev = mainCaret.fX >> 16;
			} else {
				leftToRight = true;
				prev = secondaryCaret.fX >> 16;
			}
		}
//*dst++ = mainCaret.fX >> 16;
//*dst++ = secondaryCaret.fX >> 16;
		if (otherEdge <= thisEdge) {
			*dst++ = otherEdge;
			*dst++ = thisEdge;
		} else {
			*dst++ = thisEdge;
			*dst++ = otherEdge;
		}
	}

	fixMulicharRanges(resultPtr, (2 * g_utf16Length));
	return g_utf16Length;
 */
	return stringSize;
}

void unicodeMeasureString(char *utf8, int utf8Length, int *wPtr, int *hPtr) {
	NSString *stringToDraw = [[NSString alloc] initWithBytes: utf8 length: utf8Length encoding:  NSUTF8StringEncoding];
	if (gFontToUse == nil) {
		gFontSize = 12.0f;
		gFontToUse = [[UIFont systemFontOfSize: gFontSize] retain];
		gFontName = [[gFontToUse fontName] retain];
	}
		
	CGSize size = [stringToDraw sizeWithFont: gFontToUse];
	*wPtr = size.width + 0.5f;
	*hPtr = size.height + 0.5f;
	[stringToDraw release];
}

void unicodeSetColors(int fgRed, int fgGreen, int fgBlue, int bgRed, int bgGreen, int bgBlue, int mapBGToTransparent) {
	g_fgRed   = fgRed & 255;
	g_fgGreen = fgGreen & 255;
	g_fgBlue  = fgBlue & 255;
	g_bgRed   = bgRed & 255;
	g_bgGreen = bgGreen & 255;
	g_bgBlue  = bgBlue & 255;
	g_bgRGB = (g_bgRed << 16) | (g_bgGreen << 8) | g_bgBlue;  // Squeak pixel format
	if (!vmEndianness()) g_bgRGB |= 0xFF000000;  // add alpha on little-endian computer
	g_bgTransparent = mapBGToTransparent;
}

void unicodeSetFont(char *aFontName, int fontSize, int boldFlag, int italicFlag, int antiAliasFlag) {
	NSString *possibleFontName = [[NSString alloc ]initWithCString: aFontName encoding: NSUTF8StringEncoding];
	if ([possibleFontName isEqualToString: gFontName] && 
		gFontSize == fontSize &&
		gBoldFlag == boldFlag &&
		gItalicFlag == italicFlag &&
		gAntialiasFlag == antiAliasFlag) {
		[possibleFontName release];
		return;
	}
	NSAutoreleasePool * pool = [NSAutoreleasePool new];
	[gFontName release];
	gFontName = possibleFontName;
	gFontSize = fontSize;
	gBoldFlag = boldFlag;
	gItalicFlag = italicFlag;
	gAntialiasFlag = antiAliasFlag;
	NSString *boldNameString = boldFlag ? @"Bold" : @"";
	NSString *italicNameString = italicFlag ? @"Italic" : @"";
	NSString *dashString = (boldFlag | italicFlag) ? @"-" : @"";
	NSString *trimmedFontName = [gFontName stringByTrimmingCharactersInSet: [NSCharacterSet whitespaceCharacterSet]];
	NSString *fontActualName = [[[NSString alloc] initWithFormat: @"%@%@%@%@",trimmedFontName,dashString,boldNameString,italicNameString ] autorelease];
	[gFontToUse release];
	gFontToUse = [[UIFont fontWithName: fontActualName size: gFontSize] retain];
	if (gFontToUse == nil) 
		gFontToUse = [[UIFont fontWithName: trimmedFontName size: gFontSize] retain];
	[pool drain];
}
