//
//  ScratchIPhoneAppDelegate.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-14.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "ScratchIPhoneAppDelegate.h"
#import "LoginViewControlleriPad.h"
#import "sqSqueakIPhoneInfoPlistInterface.h"
#import "ScratchIPhonePresentationSpace.h"
#import "sqScratchIPhoneApplication.h"
#import "LFCGzipUtility.h"
#import <asl.h>
#import <mach/mach_host.h>
#import <sys/sysctl.h>


@implementation ScratchIPhoneAppDelegate
@synthesize	loginAutomatically,userId,password,canMakeTelephoneCalls,squeakProxy,imageView,
presentationSpace,loginToScratchCompleted,brokenWalkBackString,webSite,squeakVMIsReady;

- (sqSqueakMainApplication *) makeApplicationInstance {
	return [sqScratchIPhoneApplication new];
}

- (void)applicationDidFinishLaunching:(UIApplication *)application  {
	sizeOfMemoryIsTooLow = [self sizeOfMemory] < 128*1024*1024;
	outOfMemory = NO;
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(squeakVMIsReadyNow) name:@"squeakVMIsReadyNow" object:nil];
	[super applicationDidFinishLaunching: application];
	self.canMakeTelephoneCalls = [[UIApplication sharedApplication] canOpenURL: [NSURL URLWithString: @"tel://0"]];
	LoginViewController *loginViewController;
	if( UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM() ) {
		Class loginViewControlleriPadClass = NSClassFromString(@"LoginViewControlleriPad");
		loginViewController = [[loginViewControlleriPadClass alloc] initWithNibName:@"LoginViewControlleriPad" bundle:[NSBundle mainBundle]];
	} else {
		loginViewController = [[LoginViewController alloc] initWithNibName:@"LoginViewController" bundle:[NSBundle mainBundle]];
	}
	viewController = [[UINavigationController alloc] initWithRootViewController: loginViewController];
	[loginViewController release];
	
	self.viewController.navigationBarHidden = NO;		
	[window addSubview: self.viewController.view];
	[window makeKeyAndVisible];	
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (outOfMemory) {
		outOfMemory = NO;
		[self restartWeHope];
		return;
	}
	
	if (buttonIndex) {
		[self doEmail];
		return;
	}
	[self restartWeHope];
}

- (NSInteger) sizeOfMemory {
	size_t length;
	int mib[6];	
	NSInteger result;
	mib[0] = CTL_HW;
	mib[1] = HW_PHYSMEM;
	length = sizeof(result);
	if (sysctl(mib, 2, &result, &length, NULL, 0) < 0)
	{
		return 128*1024*1024+1;
	}
	return result;
}

- (BOOL) sizeOfMemoryIsTooLowForLargeImages {
	return sizeOfMemoryIsTooLow;
}

- (void) squeakVMIsReadyNow {
	self.squeakVMIsReady = YES;
}

- (void) bailWeAreBrokenOnMainThread: (NSString *) oopsText {
	self.brokenWalkBackString = oopsText;
	[self.presentationSpace deletePendingTimer];
	[self terminateActivityView];
	UIAlertView *alertView = [UIAlertView alloc];
	NSString *cough = NSLocalizedString(@"Cough",nil);
	NSString *massive = NSLocalizedString(@"Massive",nil);
	NSString *reset = NSLocalizedString(@"Reset",nil);
	NSString *email = NSLocalizedString(@"Email",nil);
	if ([MFMailComposeViewController canSendMail])
		alertView = [alertView initWithTitle: cough message: massive delegate: self cancelButtonTitle: reset otherButtonTitles: email,nil];
	else 
		alertView = [alertView initWithTitle: cough message: massive delegate: self cancelButtonTitle: reset otherButtonTitles: nil];
	[alertView show];
	[alertView release];
}	

- (void) bailWeAreBroken: (NSString *) oopsText {
	[self performSelectorOnMainThread:@selector(bailWeAreBrokenOnMainThread:) withObject: oopsText waitUntilDone: NO];
}

- (void) bailWeAreOutOfMemoryOnMainThread {
	[self.presentationSpace deletePendingTimer];
	[self terminateActivityView];
	outOfMemory = YES;
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	if(standardUserDefaults) {
		self.presentationSpace.pathToProjectFile = nil;
		[standardUserDefaults removeObjectForKey: @"pathToProjectFile"];
		[standardUserDefaults synchronize];
	}	
	UIAlertView *alertView = [UIAlertView alloc];
	NSString *cough = NSLocalizedString(@"Cough",nil);
	NSString *kilogram = NSLocalizedString(@"Kilogram",nil);
	NSString *reset = NSLocalizedString(@"Reset",nil);
	alertView= [alertView initWithTitle: cough message: kilogram delegate: self cancelButtonTitle: reset otherButtonTitles: nil];
	[alertView show];
	[alertView release];
}	

- (void) bailWeAreOutOfMemory {
	[self performSelectorOnMainThread:@selector(bailWeAreOutOfMemoryOnMainThread) withObject: nil waitUntilDone: NO];
}


#pragma mark -
#pragma mark Mail
- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)aError {
	
	UIAlertView *mailResult = [[UIAlertView alloc] init];
	
	NSString *ok = NSLocalizedString(@"Ok",nil);
	switch(result) {
		case MFMailComposeResultSaved: {
			NSString *saved = NSLocalizedString(@"Saved",nil);
			NSString *savedMsg = NSLocalizedString(@"SavedMsg",nil);
 			mailResult.title = saved;
			mailResult.message = savedMsg;
			int indexOfButton = [mailResult addButtonWithTitle: ok];
			[mailResult dismissWithClickedButtonIndex:indexOfButton animated:YES];			
			[self.viewController  dismissModalViewControllerAnimated:YES];			
			[mailResult show];
			[mailResult release];
			break;
		}
		case MFMailComposeResultFailed: {
			NSString *failed = NSLocalizedString(@"Failed",nil);
			NSString *failedMsg = NSLocalizedString(@"FailedMsg",nil);
			mailResult.title = failed;
			mailResult.message = failedMsg;
			int indexOfButton = [mailResult addButtonWithTitle: ok];
			[mailResult dismissWithClickedButtonIndex:indexOfButton animated:YES];			
			[self.viewController dismissModalViewControllerAnimated:YES];			
			[mailResult show];
			[mailResult release];
			break;
		}
		default: 
			[self.viewController dismissModalViewControllerAnimated:YES];
			[mailResult release];			
	}
	[self restartWeHope];
}

- (void) doEmail {
	if (!([MFMailComposeViewController canSendMail]))
		return;
#error you must alter the localization data to supply a valid email address
	NSString *helpEmailAddress = NSLocalizedString(@"helpEmailAddress",nil);
	NSString *helpSubject = NSLocalizedString(@"helpSubject",nil);
	NSString *helpMessage = NSLocalizedString(@"helpMessage",nil);
	MFMailComposeViewController *emailController = [[MFMailComposeViewController alloc] init];
	emailController.mailComposeDelegate = self;
	[emailController setToRecipients: [NSArray arrayWithObject: helpEmailAddress]];
	[emailController setSubject: helpSubject];
	[emailController setMessageBody: helpMessage isHTML: NO];
	NSData *data = [self.brokenWalkBackString dataUsingEncoding: NSUTF8StringEncoding];
	NSData *datazipped = [LFCGzipUtility gzipData: data];
	[emailController addAttachmentData: datazipped mimeType: @"application/octet-stream" fileName: @"DiagnosticsInformationWB.bin"];
	NSData *data2 = [self dumpConsoleLog];
	NSData *datazipped2 = [LFCGzipUtility gzipData: data2];
	[emailController addAttachmentData: datazipped2 mimeType: @"application/octet-stream" fileName: @"DiagnosticsInformationLog.bin"];
	[self.viewController presentModalViewController: emailController animated:YES];
	[emailController release];
}


- (void) restartWeHope {
	[self.squeakThread cancel];
	[self performSelector: @selector(restartWeHope2) withObject: nil afterDelay: 0.5];
}

- (void) restartWeHope2 {
	
	while (![self.squeakThread isFinished]) {
	}
	extern int sqMacMemoryFree();
	
	sqMacMemoryFree();
	self.squeakThread = nil;
	[viewController popToRootViewControllerAnimated: NO];
	[self.presentationSpace.view removeFromSuperview];
	[[viewController view] removeFromSuperview];
	self.mainView = nil;
	self.scrollView = nil;
	self.viewController = nil;
	self.squeakApplication = nil;
	self.userId = nil;
	self.password = nil;
	self.webSite = nil;
	self.loginToScratchCompleted = NO;
	self.squeakProxy  = nil;
	self.presentationSpace  = nil;
	if (self.screenAndWindow.blip) {
		[self.screenAndWindow.blip invalidate];
		self.screenAndWindow.blip = nil;
	}
	self.screenAndWindow  = nil;
	self.brokenWalkBackString = nil;
	self.squeakVMIsReady = NO;
	[self performSelector: @selector(applicationDidFinishLaunching:) withObject: [UIApplication sharedApplication] afterDelay: 1.0];
}

- (NSData *) dumpConsoleLog {
	NSMutableString *dumpString = [[NSMutableString alloc] init];
	
	//Build a query message containing all our criteria.
	aslmsg query = asl_new(ASL_TYPE_QUERY);
	//Specify one or more criteria with calls to asl_set_query.
	asl_set_query(query, ASL_KEY_MSG, "com.smalltalkconsulting.scratch", ASL_QUERY_OP_EQUAL);
	
	//Begin the search.
	aslresponse response = asl_search(NULL,query);
	
	//We don't need this anymore.
	asl_free(query);
	
	aslmsg msg;
	while ((msg = aslresponse_next(response))) {
		//Do something with the message. For example, to iterate all its key-value pairs:
		const char *key;
		for (unsigned i = 0U; (key = asl_key(msg, i)); ++i) {
			const char *value = asl_get(msg, key);
			
			//Example: Print the key-value pair to stdout, with a tab between the key and the value.
			if (strcmp(key,"Message") == 0) {
				[dumpString appendFormat: @"%s\n",value];
			}
		}
		
		//Don't call asl_free for the message. The response owns it and will free it itself.
	}
	
	aslresponse_free(response);
	NSData *data = [dumpString dataUsingEncoding: NSUTF8StringEncoding];
	[dumpString release];
	return data;
}

- (void) makeMainWindowOnMainThread
{
	
	//This is fired via a cross thread message send from logic that checks to see if the window exists in the squeak thread.
	// Set up content view

	CGRect mainScreenSize = CGRectMake(0,0,480,360);
	CGRect fakeScreenSize = mainScreenSize	;
//	fakeScreenSize.size.width *= 2.0; 
//	fakeScreenSize.size.height *= 2.0;
	mainView = [[[self whatRenderCanWeUse] alloc] initWithFrame: fakeScreenSize];
	self.mainView.clearsContextBeforeDrawing = NO;
	self.mainView.autoresizesSubviews=NO;
// JMM show the screen	self.mainView.alpha = 0.0f;
	
	//Setup the scroll view which wraps the mainView
	if( UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM() )
		presentationSpace = [[ScratchIPhonePresentationSpace alloc] initWithNibName:@"ScratchIPhonePresentationSpaceiPad" bundle:[NSBundle mainBundle]];
	else
		presentationSpace = [[ScratchIPhonePresentationSpace alloc] initWithNibName:@"ScratchIPhonePresentationSpace" bundle:[NSBundle mainBundle]];
		
}

- (void)dealloc {
	[super dealloc];
	[userId release];
	[password release];
	[squeakProxy release];
	[imageView release];
	[presentationSpace release];
}
@end



void logIntegerToNSLog(int input) {
	NSNumber *num = [[NSNumber alloc] initWithInt: input];
	NSLog(@"%@",[num stringValue]);
	[num release];
}

