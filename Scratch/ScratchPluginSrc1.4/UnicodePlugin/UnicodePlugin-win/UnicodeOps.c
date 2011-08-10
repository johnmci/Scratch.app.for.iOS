#define UNICODE 1  // use WCHAR API's

#include <string.h>
#include <windows.h>
#include "usp10.h"

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

#define UTF16_BUFSIZE 500000
WCHAR	g_wStr[UTF16_BUFSIZE];
int		g_wLength;

HFONT g_font = NULL;
unsigned int g_fgColor = RGB(0, 0, 10);
unsigned int g_bgColor = RGB(255, 255, 255);
unsigned int g_bgRGB = 0xffffff;  // Squeak format
int g_bgTransparent = 0;

char *	g_fontString = NULL;
int		g_fontStringLength = 0;
int		g_fontStringIndex = 0;

/* helper procecures */

static int CALLBACK fontEnumCallback(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD fontType, LPARAM lParam) {
	unsigned short *utf16;
	int count;
	char utf8[1000];
	char *src, *dst;

	if (g_fontString == NULL) return 1;
	if (fontType != TRUETYPE_FONTTYPE) return 1;

	// convert to UTF8
	utf16 = (lpelfe->elfLogFont.lfFaceName[0]) ? lpelfe->elfLogFont.lfFaceName : lpelfe->elfFullName;
	count = WideCharToMultiByte(
		CP_UTF8, 0,
		utf16, -1, // null terminated
		utf8, sizeof(utf8),
		NULL, NULL);
	if (count == 0) return 1;

	// append the font name to g_fontString followed by a newline
	src = utf8;
	dst = &g_fontString[g_fontStringIndex];
	while ((g_fontStringIndex < g_fontStringLength) && (*src != 0)) {
		*dst++ = *src++;
		g_fontStringIndex++;
	}
	if (g_fontStringIndex < g_fontStringLength) g_fontString[g_fontStringIndex++] = '\n';

	return 1;
}

static SCRIPT_STRING_ANALYSIS analyze(HDC hdc, char *utf8, int utf8Length) {
	SCRIPT_STRING_ANALYSIS	ssa = NULL;
 	SCRIPT_CONTROL			scriptControl = {0};
	SCRIPT_STATE			scriptState   = {0};

	if (utf8Length == 0) return NULL;

	g_wLength = MultiByteToWideChar(
		CP_UTF8, 0,
		utf8, utf8Length,
		g_wStr, sizeof(g_wStr) / sizeof(WCHAR));
	if (g_wLength < 1) return NULL;

	if (g_font == NULL) unicodeSetFont("", 12, 0, 0, 0);
	if (g_font != NULL) SelectObject(hdc, g_font);

	ScriptStringAnalyse(
		hdc,
		g_wStr, g_wLength, (2 * g_wLength) + 16,
		-1,  // Unicode string
		SSA_GLYPHS | SSA_FALLBACK,
		0, // no clipping
		&scriptControl, &scriptState,
		0, 0, 0,
		&ssa);
	return ssa;
}

/* primitives */

void unicodeSetColors(int fgRed, int fgGreen, int fgBlue, int bgRed, int bgGreen, int bgBlue, int mapBGToTransparent) {
	g_fgColor = RGB(fgRed, fgGreen, fgBlue);
	g_bgColor = RGB(bgRed, bgGreen, bgBlue);
	g_bgRGB = (bgRed << 16) | (bgGreen << 8) | bgBlue;
	g_bgTransparent = mapBGToTransparent;
}

void unicodeSetFont(char *fontName, int fontSize, int boldFlag, int italicFlag, int antiAliasFlag) {
	g_wLength = MultiByteToWideChar(
		CP_UTF8, 0,
		fontName, -1,  // null terminated
		g_wStr, sizeof(g_wStr) / sizeof(WCHAR));
	if (g_wLength < 1) return;

	if (g_font != NULL) DeleteObject(g_font);
	g_font = CreateFont(
		-abs(fontSize), 0, 0, 0,
		(boldFlag ? FW_BOLD : FW_NORMAL),
		italicFlag, 0, 0,
		0, OUT_DEFAULT_PRECIS, 0,
		(antiAliasFlag ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY), 
		FF_DONTCARE,
		g_wStr);
}

void unicodeMeasureString(char *utf8, int utf8Length, int *wPtr, int *hPtr) {
	SCRIPT_STRING_ANALYSIS ssa;
	HDC				hdc;
	CONST SIZE		*pSize;

	if (utf8Length == 0) return;

	hdc = CreateCompatibleDC(0);
	ssa = analyze(hdc, utf8, utf8Length);
	if (ssa != NULL) {
		pSize = ScriptString_pSize(ssa);
		if (pSize != NULL) {
			*wPtr = pSize->cx;
			*hPtr = pSize->cy;
		}
		ScriptStringFree(&ssa);
	}
	DeleteDC(hdc);
}

void unicodeDrawString(char *utf8, int utf8Length, int *wPtr, int *hPtr, unsigned int *bitmapPtr) {
	SCRIPT_STRING_ANALYSIS ssa;
	HDC				hdc;
	BITMAPINFO		bi;
	HBITMAP			hBitmap;
	unsigned int	*dibBits;
	HGDIOBJ			oldObj;
	CONST SIZE		*pSize;
	int				w = *wPtr;
	int				h = *hPtr;
	unsigned int	*src, *dst, *end;

	*wPtr = *hPtr = 0;
	if (utf8Length == 0) return;

	hdc = CreateCompatibleDC(0);
	ssa = analyze(hdc, utf8, utf8Length);
	if (ssa == NULL) goto cleanup;

	// create a device independent bitmap
	bi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth         = w;
	bi.bmiHeader.biHeight        = -h;  // negative indicates top-down bitmap
	bi.bmiHeader.biPlanes        = 1;
	bi.bmiHeader.biBitCount      = 32;
	bi.bmiHeader.biCompression   = BI_RGB;
	bi.bmiHeader.biSizeImage     = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed       = 0;
	bi.bmiHeader.biClrImportant  = 0;
	hBitmap = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &dibBits, NULL, 0);
	if (hBitmap == NULL) goto cleanup;

	// attached the bitmap to the context
	oldObj = SelectObject(hdc, hBitmap);
	if (oldObj != NULL) {
		// set fg and bg colors and render the string
		SetBkColor(hdc, g_bgColor);
		SetTextColor(hdc, g_fgColor);
		ScriptStringOut(ssa, 0, 0, 0, NULL, 0, 0, FALSE);

		pSize = ScriptString_pSize(ssa);
		if (pSize != NULL) {
			*wPtr = pSize->cx;
			*hPtr = pSize->cy;
		}

		// copy pixels into Squeak's bitmap
		src = dibBits;
		dst = (int *) bitmapPtr;
		end = dst + (w * h);
		if (g_bgTransparent && (g_bgRGB != 0)) {
			// if g_bgTransparent was set, map g_bgRGB to 0 (transparent)
			while (dst < end) {
				*dst++ = (*src == g_bgRGB) ? 0 : *src;
				src++;
			}
		} else {
			while (dst < end) *dst++ = *src++;
		}
		SelectObject(hdc, oldObj);
	}

	ScriptStringFree(&ssa);
	DeleteObject(hBitmap);

cleanup:
	DeleteDC(hdc);
}

int unicodeGetXRanges(char *utf8, int utf8Length, int *resultPtr, int resultLength) {
	SCRIPT_STRING_ANALYSIS ssa;
	HDC				hdc;
	int				i, pX;
	HRESULT			r;

	if (utf8Length == 0) return 0;

	g_wLength = 0;
	hdc = CreateCompatibleDC(0);
	ssa = analyze(hdc, utf8, utf8Length);
	if (ssa != NULL) {
		for (i = 0; i < g_wLength; i++) {
			r = ScriptStringCPtoX(ssa, i, FALSE, &pX);
			if (FAILED(r)) pX = -1;
			resultPtr[2 * i] = pX;
			r = ScriptStringCPtoX(ssa, i, TRUE, &pX);
			if (FAILED(r)) pX = -1;
			resultPtr[(2 * i) + 1] = pX;
		}
		ScriptStringFree(&ssa);
	}
	DeleteDC(hdc);
	return g_wLength;
}

/* not yet implemented */

int unicodeClipboardGet(unsigned short *utf16, int utf16Length) {
	HANDLE h = NULL;
	WCHAR *src, *dst;
	int count = 0;

	if (!IsClipboardFormatAvailable(CF_TEXT)) return 0;
	if (!OpenClipboard(NULL)) return 0;

	h = GetClipboardData(CF_UNICODETEXT);
	if (h != NULL) {
		src = GlobalLock(h);
		if (src != NULL) {
			dst = utf16;
			while ((*src != 0) && (count < utf16Length)) {
				*dst++ = *src++;
				count++;
			}
		}
		GlobalUnlock(h);
	}
	CloseClipboard();
	return count;
}

void unicodeClipboardPut(unsigned short *utf16, int utf16Length) {
	HANDLE h;
	WCHAR *src, *dst;
	int count = 0;

	// Note: be sure to handle empty string case
	if (!OpenClipboard(NULL)) return;

	EmptyClipboard();
	if (utf16Length == 0) goto cleanup;
	
	h = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (utf16Length + 1) * sizeof(WCHAR));
	if (h == NULL) goto cleanup;

	src = utf16;
	dst = GlobalLock(h);
	while (count < utf16Length) {
		*dst++ = *src++;
		count++;
	}
	*dst = 0; // terminator

	GlobalUnlock(h);
	SetClipboardData(CF_UNICODETEXT, h);

cleanup:
	CloseClipboard();
}

int unicodeClipboardSize(void) {
	HANDLE h = NULL;
	WCHAR *src;
	int count = 0;

	if (!IsClipboardFormatAvailable(CF_TEXT)) return 0;
	if (!OpenClipboard(NULL)) return 0;

	h = GetClipboardData(CF_UNICODETEXT);
	if (h != NULL) {
		src = GlobalLock(h);
		if (src != NULL) {
			while (*src++ != 0) count++;
		}
		GlobalUnlock(h);
	}
	CloseClipboard();
	return count;
}

int unicodeGetFontList(char *str, int strLength) {
	HDC hdc;
	LOGFONT fontSpec;

	g_fontString = str;
	g_fontStringLength = strLength;
	g_fontStringIndex = 0;

	hdc = CreateCompatibleDC(0);

	memset(&fontSpec, 0, sizeof(fontSpec));
	fontSpec.lfCharSet = DEFAULT_CHARSET;
	fontSpec.lfFaceName[0] = 0; 
	EnumFontFamiliesEx(hdc, &fontSpec, (FONTENUMPROC) fontEnumCallback, 0, 0);

	g_fontString = NULL;
	g_fontStringLength = 0;

	DeleteDC(hdc);
	return g_fontStringIndex;
}
