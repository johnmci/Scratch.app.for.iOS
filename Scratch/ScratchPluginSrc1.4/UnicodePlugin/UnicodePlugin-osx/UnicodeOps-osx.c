#include <Carbon/Carbon.h>

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

/* globals */

ATSUStyle		g_style = NULL;
ATSUTextLayout	g_layout = NULL;
CGColorSpaceRef	g_cSpace = NULL;

int	gFontID = 0;
int	gFontSize = 18;
int	gBoldFlag = 0;
int	gItalicFlag = 0;
int	gAntialiasFlag = 1;

int	g_bgRed = 255, g_bgGreen = 255, g_bgBlue = 255;
int	g_fgRed = 0,   g_fgGreen = 0,   g_fgBlue = 0;
int	g_bgRGB = 0; // Squeak format
int	g_bgTransparent = 0;

#define MAX_FONTS 1000
ATSUFontID gfontIDList[MAX_FONTS];

#define UTF16_BUFSIZE 2000
unsigned short	g_utf16[UTF16_BUFSIZE];
int				g_utf16Length;

/* helper procedures */

static Boolean isBigEndian() {
	char buf[] = {0, 0, 0, 1};
	return *((int *) buf) == 1;
}

static ATSUTextLayout createLayoutAndStyle() {
	OSStatus		err = noErr;
	ATSUTextLayout	layout;
	unsigned short	tmp[2];

	err = ATSUCreateTextLayout(&layout);
	if (err != noErr) return NULL;
	err = ATSUCreateStyle(&g_style);
	if (err != noErr) return NULL;

	ATSUSetTextPointerLocation(layout, tmp, kATSUFromTextBeginning, kATSUToTextEnd, 0);
	ATSUSetTransientFontMatching(layout, true);

	g_cSpace = CGColorSpaceCreateDeviceRGB();
	gFontID = ATSFontFindFromName(CFSTR("Times Bold"), kATSOptionFlagsDefault);

	return layout;
}

static void setLayoutFont() {
	OSStatus				err;
	Fixed					fontSize;
	ATSUAttributeTag		tag;
	ByteCount				valueSize;
	ATSUAttributeValuePtr	valuePtr;

	// workaround: re-using the cached style sometimes failed to install
	// new font settings, so now we release and re-create it each time
	if (g_style != NULL) {
		CFRelease(g_style);
		g_style = NULL;
	}

	err = ATSUCreateStyle(&g_style);
	if (err != noErr) return;

	tag = kATSUFontTag;
	valueSize = sizeof(ATSUFontID);
	valuePtr = &gFontID;
	ATSUSetAttributes(g_style, 1, &tag, &valueSize, &valuePtr);

	fontSize = gFontSize << 16;
	tag = kATSUSizeTag;
	valueSize = sizeof(Fixed);
	valuePtr = &fontSize;
	ATSUSetAttributes(g_style, 1, &tag, &valueSize, &valuePtr);

	tag = kATSUQDBoldfaceTag;
	valueSize = sizeof(Boolean);
	valuePtr = &gBoldFlag;
	ATSUSetAttributes(g_style, 1, &tag, &valueSize, &valuePtr);

	tag = kATSUQDItalicTag;
	valueSize = sizeof(Boolean);
	valuePtr = &gItalicFlag;
	ATSUSetAttributes(g_style, 1, &tag, &valueSize, &valuePtr);

	ATSUSetRunStyle(g_layout, g_style, kATSUFromTextBeginning, kATSUToTextEnd);
}

static void setLayoutTag(ATSUTextLayout layout, ATSUAttributeTag tag, ByteCount size, ATSUAttributeValuePtr value) {
	ATSUSetLayoutControls(layout, 1, &tag, &size, &value);
}

static int utf8_to_utf16(unsigned char *utf8, int utf8Size) {
	int src = 0;
	int dst = 0;
	int n, n2, n3, n4;

	while (src < utf8Size) {
		if (dst >= UTF16_BUFSIZE) return -1; // error - string too long for utf16 buffer
		n = utf8[src++];
		if (n < 128) {
			g_utf16[dst++] = n;
			continue;
		}
		if (n < 0xE0) {
			if (src >= utf8Size) return dst; // ignore incomplete final utf8 character
			n2 = utf8[src++];
			g_utf16[dst++] = ((n & 0x1F) << 6) | (n2 & 0x3F);
			continue;
		}
		if (n < 0xF0) {
			if ((src + 1) >= utf8Size) return dst; // ignore incomplete final utf8 character
			n2 = utf8[src++];
			n3 = utf8[src++];
			g_utf16[dst++] = ((n & 0xF) << 12) | ((n2 & 0x3F) << 6) | (n3 & 0x3F);
			continue;
		}
		// four byte utf8 (requires two utf16 characters)
		if ((dst + 1) >= UTF16_BUFSIZE) return -1; // error - string too long for utf16 buffer
		if ((src + 2) >= utf8Size) return dst; // ignore incomplete final utf8 character
		n2 = utf8[src++];
		n3 = utf8[src++];
		n4 = utf8[src++];
		n = ((n & 0x7) << 18) | ((n2 & 0x3F) << 12) | ((n3 & 0x3F) << 6) | (n4 & 0x3F);
		n = n - 0x10000;
		g_utf16[dst++] = 0xD800 | ((n >> 10) & 0x3FF);
		g_utf16[dst++] = 0xDC00 | (n & 0x3FF);
	}
	return dst;
}

static int setText(char *utf8, int utf8Length) {
	if (!g_layout) g_layout = createLayoutAndStyle();
	if (!g_layout) return -1;

	g_utf16Length = utf8_to_utf16((unsigned char *) utf8, utf8Length);
	if (g_utf16Length <= 0) return -1;

	// point the text layout point to the text buffer
	ATSUSetTextPointerLocation(
		g_layout, g_utf16, kATSUFromTextBeginning, kATSUToTextEnd, g_utf16Length);
	setLayoutFont();

	return noErr;
}

static void measureTextRect(int *xOffsetPtr, int *yOffsetPtr, int *wPtr, int *hPtr) {
	Fixed	left, right, ascent, descent;
	Rect	rect;
	int		xOffset, yOffset, w, h;

	// compute the text rectangle and offset
	ATSUGetUnjustifiedBounds(g_layout, kATSUFromTextBeginning, kATSUToTextEnd, &left, &right, &ascent, &descent);
	ATSUMeasureTextImage(g_layout, kATSUFromTextBeginning, kATSUToTextEnd, 0, 0, &rect);
	xOffset = 0;
	if (rect.left < 0) xOffset = -rect.left;
	w = ((right - left) + 0xFFFF) >> 16;
	if ((rect.right - rect.left) > w) w = rect.right - rect.left;

	yOffset = (descent + 0xFFFF) >> 16;
	if (rect.bottom > yOffset) yOffset = rect.bottom;
	h = ((ascent + descent) + 0xFFFF) >> 16;
	if ((rect.bottom - rect.top) > h) h = rect.bottom - rect.top;

	*xOffsetPtr = xOffset;
	*yOffsetPtr = yOffset;
	*wPtr = w;
	*hPtr = h;
}

static int clipboardFindItemWithFlavor(PasteboardRef clipboard, CFStringRef desiredFlavor, PasteboardItemID *itemIDPtr) {
	OSStatus	err = noErr;
	ItemCount	itemCount, i;

	err = PasteboardGetItemCount(clipboard, &itemCount);
	if (err != noErr) return err;

	for (i = 1; i <= itemCount; i++) {
		PasteboardItemID	itemID;
		CFArrayRef			flavorTypes;
		CFIndex				flavorCount, j;

		err = PasteboardGetItemIdentifier(clipboard, i, &itemID);
		if (err != noErr) continue;

		err = PasteboardCopyItemFlavors(clipboard, itemID, &flavorTypes);
		if (err != noErr) continue;

		flavorCount = CFArrayGetCount(flavorTypes);
		for (j = 0; j < flavorCount; j++) {
			CFStringRef flavorType;

			flavorType = (CFStringRef) CFArrayGetValueAtIndex(flavorTypes, j);
			if (UTTypeConformsTo(flavorType, desiredFlavor)) {
				*itemIDPtr = itemID;
				CFRelease(flavorTypes);
				return noErr;
			}
		}
		CFRelease(flavorTypes);
	}
	return -1;
}

/* entry points */

int unicodeClipboardGet(unsigned short *utf16, int utf16Length) {
	PasteboardRef clipboard;
	OSStatus err = noErr;
	PasteboardItemID itemID;
	CFDataRef dataRef;
	unsigned short *src, *dst;
	int count, i;

	err = PasteboardCreate(kPasteboardClipboard, &clipboard);
	if (err != noErr) return 0;

	err = clipboardFindItemWithFlavor(clipboard, kUTTypeUTF16PlainText, &itemID);
	if (err != noErr) return 0;

	err = PasteboardCopyItemFlavorData(clipboard, itemID, kUTTypeUTF16PlainText, &dataRef);
	if (err != noErr) return 0;

	count = CFDataGetLength(dataRef) / 2;
	if (count > utf16Length) count = utf16Length;

	src = (unsigned short *) CFDataGetBytePtr(dataRef);
	dst = utf16;
	for (i = 0; i < count; i++) *dst++ = *src++;

	CFRelease(dataRef);
	return count;
}

void unicodeClipboardPut(unsigned short *utf16, int utf16Length) {
	PasteboardRef	clipboard;
	OSStatus		err;
	CFDataRef		dataRef;

	err = PasteboardCreate(kPasteboardClipboard, &clipboard);
	if (err != noErr) return;

	err = PasteboardClear(clipboard);
	if (err != noErr) return;

	dataRef = CFDataCreate(kCFAllocatorDefault, (UInt8 *) utf16, utf16Length * 2);
	if (dataRef == NULL) return;

	err = PasteboardPutItemFlavor(clipboard, (PasteboardItemID) 1, kUTTypeUTF16PlainText, dataRef, 0);
	CFRelease(dataRef);
}

int unicodeClipboardSize(void) {
	PasteboardRef	clipboard;
	OSStatus		err = noErr;
	CFDataRef		dataRef;

	PasteboardItemID itemID;
	int result = 0;

	err = PasteboardCreate(kPasteboardClipboard, &clipboard);
	if (err != noErr) return 0;

	err = clipboardFindItemWithFlavor(clipboard, kUTTypeUTF16PlainText, &itemID);
	if (err != noErr) return 0;

	err = PasteboardCopyItemFlavorData(clipboard, itemID, kUTTypeUTF16PlainText, &dataRef);
	if (err != noErr) return 0;

	result = CFDataGetLength(dataRef);
	CFRelease(dataRef);
	return result;
}

// this version of getFontList provides Unicode font names but does not seem to report all fonts
int unicodeGetFontListOLD(char *str, int strLength) {
	OSStatus err;
	ATSFontFamilyIterator iterator;
	ATSFontFamilyRef family;
	CFStringRef familyName;
	char cFamilyName[500];
	int src, dst = 0;
	int ok;

	err = ATSFontFamilyIteratorCreate(
		kATSFontContextLocal, NULL, NULL, kATSOptionFlagsUnRestrictedScope, &iterator);
	while (1) {
		err = ATSFontFamilyIteratorNext(iterator, &family);
		if (err != noErr) break;
		ATSFontFamilyGetName(family, kATSOptionFlagsDefault, &familyName);
		ok = CFStringGetCString(familyName, cFamilyName, sizeof(cFamilyName), kCFStringEncodingUTF8);
		src = 0;
		while ((dst < strLength) && (cFamilyName[src] != 0)) {
			str[dst++] = cFamilyName[src++];
		}
		if (dst < strLength) str[dst++] = '\n'; // new line
	}
	ATSFontFamilyIteratorRelease(&iterator);

	// return the size of font list string
	return dst;
}

int unicodeGetFontList(char *str, int strLength) {
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
}

void unicodeDrawString(char *utf8, int utf8Length, int *wPtr, int *hPtr, unsigned int *bitmapPtr) {
	CGContextRef	cgContext;
	int				w = *wPtr;
	int				h = *hPtr;
	int				pixelCount = w * h;
	int				xOffset = 0, yOffset = 0;
	unsigned int	*pixelPtr, *lastPtr;

	if (setText(utf8, utf8Length) != noErr) return;

	// create a Quartz graphics context
	cgContext = CGBitmapContextCreate(bitmapPtr, w, h, 8, w * 4, g_cSpace, kCGImageAlphaPremultipliedLast);
	if (cgContext == NULL) return;
	CGContextSetShouldAntialias(cgContext, gAntialiasFlag);
	CGContextClipToRect(cgContext, CGRectMake(0, 0, w, h));

	// connect the text layout to the graphic context
	setLayoutTag(g_layout, kATSUCGContextTag, sizeof(CGContextRef), &cgContext);

	// compute the text rectangle and offset
	measureTextRect(&xOffset, &yOffset, &w, &h);
	*wPtr = w;
	*hPtr = h;

	// Note about pixel formats:
	// Quartz stores a pixel as four bytes in memory, RGBA, independent of the endianness
	// Squeak treats a 32-bit pixel as a word with A in the high bits and R in the low bits
	// on little endian computers (Intel), a Squeak pixel is stored in memory as BGRA
	//   thus, on little endian computers, we just swap blue and red
	// on big endian computers (PPC), a Squeak pixel is stored in memory as ARGB
	//   thus, on big endian computers we convert to Squeak pixels by shifting right

	// fill the background
	if (isBigEndian()) {
		CGContextSetRGBFillColor(cgContext, g_bgRed / 255.0, g_bgGreen / 255.0, g_bgBlue / 255.0, 1);
	} else {
		CGContextSetRGBFillColor(cgContext, g_bgBlue / 255.0, g_bgGreen / 255.0, g_bgRed / 255.0, 1);
	}
	CGContextFillRect(cgContext, CGRectMake(0, 0, w, h));

	// draw the text
	if (isBigEndian()) {
		CGContextSetRGBFillColor(cgContext, g_fgRed / 255.0, g_fgGreen / 255.0, g_fgBlue / 255.0, 1);
	} else {
		CGContextSetRGBFillColor(cgContext, g_fgBlue / 255.0, g_fgGreen / 255.0, g_fgRed / 255.0, 1);
	}
	ATSUDrawText(g_layout, kATSUFromTextBeginning, kATSUToTextEnd, xOffset << 16, yOffset << 16);

	// convert pixels to Squeak format if necessary (see note above)
	if (isBigEndian()) {
		pixelPtr = bitmapPtr;
		lastPtr = pixelPtr + pixelCount;
		while (pixelPtr < lastPtr) {
			*pixelPtr++ = *pixelPtr >> 8;
		}
	}

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
}

void fixMulicharRanges(int *pairs, int length) {
	int i;

	for (i = 2; i < length; i += 2) {
		if (abs(pairs[i] - pairs[i + 1]) <= 1) {
			if (pairs[i - 2] < pairs[i]) {
				// left-to-right; make left edge of this pair = left edge of previous
				// and right edge of previous pair = right edge of this one
				pairs[i] = pairs[i - 2];
				pairs[(i - 2) + 1] = pairs[i + 1];
			} else {
				// right to left; make right edge of this pair = right edge of previous
				// and left edge of previous pair = left edge of this one
				pairs[i + 1] = pairs[(i - 2) + 1];
				pairs[(i - 2)] = pairs[i];
			}
		}
	}
}

int unicodeGetXRanges(char *utf8, int utf8Length, int *resultPtr, int resultLength) {
	ATSUCaret mainCaret;
	ATSUCaret secondaryCaret;
	Boolean isSplit;
	int *dst = resultPtr;
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
}

void unicodeMeasureString(char *utf8, int utf8Length, int *wPtr, int *hPtr) {
	int	ignore;

	*wPtr = *hPtr = 0;
	if (setText(utf8, utf8Length) != noErr) return;

	measureTextRect(&ignore, &ignore, wPtr, hPtr);
}

void unicodeSetColors(int fgRed, int fgGreen, int fgBlue, int bgRed, int bgGreen, int bgBlue, int mapBGToTransparent) {
	g_fgRed   = fgRed & 255;
	g_fgGreen = fgGreen & 255;
	g_fgBlue  = fgBlue & 255;
	g_bgRed   = bgRed & 255;
	g_bgGreen = bgGreen & 255;
	g_bgBlue  = bgBlue & 255;
	g_bgRGB = (g_bgRed << 16) | (g_bgGreen << 8) | g_bgBlue;  // Squeak pixel format
	if (!isBigEndian()) g_bgRGB |= 0xFF000000;  // add alpha on little-endian computer
	g_bgTransparent = mapBGToTransparent;
}

void unicodeSetFont(char *fontName, int fontSize, int boldFlag, int italicFlag, int antiAliasFlag) {
	CFStringRef cfFontName = CFStringCreateWithCString(NULL, fontName, kCFStringEncodingUTF8);
	if (cfFontName == NULL) return;
	gFontID = ATSFontFindFromName(cfFontName, kATSOptionFlagsDefault);
	CFRelease(cfFontName);
	gFontSize = fontSize;
	gBoldFlag = boldFlag;
	gItalicFlag = italicFlag;
	gAntialiasFlag = antiAliasFlag;
}
