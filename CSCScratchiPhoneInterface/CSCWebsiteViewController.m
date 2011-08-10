    //
//  CSCWebsiteViewController.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-01-31.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "ScratchIPhoneAppDelegate.h"
#import "CSCWebsiteViewController.h"
#import "ASIHTTPRequest.h"
#import "ASIFormDataRequest.h"

extern ScratchIPhoneAppDelegate *gDelegateApp;


@implementation CSCWebsiteViewController
@synthesize webView, webUrl, squeakProxy, backArrow, forwardArrow;


- (id) initWithNibName:(NSString *)name bundle:(NSBundle *)bundle website:(NSURL *)aWebURL {
	// load the nib
	[super initWithNibName:name bundle:bundle];
	
	self.webUrl = aWebURL;
	return self;
}

- (id) initWithNibName:(NSString *)name bundle:(NSBundle *)bundle website:(NSURL *)aWebURL squeakProxy: aProxy{
	// load the nib
	[self initWithNibName: name bundle: bundle website: aWebURL];
	self.squeakProxy = aProxy;
	
	return self;
}

- (void) viewWillAppear:(BOOL)animated {
//	[[UIApplication sharedApplication] setStatusBarHidden: NO withAnimation: YES];
	[[UIApplication sharedApplication] setStatusBarHidden: NO animated: YES];	
//	[gDelegateApp.viewController setNavigationBarHidden: NO animated: YES];	
	[super viewWillAppear: animated];
	self.backArrow.enabled = self.webView.canGoBack;
	self.forwardArrow.enabled = self.webView.canGoForward;
}

- (void) viewDidLoad {
	[super viewDidLoad];
	// set the url
	if((UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM())) 
		webView.scalesPageToFit = YES;			
	[webView loadRequest: [NSURLRequest requestWithURL: self.webUrl]];
	
}

- (void)viewWillDisappear:(BOOL)animated {
	[super viewWillDisappear: animated];
}

- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)aError {
	
	UIAlertView *mailResult = [[UIAlertView alloc] init];
	
	NSString *ok = NSLocalizedString(@"Ok",nil);
	switch(result) {
		case MFMailComposeResultSaved:  {
			NSString *saved = NSLocalizedString(@"Saved",nil);
			NSString *savedMsg = NSLocalizedString(@"SavedMsg",nil);
  			mailResult.title = saved;
			mailResult.message = savedMsg;
			int indexOfButton = [mailResult addButtonWithTitle: ok];
			[mailResult dismissWithClickedButtonIndex:indexOfButton animated:YES];			
			[[self navigationController] dismissModalViewControllerAnimated:YES];			
			[mailResult show];
			[mailResult release];
			break;
		}
		case MFMailComposeResultFailed: {
			NSString *failed = NSLocalizedString(@"Failed",nil);
			NSString *failedMsg = NSLocalizedString(@"FailedMsg",nil);
			mailResult.title = failed;
			mailResult.message = failedMsg;
			int indexOfButton = [mailResult addButtonWithTitle:ok];
			[mailResult dismissWithClickedButtonIndex:indexOfButton animated:YES];			
			[[self navigationController] dismissModalViewControllerAnimated:YES];			
			[mailResult show];
			[mailResult release];
			break;
		default: 
			[[self navigationController] dismissModalViewControllerAnimated:YES];
			[mailResult release];			
		}
	}	
}

- (void) doEmail: (NSString*) to {
	if (!([MFMailComposeViewController canSendMail]))
		return;
	NSRange whereisQ = [to rangeOfString:@"?"];
	if (whereisQ.length == 0) 
		return;

	NSRange whereisE = [to rangeOfString:@"="];
	MFMailComposeViewController *email = [[MFMailComposeViewController alloc] init];
	email.mailComposeDelegate = self;	
	[email setToRecipients: [NSArray arrayWithObject: [to substringToIndex: whereisQ.location]]];
	[email setSubject: [[to substringFromIndex: whereisE.location+1] stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
	[[self navigationController] presentModalViewController:email animated:YES];
	[email release];
	
}

- (BOOL)webView:(UIWebView *) aWebView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
	BOOL what = [[UIApplication sharedApplication] canOpenURL: [request URL]];
	what = YES;
	
	if (what == NO) {
		
		NSRange howMuch;
		howMuch.location=0;
		howMuch.length=5;
		
		NSComparisonResult equal = [[[request URL] scheme] compare: @"about" options: NSCaseInsensitiveSearch  range: howMuch];
		if (equal == NSOrderedSame) {
			return YES;
		}
		
		if ([request.URL.scheme isEqualToString: @"mailto"] && [MFMailComposeViewController canSendMail]) {
			[self doEmail: request.URL.resourceSpecifier];
			return NO;
		}

		NSString *myLocalHost = @"localhost";
		
		howMuch.location=0;
		howMuch.length=myLocalHost.length;
		equal = [[[request URL] host] compare: myLocalHost options: NSCaseInsensitiveSearch  range: howMuch];
		if (equal == NSOrderedSame) {
			return YES;
		}
		
		// report the error inside the webview
		NSString *restriction = NSLocalizedString(@"Restriction",nil);
		NSString* errorString = [NSString stringWithFormat: restriction];
		[aWebView loadHTMLString:errorString baseURL:nil];
		
		return NO;
	}

	NSRange howMuch;
	NSString *urlString = [[request URL] absoluteString];
	NSString *urlHostString = [[request URL] host];
	NSString *masterString = @"scratch.mit.edu";
	NSComparisonResult	equal;

	
	if (([urlHostString isEqualToString: @"www.google.com"]) && ([urlString rangeOfString: masterString].length > 0))
		return YES;

	if ([urlHostString isEqualToString: @"www.mobilewikiserver.com"])
		return YES;

	if ([request.URL.scheme isEqualToString: @"mailto"] && [MFMailComposeViewController canSendMail]) {
		[self doEmail: request.URL.resourceSpecifier];
		return NO;
	}
	
	if (urlHostString.length < masterString.length)
		return NO;
	
	howMuch.location=urlHostString.length - masterString.length;
	howMuch.length=masterString.length;
	equal = [urlHostString compare: masterString options: NSCaseInsensitiveSearch  range: howMuch];
	if (!(equal == NSOrderedSame)) {
		return NO;
	}
	
	NSString *scratchDownLoad = @"http://scratch.mit.edu/projects";
	NSString *download = @"/downloadsb";  //Fix Summer 2011 URL at MIT changes from /download
	
	howMuch.location=0;
	howMuch.length=scratchDownLoad.length;
	equal = [urlString compare: scratchDownLoad options: NSCaseInsensitiveSearch  range: howMuch];
	if (equal == NSOrderedSame && [urlString length] > ([scratchDownLoad length]+[download length])) {
		NSArray *urlComponents = [urlString componentsSeparatedByString:@"/"];
		howMuch.location=[urlString length]-[download length];
		howMuch.length = [download length];
		equal = [urlString compare: download options: NSCaseInsensitiveSearch  range: howMuch];
		if (equal == NSOrderedSame ) {
/*
			NSString *projectNumber = [urlComponents objectAtIndex: [urlComponents count] -2];
			NSString *tempFileName = [NSTemporaryDirectory() stringByAppendingPathComponent: projectNumber];
			NSFileManager *fileManager = [NSFileManager defaultManager];
			if ([fileManager isReadableFileAtPath: tempFileName]) {
				[self.squeakProxy chooseThisProject: tempFileName runProject: 1];
				[gDelegateApp.presentationSpace goBackToPlayerNoAlphaAlter: tempFileName];
				return NO;
			}
*/
			[self grabURLInBackground: [request URL]];
			return NO;
		}
		NSString *projectNumber2 = [urlComponents objectAtIndex: [urlComponents count]-1];
		if ([self holdsIntegerValue: projectNumber2]) {
/*
 
			NSString *tempFileName = [NSTemporaryDirectory() stringByAppendingPathComponent: projectNumber2];
			NSFileManager *fileManager = [NSFileManager defaultManager];
			if ([fileManager isReadableFileAtPath: tempFileName]) {
				[self.squeakProxy chooseThisProject: tempFileName runProject: 1];
				[gDelegateApp.presentationSpace  goBackToPlayerNoAlphaAlter: tempFileName];
				return YES;
			}
*/
            NSURL *downloadURL = [NSURL URLWithString: [NSString stringWithFormat:
                                                        @"%@%@",[[request URL] absoluteString],download]];
			[self grabURLInBackground: downloadURL];
			return YES;
		}
	}
	return YES;
}

- (void) grabURLInBackground: (NSURL *) url
{
	[gDelegateApp turnActivityViewOn];
	ASIHTTPRequest *request = [ASIHTTPRequest requestWithURL:url];
	[request setDelegate:self];
	request.useCookiePersistance = NO;
	[request startAsynchronous];
}

- (BOOL)holdsIntegerValue: (NSString *) check;
{
    if ([check length] == 0)
        return NO;
    
    NSString *compare = [check stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    NSCharacterSet *validCharacters = [NSCharacterSet decimalDigitCharacterSet];
    for (NSUInteger i = 0; i < [compare length]; i++) 
    {
        unichar oneChar = [compare characterAtIndex:i];
        if (![validCharacters characterIsMember:oneChar])
            return NO;
    }
    return YES;
}


- (void)requestFinished:(ASIHTTPRequest *)request
{
	// Use when fetching binary data
	NSData *responseData = [request responseData];
	NSString *urlString = [[request originalURL] absoluteString];
	NSArray *urlComponents = [urlString componentsSeparatedByString:@"/"];
	NSString *projectNumber = [urlComponents objectAtIndex: [urlComponents count] -2];
	NSString *tempFileName = [NSTemporaryDirectory() stringByAppendingPathComponent: projectNumber];
	NSFileManager *fileManager = [NSFileManager defaultManager];
	[fileManager removeItemAtPath: tempFileName error:NULL];
	BOOL ok = [responseData writeToFile: tempFileName atomically: NO];
	[gDelegateApp.presentationSpace  goBackToPlayerNoAlphaAlter: tempFileName];
}

- (NSString *) craftFailureMessage: (NSString*) ld {
	NSString *internetFailure = NSLocalizedString(@"InternetFailure",nil);
	if (ld == nil) {
		return internetFailure;
	} else {
		NSMutableString *ldExtra = [NSMutableString stringWithString: internetFailure];
		[ldExtra appendString:ld];
		return ldExtra;
	}	
}

- (void)requestFailed:(ASIHTTPRequest *)request
{
	NSError *aError = [request error];
	NSString * ld = [aError localizedDescription];
	NSString * lrs = [aError localizedRecoverySuggestion];
	NSString * okText = NSLocalizedString(@"Ok", nil);
	NSString * lfr = (NSString *)[aError localizedFailureReason];
	NSString * ldExtra = [self craftFailureMessage: ld];
#pragma unused(lrs)
	[gDelegateApp terminateActivityView];

	UIAlertView * dialog = [[UIAlertView alloc] initWithTitle: lfr 
													  message: ldExtra  delegate: nil
											cancelButtonTitle: okText  otherButtonTitles: nil];
	[dialog show];
	[dialog release];
}

- (void)webViewDidStartLoad:(UIWebView *)aWebView
{
	// starting the load, show the activity indicator in the status bar
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible: YES];
}

- (void)webViewDidFinishLoad:(UIWebView *)aWebView
{ 
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible: NO];

	self.backArrow.enabled = self.webView.canGoBack;
	self.forwardArrow.enabled = self.webView.canGoForward;
	
	if (gDelegateApp.loginToScratchCompleted) {
		return;
	}
	
	gDelegateApp.loginToScratchCompleted = YES;
	NSURL *urlRequest = [[aWebView request] URL];
	NSString *urlString = [urlRequest absoluteString];
	
	if ([urlString isEqualToString: @"http://scratch.mit.edu/login"] ) {
		NSString *s = [[NSString alloc] initWithFormat:
					   @"<html><body onload=\"document.forms[0].submit()\"><form  name=\"logs\" action=\"/login\" \
					   method=\"POST\" accept-charset=\"UTF-8\"> username:<input id=\"UserInput\" \
					   type=\"text\" name=\"User\" value=\"%@\">password:<input type=\"password\" \
					   name=\"Pass\" value=\"%@\"><input type=\"hidden\" name=\"refer\" value=\"/login\"> \
					   </form></body></html>",gDelegateApp.userId,gDelegateApp.password];
		[s autorelease];
		[aWebView loadHTMLString: s baseURL: urlRequest];
	}
	
}

//This is called on an error, grab all error information, but we only use some of it

- (void)webView:(UIWebView *)aWebView didFailLoadWithError:(NSError *)aError
{
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible: NO];
	
	if (aError.code == NSURLErrorCancelled) return;
	
	NSString * ld = [aError localizedDescription];
	NSString * lfr = (NSString *)[aError localizedFailureReason];
	NSArray * lro = [aError localizedRecoveryOptions];
	NSString * lrs = [aError localizedRecoverySuggestion];
#pragma unused(lrs,lro)
	NSString * okText = NSLocalizedString(@"Ok", nil);
	NSString * ldExtra = [self craftFailureMessage: ld];
	
	UIAlertView * dialog = [[UIAlertView alloc] initWithTitle: lfr 
													  message: ldExtra  delegate: nil
											cancelButtonTitle: okText  otherButtonTitles: nil];
	[dialog show];
	[dialog release];
}

- (IBAction) goBackToPlayer:(id)sender {
	[gDelegateApp.presentationSpace goBackToPlayer];
}

- (IBAction) helpSystem: (id) sender {
	[webView loadRequest: [NSURLRequest requestWithURL: [NSURL URLWithString: @"http://www.mobilewikiserver.com/Scratch.html"]]];

}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return NO;
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	[super viewDidUnload];
	self.webView = nil;
	self.backArrow = nil;
	self.forwardArrow = nil;
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible: NO];
}

- (void)dealloc {
	[webView release];
	[webUrl release];
//	[backArrow release];
//	[forwardArrow release];
    [super dealloc];
}


@end
