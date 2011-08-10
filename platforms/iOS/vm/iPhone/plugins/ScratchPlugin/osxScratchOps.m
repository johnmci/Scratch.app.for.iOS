//  Created by John M McIntosh on 12/28/2010.
/*
 Copyright (c) 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
 */

#import <UIKit/UIKit.h>
#include "ScratchOps.h"

void OpenURL(char *url) {
	NSString *string = [[NSString alloc] initWithUTF8String: url];
	NSURL *theUrl = [NSURL URLWithString:string];
	[string release];
	[[UIApplication sharedApplication] openURL: theUrl];
	
}

void SetScratchWindowTitle(char *title) {
//
}

void GetFolderPathForID(int folderID, char *path, int maxPath) {
	int fType;
	path[0] = 0;  // zero-length return value indicates failure

	switch(folderID){
	case 1: fType = NSUserDirectory; break;
	case 2: fType = NSDocumentDirectory; break;
	case 3: fType = NSDocumentDirectory; break;
	case 4: fType = NSUserDirectory; break;
	case 5: fType = NSUserDirectory; break;
	default: return;
	}

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES); 
	NSString *directory = [paths objectAtIndex:0]; 
	strlcpy(path,[directory UTF8String],maxPath);
}

int WinShortToLongPath(char *shortPath, char* longPath, int maxPath) {
	return -1; // fail on Mac OS
}

int IsFileOrFolderHidden(char *fullPath) {
	return false;
}

void SetUnicodePasteBuffer(short int *utf16, int count) {
	NSData *data = [[NSData alloc] initWithBytes: utf16 length:count];
	[[UIPasteboard generalPasteboard] setData: data forPasteboardType: @"public.utf16-plain-text"];
	[data release];
							
}
