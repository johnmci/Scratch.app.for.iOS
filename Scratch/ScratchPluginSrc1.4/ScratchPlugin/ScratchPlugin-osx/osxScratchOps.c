#include "ScratchOps.h"

void OpenURL(char *url) {
	CFStringRef urlString = CFStringCreateWithCString(kCFAllocatorDefault, url, kCFStringEncodingUTF8);
	CFURLRef urlRef = CFURLCreateWithString(kCFAllocatorDefault, urlString, NULL);

	if (urlRef != NULL) {
		LSOpenCFURLRef(urlRef, NULL);
		CFRelease (urlRef);
	}
	CFRelease(urlString);
}

void SetScratchWindowTitle(char *title) {
	CFStringRef str = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);
	SetWindowTitleWithCFString(FrontWindow(), str);
	CFRelease(str);
}

void GetFolderPathForID(int folderID, char *path, int maxPath) {
	int fType;
	path[0] = 0;  // zero-length return value indicates failure

	switch(folderID){
	case 1: fType = kCurrentUserFolderType; break;
	case 2: fType = kDesktopFolderType; break;
	case 3: fType = kDocumentsFolderType; break;
	case 4: fType = kPictureDocumentsFolderType; break;
	case 5: fType = kMusicDocumentsFolderType; break;
	default: return;
	}

	FSRef fsRef;
	int err = FSFindFolder(kUserDomain, fType, false, &fsRef);
	if (err == 0) err = FSRefMakePath(&fsRef, (UInt8 *) path, maxPath);
}

int WinShortToLongPath(char *shortPath, char* longPath, int maxPath) {
	return -1; // fail on Mac OS
}

int IsFileOrFolderHidden(char *fullPath) {
	return false;
}

void SetUnicodePasteBuffer(short int *utf16, int count) {
	ScrapRef scrap;
	GetCurrentScrap(&scrap);
	ClearScrap(&scrap);
	PutScrapFlavor(scrap, kScrapFlavorTypeUnicode, kScrapFlavorMaskNone, count, utf16);
}
