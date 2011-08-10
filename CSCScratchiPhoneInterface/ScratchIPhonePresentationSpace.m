//
//  ScratchIPhonePresentationSpace.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-15.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "ScratchIPhonePresentationSpace.h"
#import "ScratchIPhoneAppDelegate.h"
#import "SqueakUIController.h"
#import "AboutInternalViewController.h"
#import "sqSqueakIPhoneInfoPlistInterface.h"

#import <QuartzCore/QuartzCore.h>

extern ScratchIPhoneAppDelegate *gDelegateApp;


@implementation ScratchIPhonePresentationSpace
@synthesize scrollView,textView,scrollViewController,infoButton,shoutGoButton,stopAllButton,	
	webButton,textField,activityTimer,pathToProjectFile,repeatKeyDict,rememberTextOnMemoryWarning,
	shoutGoLandscapeButton,stopAllLandscapeButton,landscapeToolBar,landscapeToolBar2,thereIsNoInternet,padLockButton,
	popUpInfoViewController;


 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
		firstView = YES;
		rememberTextOnMemoryWarning = nil;
		repeatKeyDict = [[NSMutableDictionary alloc] init];
	}
    return self;
}



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];

	if (self.rememberTextOnMemoryWarning) 
		self.textView.text = self.rememberTextOnMemoryWarning;
	
	self.textView.layer.borderWidth = 1;
	self.textView.layer.borderColor = [[UIColor grayColor] CGColor];
	self.textField.keyboardAppearance = UIKeyboardAppearanceAlert;
	self.scrollView.contentSize = [gDelegateApp.mainView bounds].size; 
	[self.scrollView addSubview: gDelegateApp.mainView];
	if((UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM())) {
		[self.scrollView setZoomScale: 1.5f animated: NO];
		self.scrollView.minimumZoomScale = 1.5f; 
	} else {
		[self.scrollView setZoomScale: 2.0f/3.0f animated: NO];
		self.scrollView.minimumZoomScale = 2.0f/3.0f; 
	}

}

- (void) viewWillAppear:(BOOL)animated {
//	[[UIApplication sharedApplication] setStatusBarHidden: YES withAnimation: YES];
	[[UIApplication sharedApplication] setStatusBarHidden: YES animated: YES];	
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible: NO];
	[gDelegateApp.viewController setNavigationBarHidden: YES animated: YES];	
	if (self.thereIsNoInternet) 
		self.webButton.enabled = NO;
	self.scrollView.delaysContentTouches = self.padLockButton.selected; 
	[super viewWillAppear: animated];
}

- (void) viewWillDisappear:(BOOL)animated {
	[super viewWillDisappear: animated];
	@synchronized(self) {
		[self deletePendingTimer];
	}	
}

- (void) fireAlphaToOneOnMainThread {
	[gDelegateApp terminateActivityView];
	@synchronized(self) {
		[self deletePendingTimer];
	}
/* JMM show the scratch desktop now 
	if (gDelegateApp.mainView.alpha == 0.0f) {
		[UIView beginAnimations:nil context:nil];
		[UIView setAnimationDuration: 0.75];
		gDelegateApp.mainView.alpha = 1.0f;
		[UIView commitAnimations];
	}
*/
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	if(standardUserDefaults) {
		[standardUserDefaults setObject: self.pathToProjectFile forKey:@"pathToProjectFile"];
		[standardUserDefaults synchronize];
	}	
}

- (void) fireAlphaToOne {
	projectIsLive = YES;
	[self performSelectorOnMainThread:@selector(fireAlphaToOneOnMainThread) withObject: nil waitUntilDone: NO];
}

- (IBAction) goBackToPlayer {
	[self  goBackToPlayerNoAlphaAlter: nil];
	[self fireAlphaToOne];
	self.shoutGoButton.selected = NO;
	self.stopAllButton.selected = YES;
	self.shoutGoLandscapeButton.selected = NO;
	self.stopAllLandscapeButton.selected = YES;
}

- (void) loadingProjectPleaseWaitOnMainThread {
	NSString *herding = NSLocalizedString(@"herding",nil);
	self.textView.text = herding;
	[gDelegateApp turnActivityViewOn];
}
	
- (void) deletePendingTimer {
	if (self.activityTimer) {
		[self.activityTimer invalidate];
		self.activityTimer = NULL;
	}	
}

- (void) startTheWaitTimmer {
	CGFloat waitTimeout = [(sqSqueakIPhoneInfoPlistInterface*) gDelegateApp.squeakApplication.infoPlistInterfaceLogic timeOut];
	self.activityTimer = [NSTimer timerWithTimeInterval: waitTimeout target:self selector:@selector(showAlertViaTimer) userInfo:nil repeats:NO];
	[[NSRunLoop mainRunLoop] addTimer: self.activityTimer forMode: NSDefaultRunLoopMode];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (projectIsLive)
		return;
	if (buttonIndex) {
		[gDelegateApp turnActivityViewOn];
		@synchronized(self) {
			[self startTheWaitTimmer];
		}
		return;
	}
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	if(standardUserDefaults) {
		[standardUserDefaults removeObjectForKey: @"pathToProjectFile"];
		[standardUserDefaults synchronize];
	}	
	[gDelegateApp restartWeHope];
}

- (void) showAlertViaTimer{
	@synchronized(self) {
		self.activityTimer = nil;
	}
	[gDelegateApp terminateActivityView];
	UIAlertView *alertView = [UIAlertView alloc];
	NSString *ahem = NSLocalizedString(@"Ahem",nil);
	NSString *ahemMsg = NSLocalizedString(@"AhemMsg",nil);	
	NSString *reset = NSLocalizedString(@"Reset",nil);	
	NSString *waitString = NSLocalizedString(@"Wait",nil);	
	alertView = [alertView initWithTitle: ahem message: ahemMsg delegate: self cancelButtonTitle: reset otherButtonTitles: waitString,nil];
	[alertView show];
	[alertView release];
}

- (void) loadingProjectPleaseWait {
	projectIsLive = NO;
	[self performSelectorOnMainThread:@selector(loadingProjectPleaseWaitOnMainThread) withObject: nil waitUntilDone: NO];
	@synchronized(self) {
		[self deletePendingTimer];
		[self startTheWaitTimmer];
	}
}

- (void) chooseThisProjectDontRun: (NSString *) file {
	[gDelegateApp.squeakProxy chooseThisProject: file runProject: 0];
}

- (void) chooseThisProjectRun: (NSString *) file {
	[gDelegateApp.squeakProxy chooseThisProject: file runProject: 1];
}


- (void) viewDidAppear:(BOOL)animated {
	[super viewDidAppear: animated];
	if (firstView) {
		firstView = NO;
		[gDelegateApp terminateActivityView];
		NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
		if(standardUserDefaults) {
			NSFileManager *fileManager = [NSFileManager defaultManager];
			NSString *tempFileName = self.pathToProjectFile = [standardUserDefaults objectForKey:@"pathToProjectFile"];
			if (tempFileName == nil || ![fileManager isReadableFileAtPath: tempFileName]) {
				NSAutoreleasePool * pool = [NSAutoreleasePool new];
				NSString *defaultProjectName = NSLocalizedString(@"defaultProjectName",nil);
				tempFileName = [[NSBundle mainBundle] pathForResource: defaultProjectName ofType: @"sb"];
				self.shoutGoButton.selected = YES;
				[self performSelectorOnMainThread:@selector(chooseThisProjectRun:) withObject: tempFileName waitUntilDone: NO];
				[pool drain];
			} else  {
				self.shoutGoButton.selected = NO;
				[self performSelectorOnMainThread:@selector(chooseThisProjectDontRun:) withObject: tempFileName waitUntilDone: NO];
			}
		}	
	}
	if((UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM()) && 
	   (UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation])))
			self.textField.becomeFirstResponder;
	
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return YES;
}

- (void)willAnimateRotationToInterfaceOrientation:(UIInterfaceOrientation)targetedOrientation duration:(NSTimeInterval)duration {
	if( UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM()) {
		if (UIInterfaceOrientationIsLandscape(targetedOrientation)){
			CGRect r = self.landscapeToolBar.frame;
			r.size.width += 24;
			r.origin.x -= 24;
			self.landscapeToolBar.frame = r;
			r = self.landscapeToolBar2.frame;
			r.size.width += 24;
			self.landscapeToolBar2.frame = r;
		}
		if (UIInterfaceOrientationIsPortrait(targetedOrientation)) {
			CGRect r = self.landscapeToolBar.frame;
			r.size.width -= 24;
			r.origin.x += 24;
			self.landscapeToolBar.frame = r;
			r = self.landscapeToolBar2.frame;
			r.size.width -= 24;
			self.landscapeToolBar2.frame = r;
		}
		return;
	}
	
	
	if (UIInterfaceOrientationIsLandscape(targetedOrientation)) {
		self.infoButton.hidden = YES;
		self.shoutGoButton.hidden = YES;
		self.stopAllButton.hidden = YES;
		self.webButton.hidden = YES;
		self.textField.hidden = YES;
		self.textView.hidden = YES;
		self.padLockButton.hidden = YES;
		self.landscapeToolBar.hidden = NO;
		self.landscapeToolBar2.hidden = NO;
		self.scrollView.frame = CGRectMake(26.0f,0.0f, 427.0f, 320.0f);
		self.scrollView.minimumZoomScale = 320.0f/360.0f; 
		[self.scrollView setZoomScale: 320.0f/360.0f animated: NO];
	}
	if (UIInterfaceOrientationIsPortrait(targetedOrientation)) {
		self.infoButton.hidden = NO;
		self.padLockButton.hidden = NO;
		self.shoutGoButton.hidden = NO;
		self.stopAllButton.hidden = NO;
		self.webButton.hidden = NO;
		self.textField.hidden = NO;
		self.textView.hidden = NO;
		self.landscapeToolBar.hidden = YES;
		self.landscapeToolBar2.hidden = YES;
		self.scrollView.frame = CGRectMake(0.0f,0.0f, 320.0f, 240.0f);
		self.scrollView.minimumZoomScale = 2.0f/3.0f; 
		[self.scrollView setZoomScale: 2.0f/3.0f animated: NO];
	}
}

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController {
	self.popUpInfoViewController = nil;
}

- (IBAction) clickInfo:(id)sender {
	AboutInternalViewController *aboutViewController = [[AboutInternalViewController alloc] initWithNibName:@"AboutInternalViewController" bundle:[NSBundle mainBundle]];

//	[gDelegateApp bailWeAreBroken: @"Forced email diagnostics"];
	if( UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM()) { 
		Class UIPopoverControllerClass = NSClassFromString(@"UIPopoverController");
		popUpInfoViewController  = [[UIPopoverControllerClass alloc] initWithContentViewController: aboutViewController];
		self.popUpInfoViewController.delegate = self;
		[self.popUpInfoViewController setPopoverContentSize: CGSizeMake(320.0f,480.0f - 44.0f) animated: YES];
		[self.popUpInfoViewController presentPopoverFromRect: self.infoButton.frame inView: self.view permittedArrowDirections:UIPopoverArrowDirectionAny animated: YES];
	} else {
		[gDelegateApp.viewController pushViewController: aboutViewController animated:YES];
	}
	[aboutViewController release];	
}

- (IBAction) operatePadLock: (id) sender {
	self.padLockButton.selected = !self.padLockButton.selected;
	self.scrollView.delaysContentTouches = self.padLockButton.selected;  // padlock is open (aka selected) so we delay
}

- (IBAction) shoutGo:(id)sender {
	self.shoutGoButton.selected = YES;
	self.stopAllButton.selected = NO;
	self.shoutGoLandscapeButton.selected = YES;
	self.stopAllLandscapeButton.selected = NO;
	[gDelegateApp.squeakProxy performSelectorOnMainThread:@selector(shoutGo) withObject: nil waitUntilDone: NO];
}

- (IBAction) stopAll:(id)sender {
	self.shoutGoButton.selected = NO;
	self.stopAllButton.selected = YES;
	self.shoutGoLandscapeButton.selected =  NO;
	self.stopAllLandscapeButton.selected = YES;
	[gDelegateApp.squeakProxy performSelectorOnMainThread:@selector(stopAll) withObject: nil waitUntilDone: NO];
}

- (void) launchWebSiteFromPresentationMode {
	[gDelegateApp.squeakProxy performSelectorOnMainThread:@selector(launchWebSiteFromPresentationMode) withObject: nil waitUntilDone: NO];
}

- (IBAction) takeMeBackToTheWeb:(id)sender{ 
	self.shoutGoButton.selected = NO;
	self.stopAllButton.selected = YES;
	self.padLockButton.selected = NO;
	self.shoutGoLandscapeButton.selected = NO;
	self.stopAllLandscapeButton.selected = YES;
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration: 0.50];
	[UIView setAnimationDelegate: self];
	[UIView setAnimationDidStopSelector: @selector(launchWebSiteFromPresentationMode)];

//JMM don't do the alpha now show the desktop	gDelegateApp.mainView.alpha = 0.0f;

	[UIView commitAnimations];
}

- (void) goBackToPlayerNoAlphaAlter: (NSString *) aPathToProjectFile {
	if (aPathToProjectFile)
		self.pathToProjectFile = aPathToProjectFile;
	[[gDelegateApp viewController] popToViewController: gDelegateApp.presentationSpace animated: YES];
	if (aPathToProjectFile) 
		[gDelegateApp.squeakProxy chooseThisProject: aPathToProjectFile runProject: 1];
	self.shoutGoButton.selected = YES;
	self.stopAllButton.selected = NO;
	self.shoutGoLandscapeButton.selected = YES;
	self.stopAllLandscapeButton.selected = NO;
	[gDelegateApp terminateActivityView];
}

- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
	return gDelegateApp.mainView;
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *) aTextField {
	if (!self.shoutGoButton.selected)
		return NO;
	aTextField.text = @" ";
	characterCounter = 0;
	return YES;
}


- (BOOL)textField:(UITextField *) aTextField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
	if ([string length] > 0 && [string characterAtIndex: 0] == (unichar) 10) {
		[aTextField resignFirstResponder];
		aTextField.text = @"";
		[gDelegateApp.mainView recordCharEvent: string];
		return NO;
	}
	
/*	if (characterCounter <= 4 && [string length] > 0) {
		debugCharacters[characterCounter] = [string characterAtIndex: 0];
		if (characterCounter == 4) {
			NSString *debugString = [[NSString alloc] initWithCharacters: (const unichar *)&debugCharacters length: 5];
			if ([debugString isEqualToString: @"debug"]) {
				[debugString release];
				[aTextField resignFirstResponder];
				gDelegateApp.mainView.alpha = 1.0f;
				[gDelegateApp.squeakProxy performSelectorOnMainThread:@selector(exitToDebugMode) withObject: nil waitUntilDone: NO];
				return NO;
			}
			[debugString release];
		}
		characterCounter++;
	}
 */
	if ([string length] == 0) {
		const unichar delete = 0x08;
		[gDelegateApp.mainView recordCharEvent: [NSString stringWithCharacters: &delete length: 1] ];
	}
	else
		[gDelegateApp.mainView recordCharEvent: string];
	return NO;
}

- (void) pushCharacter: (NSString*) string {
	[gDelegateApp.mainView recordCharEvent: string];
}

- (void)repeatKeyDoKey:(NSTimer*)theTimer {
	[self pushCharacter: [[theTimer userInfo] string]];
}

- (void)repeatKeySecondPhase:(NSTimer*)theTimer {
	[self repeatKeyDoKey: theTimer];
	NSNumber *senderHash = [[theTimer userInfo] senderHash];
	@synchronized(self) {
		NSTimer *newTimer = [NSTimer scheduledTimerWithTimeInterval:0.05f target:self 
															 selector:@selector(repeatKeyDoKey:) 
												  userInfo: [theTimer userInfo] repeats:YES];
		[self.repeatKeyDict removeObjectForKey: senderHash];
		[self.repeatKeyDict setObject:newTimer forKey: senderHash];
	}
}

- (void) startRepeatKeyAction: (NSString*) string  for: (id) sender {
	@synchronized(self) {
		ScratchRepeatKeyMetaData *stub = [[ScratchRepeatKeyMetaData alloc] init];
		stub.string = string;
		stub.senderHash = [NSNumber numberWithUnsignedInteger:[sender hash]] ;
		NSTimer *repeatKeyTimerInstance = [NSTimer scheduledTimerWithTimeInterval:0.2f target:self 
															 selector:@selector(repeatKeySecondPhase:) 
											  userInfo: stub repeats:NO];
		[self.repeatKeyDict setObject:repeatKeyTimerInstance forKey: stub.senderHash];
		[stub release];
	}
}

- (IBAction) keySpace: (id) sender {
	BOOL spaceRepeats = [(sqSqueakIPhoneInfoPlistInterface*) gDelegateApp.squeakApplication.infoPlistInterfaceLogic spaceRepeats];
	if (spaceRepeats) {
		unichar character = 32;
		[self startRepeatKeyProcess: character for: sender];
	} else {
		[self pushCharacter: @" "];
	}
}

- (IBAction) keyTouchUp:(id)sender {
	@synchronized(self) {
		NSNumber *senderHash = [NSNumber numberWithUnsignedInteger:[sender hash]];
		NSTimer *repeatKeyTimerInstance = [self.repeatKeyDict objectForKey: senderHash];
		if (repeatKeyTimerInstance) {
			[repeatKeyTimerInstance invalidate];
			[self.repeatKeyDict removeObjectForKey: senderHash];
		}
	}
}

- (void) startRepeatKeyProcess: (unichar) character for: (id) sender {
	NSString *string = [[NSString alloc] initWithCharacters:&character length: 1];
	[self pushCharacter: string];
	[self startRepeatKeyAction: string for: sender];
	[string release];
}

- (IBAction) keyUpArrow:(id)sender {
	unichar character = 30;
	[self startRepeatKeyProcess: character for: sender];
}

- (IBAction) keyDownArrow:(id)sender {
	unichar character = 31;
	[self startRepeatKeyProcess: character for: sender];
}

- (IBAction) keyLeftArrow:(id)sender {
	unichar character = 28;
	[self startRepeatKeyProcess: character for: sender];
}

- (IBAction) keyRightArrow:(id)sender {
	unichar character = 29;
	[self startRepeatKeyProcess: character for: sender];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	[super viewDidUnload];
	self.scrollView = nil;
	self.scrollViewController = nil;
	self.rememberTextOnMemoryWarning = self.textView.text;
	self.textView = nil;
	self.infoButton = nil;
	self.shoutGoButton = nil;
	self.stopAllButton = nil;
	self.shoutGoLandscapeButton = nil;
	self.stopAllLandscapeButton = nil;
	self.landscapeToolBar = nil;
	self.landscapeToolBar2 = nil;
	self.webButton = nil;
	self.padLockButton = nil;
	self.textField = nil;
}


- (void)dealloc {
	@synchronized(self) {
		[self deletePendingTimer];
		for (NSTimer *e in [self.repeatKeyDict allValues]) {
			[e invalidate];
		}
	}
    [super dealloc];
	[repeatKeyDict release];
	[scrollView release];
	[textView release];
	[scrollViewController release];
	[infoButton release];
	[shoutGoButton release];
	[stopAllButton release];
	[shoutGoLandscapeButton release];
	[stopAllLandscapeButton release];
	[landscapeToolBar release];
	[landscapeToolBar2 release];
	[webButton release];
	[padLockButton release];
	[textField release];
	[activityTimer release];
	[pathToProjectFile release];
	[rememberTextOnMemoryWarning release];
}
@end
