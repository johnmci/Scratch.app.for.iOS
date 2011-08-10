//
//  LoginViewController.m
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "LoginViewController.h"
#import "SFHFKeychainUtils.h"
#import "ScratchIPhoneAppDelegate.h"
#import "Reachability.h"
#import "AboutViewController.h"

extern ScratchIPhoneAppDelegate *gDelegateApp;

@implementation LoginViewController
@synthesize checkButton, usernameIn, passwordIn, scrollTextFieldAndLogo, rememberMe, loginButton, nagMessageAlert, username, password;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        squeakVMIsReady = NO;
    }
	return self;
}

- (void)loadView {
	[super loadView];
	self.title = NSLocalizedString(@"Login",nil);
}

- (void) viewDidLoad {
	[super viewDidLoad];	
	NSString *waitString = NSLocalizedString(@"Wait",nil);
	[self.loginButton setTitle:  waitString forState:UIControlStateNormal];
}

- (void)viewWillAppear:(BOOL)animated {
	[super viewWillAppear: animated];
	squeakVMIsReady = gDelegateApp.squeakVMIsReady;
	if (squeakVMIsReady)
		[self squeakVMIsReadyNowOnMainThread];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(squeakVMIsReadyNow) name:@"squeakVMIsReadyNow" object:nil];
	self.loginButton.enabled = uiThinksLoginButtonCouldBeEnabled = NO;
	
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	
	// fill in user's information
	
	checkButton.selected = [standardUserDefaults integerForKey:@"remembered"];
	
	// display whether the user had previously chosen to save authentication information
	if([standardUserDefaults integerForKey:@"remembered"] == 1) {
		if([standardUserDefaults stringForKey:@"userName"] != nil) {
			usernameIn.text = [standardUserDefaults stringForKey:@"userName"];
		}
		
		NSError *err;
		passwordIn.text = [SFHFKeychainUtils getPasswordForUsername: usernameIn.text andServiceName: @"edu.mit.scratch.password" error: &err];
		uiThinksLoginButtonCouldBeEnabled = YES;
		self.loginButton.enabled = uiThinksLoginButtonCouldBeEnabled && squeakVMIsReady;
	} else {
		usernameIn.text=@"";
		passwordIn.text=@"";		
	}	
	
	self.nagMessageAlert = [CSCNagDialogForScratchAccount showNagMessageIfRequired];
}

- (void) viewDidAppear:(BOOL)animated {
	[super viewDidAppear: animated];

	if(gDelegateApp.loginAutomatically) {
			if([usernameIn.text length] > 0 && [passwordIn.text length] > 0)
				// automatically "click login" on load
				[self clickLoginButton:self];
	}
}

- (void)viewWillDisappear:(BOOL)animated {
	[[NSNotificationCenter defaultCenter] removeObserver: self];
}

- (void) setRememberData {
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	// erase the login info if not remembered anymore
	if(standardUserDefaults) {
		if(!checkButton.selected) {
			[standardUserDefaults setInteger:0 forKey:@"remembered"];
			[standardUserDefaults setObject: nil forKey:@"userName"];
		} else {
			[standardUserDefaults setInteger:1 forKey:@"remembered"];
			[standardUserDefaults setObject:usernameIn.text forKey:@"userName"];
		}	
		[standardUserDefaults synchronize];
	}	
}

- (BOOL)isDataSourceAvailable {
	return YES;

//	return !([[Reachability reachabilityForInternetConnection] currentReachabilityStatus] == NotReachable);
//	return !([[Reachability reachabilityWithHostName: @"scratch.mit.edu"] currentReachabilityStatus] == NotReachable);
}

- (void) storeUserNameAndPassword {
	[self setRememberData];
	NSError *err;
	[SFHFKeychainUtils storeUsername: usernameIn.text andPassword: passwordIn.text forServiceName: @"edu.mit.scratch.password" updateExisting:YES error: &err];
}

#pragma mark -
#pragma mark Button Action
// responds to click to login and authenticates user
- (IBAction) clickLoginButton:(id)sender {
	
	self.loginButton.enabled = uiThinksLoginButtonCouldBeEnabled = NO;
	gDelegateApp.userId = usernameIn.text;
	gDelegateApp.password = passwordIn.text;
	[self storeUserNameAndPassword];
	
	if([self isDataSourceAvailable] && ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString: @"http://scratch.mit.edu"]])) {
		[gDelegateApp turnActivityViewOn];
		gDelegateApp.imageView.hidden = YES;
		[gDelegateApp.viewController pushViewController: gDelegateApp.presentationSpace animated: YES];
	} else { // no Internet connection - possibly in airplane mode
		NSString *noInternet = NSLocalizedString(@"NoInternet",nil);
		NSString *noInternetMsg = NSLocalizedString(@"NoInternetMsg",nil);
		NSString *ok = NSLocalizedString(@"Ok",nil);

		UIAlertView *notAuthenticated = [[UIAlertView alloc] initWithTitle:noInternet
																   message: noInternetMsg
																  delegate:nil
														 cancelButtonTitle: ok
														 otherButtonTitles:nil];
		[notAuthenticated show];
		[notAuthenticated release];
		uiThinksLoginButtonCouldBeEnabled = YES;
		self.loginButton.enabled = uiThinksLoginButtonCouldBeEnabled && squeakVMIsReady;
		[gDelegateApp turnActivityViewOn];
		gDelegateApp.imageView.hidden = YES;
//JMM DISABLE		gDelegateApp.presentationSpace.thereIsNoInternet = YES;
		[gDelegateApp.viewController pushViewController: gDelegateApp.presentationSpace animated: YES];
	}
}

// toggles the checkmark to remember the username and password
- (IBAction) clickCheckButton:(id)sender {
	UIButton *button = (UIButton *) sender;

	// switch it!
	button.selected = !button.selected;
	[self setRememberData];
}

#pragma mark -
#pragma mark Scrolling
// make room for the keyboard
-(void)textFieldDidBeginEditing:(UITextField *)textField {
	
	if (fieldWasAdjustUpwardsAlready)
		return;
	
	fieldWasAdjustUpwardsAlready = YES;
	
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration: 1.5f];
	
	scrollTextFieldAndLogo.frame = CGRectMake(7,-20,306,374); 
	[scrollTextFieldAndLogo scrollRectToVisible:textField.frame animated:YES]; //Oddly enough, it only works with the animation...
	CGPoint c = self.usernameIn.center;	
	c.y -= 30.0f;
	self.usernameIn.center = c;
	c = self.passwordIn.center;	
	c.y -= 30.0f;
	self.passwordIn.center = c;
	c = self.password.center;	
	c.y -= 30.0f;
	self.password.center = c;
	c = self.username.center;	
	c.y -= 30.0f;
	self.username.center = c;
	c = self.checkButton.center;	
	c.y -= 60.0f;
	self.checkButton.center = c;
	c = self.rememberMe.center;	
	c.y -= 60.0f;
	self.rememberMe.center = c;
	[UIView commitAnimations];
}

-(void)textFieldDidEndEditing:(UITextField *)textField {
		
	if ([[passwordIn text] length] > 0 && [[usernameIn text] length] > 0) {
		uiThinksLoginButtonCouldBeEnabled = YES;
		self.loginButton.enabled = uiThinksLoginButtonCouldBeEnabled && squeakVMIsReady;
	} else {
		self.loginButton.enabled = uiThinksLoginButtonCouldBeEnabled = NO;
	}
}

- (void) putTextBackToOriginalPlace {
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration: 1.5f];
	scrollTextFieldAndLogo.frame = CGRectMake(7,4,306,374); //original setup
	CGPoint c = self.usernameIn.center;	
	c.y += 30.0f;
	self.usernameIn.center = c;
	c = self.passwordIn.center;	
	c.y += 30.0f;
	self.passwordIn.center = c;
	c = self.password.center;	
	c.y += 30.0f;
	self.password.center = c;
	c = self.username.center;	
	c.y += 30.0f;
	self.username.center = c;
	c = self.checkButton.center;	
	c.y += 60.0f;
	self.checkButton.center = c;
	c = self.rememberMe.center;	
	c.y += 60.0f;
	self.rememberMe.center = c;
	[UIView commitAnimations];
	fieldWasAdjustUpwardsAlready = NO;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	[textField resignFirstResponder];

	if (textField == passwordIn && [[textField text] length] > 0 && [[usernameIn text] length] > 0 && squeakVMIsReady) {
		[self performSelectorOnMainThread:@selector(clickLoginButton:) withObject: self waitUntilDone: NO];
		return YES;
	}
	[self putTextBackToOriginalPlace];
	return YES;
}

- (IBAction) clickAbout:(id)sender {
	AboutViewController *aboutViewController = [[AboutViewController alloc] initWithNibName:@"AboutViewController" bundle:[NSBundle mainBundle]];
	[gDelegateApp.viewController pushViewController: aboutViewController animated:YES];
	[aboutViewController release];
}

- (void) squeakVMIsReadyNowOnMainThread {
	squeakVMIsReady = YES;
	NSString *login = NSLocalizedString(@"Login",nil);
	[self.loginButton setTitle: login forState:UIControlStateNormal];
	self.loginButton.enabled = uiThinksLoginButtonCouldBeEnabled && squeakVMIsReady;
}

- (void) squeakVMIsReadyNow {
	[self performSelectorOnMainThread:@selector(squeakVMIsReadyNowOnMainThread) withObject: nil waitUntilDone: NO];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	[super viewDidUnload];
	checkButton = nil;
	usernameIn = nil;
	passwordIn = nil;
	password = nil;
	username = nil;
	passwordIn = nil;
	loginButton = nil;
	scrollTextFieldAndLogo = nil;
	rememberMe = nil;
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver: self];
	[checkButton release];
	[usernameIn release];
	[password release];
	[username release];
	[passwordIn release];
	[scrollTextFieldAndLogo release];
	[rememberMe release];
	[nagMessageAlert release];
	[loginButton release];
    [super dealloc];
}


@end
