#include "scratchOps.h"
#include <stdlib.h>
#include <string.h>

void OpenURL(char *url) {
	// Open a browser on thet given URL. Not yet implemented.
}

void SetScratchWindowTitle(char *title) {
	// Set the text in the window title bar. Not yet implemented.
}

void GetFolderPathForID(int folderID, char *path, int maxPath) {
  // Get the full path for a special folder:
	//  1 - user's home folder
	//  2 - user's desktop folder
	//  3 - user's document folder
	//  4 - user's photos or pictures folder (does Linux have a convention for this?)
	//  5 - user's music folder (does Linux have a convention for this?)
	// path is filled in with a zero-terminated string of max length maxPath

	char *s = NULL;

	path[0] = 0;  // a zero-length path indicates failure
	
	// get the user's HOME directory
	s = getenv("HOME");
	if ((s == NULL) || (strlen(s) == 0)) return;

	strncat(path, s, maxPath); // home folder

	if (folderID == 1) return;
	if (folderID == 2) strncat(path, "/Desktop", maxPath);
	if (folderID == 4) strncat(path, "/Pictures", maxPath);
	if (folderID == 5) strncat(path, "/Music", maxPath);
	
	if (folderID == 3) {
		s = getenv("SUGAR_ACTIVITY_ROOT");
		if (s != NULL) {
			// On XO, return the writeable activity "data" directory
			strncat(path, s, maxPath);
			strncat(path, "/data", maxPath);
		} else  {
			strncat(path, "/Documents", maxPath);
		}
	}
}

int WinShortToLongPath(char *shortPath, char* longPath, int maxPath) {
	return -1; // fail on non-Windows platforms
}

int IsFileOrFolderHidden(char *fullPath) {
	// Always return false on Linux
	return 0;
}

void SetUnicodePasteBuffer(short int *utf16, int count) {
	// Store the given Unicode UTF16 string in the paste buffer.
	// No longer needed; use clipboard methods in UnicodePlugin.
}
