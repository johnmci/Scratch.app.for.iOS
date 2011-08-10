//
//  ScratchIPhoneAppDelegate.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-14.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//
#import "SqueakNoOGLIPhoneAppDelegate.h"
#import "ScratchIPhonePresentationSpace.h"
#import	"squeakProxy.h"
#import <MessageUI/MFMailComposeViewController.h>
#import "CSCWebsiteViewController.h"

@interface ScratchIPhoneAppDelegate : SqueakNoOGLIPhoneAppDelegate <UIAlertViewDelegate,MFMailComposeViewControllerDelegate> {
	BOOL loginAutomatically;
	BOOL canMakeTelephoneCalls;
	NSString *userId;
	NSString *password;
	SqueakProxy *squeakProxy;
	ScratchIPhonePresentationSpace* presentationSpace;
	IBOutlet UIImageView *imageView;
	BOOL	loginToScratchCompleted;
	NSString *brokenWalkBackString;
	CSCWebsiteViewController* webSite;
	BOOL	sizeOfMemoryIsTooLow;
	BOOL	outOfMemory;
	BOOL	squeakVMIsReady;
}
@property (nonatomic, retain) NSString *userId;
@property (nonatomic, retain) NSString *password;
@property (nonatomic, retain) NSString *brokenWalkBackString;
@property (nonatomic, retain) SqueakProxy *squeakProxy;
@property (nonatomic, retain) IBOutlet UIImageView *imageView;
@property (nonatomic, retain) ScratchIPhonePresentationSpace* presentationSpace;
@property (nonatomic, retain) CSCWebsiteViewController* webSite;
@property (nonatomic, assign) BOOL loginAutomatically;
@property (nonatomic, assign) BOOL canMakeTelephoneCalls;
@property (nonatomic, assign) BOOL loginToScratchCompleted;
@property (nonatomic, assign) BOOL squeakVMIsReady;

- (void) bailWeAreBroken: (NSString *) oopsText;
- (void) doEmail;
- (void) restartWeHope;
- (void) restartWeHope2;
- (NSData *) dumpConsoleLog;
- (BOOL) sizeOfMemoryIsTooLowForLargeImages;
- (NSInteger) sizeOfMemory;

@end
