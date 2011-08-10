#include "ScratchOps.h"

#define UNICODE 1  // use WCHAR API's

#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <shellapi.h>
#include <winuser.h>

#define true 1
#define false 0

/* globals */

#define UTF16_BUFSIZE 5000
WCHAR	g_wStr[UTF16_BUFSIZE];
int		g_wLength;
HWND	g_scratchWindow = NULL;

/* entry points */

void OpenURL(char *url) {
	g_wLength = MultiByteToWideChar(
		CP_UTF8, 0,
		url, -1,  // null terminated
		g_wStr, sizeof(g_wStr) / sizeof(WCHAR));
	if (g_wLength == 0) return;
	ShellExecute(NULL, TEXT("open"), g_wStr, NULL, NULL, SW_SHOWNORMAL);
}

void SetScratchWindowTitle(char *title) {
	g_wLength = MultiByteToWideChar(
		CP_UTF8, 0,
		title, -1,  // null terminated
		g_wStr, sizeof(g_wStr) / sizeof(WCHAR));
	if (g_wLength == 0) return;

	if (g_scratchWindow == NULL) {
		g_scratchWindow = GetActiveWindow();
		if (g_scratchWindow == NULL) return;
	}

	SetWindowText(g_scratchWindow, g_wStr);
}

// Constants were missing from the VS98 header files:
#define CSIDL_PROFILE		0x28
#define CSIDL_MYDOCUMENTS	0x05
#define CSIDL_MYPICTURES	0x27
#define CSIDL_MYMUSIC		0x0D

void GetFolderPathForID(int folderID, char *path, int maxPath) {
	int fType;
	HRESULT ok;

	path[0] = 0;  // zero-length return value indicates failure

	switch(folderID) {
	case 1: fType = CSIDL_PROFILE; break;
	case 2: fType = CSIDL_DESKTOP; break;
	case 3: fType = CSIDL_MYDOCUMENTS ; break;
	case 4: fType = CSIDL_MYPICTURES ; break;
	case 5: fType = CSIDL_MYMUSIC ; break;
	default: if (folderID > 0) return;
	}

	// the following line allows direct experimentation with
	// windows CSIDL's by passing a negative folderID
	if (folderID <= 0) fType = -folderID;

	ok = SHGetSpecialFolderPath(NULL, g_wStr, fType, false);
	if (!ok) return;
	
	WideCharToMultiByte(
		CP_UTF8, 0,
		g_wStr, -1, // null terminated
		path, maxPath,
		NULL, NULL);
}

int WinShortToLongPath(char *shortPath, char* longPath, int maxPath) {
	int result = GetLongPathNameA(shortPath, longPath, maxPath);  // use ANSI version
	if ((result >= maxPath) || (result == 0)) return -1;  // failed
	return 0;
}

int IsFileOrFolderHidden(char *fullPath) {
	DWORD attributes;

	g_wLength = MultiByteToWideChar(
		CP_UTF8, 0,
		fullPath, -1,  // null terminated
		g_wStr, sizeof(g_wStr) / sizeof(WCHAR));
	if (g_wLength == 0) return false;

	attributes = GetFileAttributes(g_wStr);
	if (attributes == -1) return false;
	return (attributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) != 0;
}

void SetUnicodePasteBuffer(short int *utf16, int count) {
	/* does nothing on Windows */
}
