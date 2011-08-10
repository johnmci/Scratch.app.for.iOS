//
//  AboutInternalViewController.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-25.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "AboutInternalViewController.h"


@implementation AboutInternalViewController
@synthesize htmlView;

- (NSString*) copyrightText {
	NSString *about = NSLocalizedString(@"About",nil);
	return about;
}


- (void)loadView {
	[super loadView];
	NSError *err;
	NSString *htmlString = [NSString stringWithContentsOfFile: [[NSBundle mainBundle] pathForResource: @"ScratchLicenseInfo" ofType: @"html"] encoding: NSUTF8StringEncoding error: &err];
	[self.htmlView setDelegate: self];
	self.htmlView.scalesPageToFit = NO;
	[self.htmlView loadHTMLString: htmlString baseURL: [NSURL fileURLWithPath: [[NSBundle mainBundle] resourcePath] isDirectory:YES]];
}

//This is called on an error, grab all error information, but we only use some of it

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
	//	NSLog(@"Starting web load didFailLoadWithError");
	//	NSLog([[[webView request] URL] absoluteString]);
	//	NSLog(@"flusFaileh");
	[[UIApplication sharedApplication] setNetworkActivityIndicatorVisible: NO];
	
	UIAlertView * dialog = [UIAlertView alloc];
	NSString * ld = [error localizedDescription];
	NSString * lfr = (NSString *)[error localizedFailureReason];
	NSArray * lro = [error localizedRecoveryOptions];
	NSString * lrs = [error localizedRecoverySuggestion];
#pragma unused(ld,lfr,lro,lrs)
	NSString * okText = NSLocalizedString(@"Ok", nil);
	dialog = [dialog initWithTitle: [error localizedDescription] 
				  message: [error localizedRecoverySuggestion]  delegate: nil
		cancelButtonTitle: okText  otherButtonTitles: nil];
	[dialog show];
	[dialog release];
}

//Check type of URL
- (BOOL) isURL: (NSURLRequest *) request type: (NSString *) type {
	NSRange howMuch;
	howMuch.location=0;
	howMuch.length= type.length;
	NSComparisonResult equal = [[[request URL] scheme] compare: type options: NSCaseInsensitiveSearch  range: howMuch];
	return (equal == NSOrderedSame) ? YES : NO;
}

//This enables the check to see if Restrictions has Safari turned off, if so no internet
//We also have to be aware of about: and file: special cases
//We also have to adjust the view to swap in the status bar
//We should not have to adjust the webview frame, but we have to, this appears to be a bug in the UIWebView logic

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
	BOOL what = [[UIApplication sharedApplication] canOpenURL: [request URL]];
	if (what == NO) {
		
		NSRange howMuch;
		howMuch.location=0;
		howMuch.length=5;
		
		if ([self isURL: request type: @"about"]) return YES;
		if ([self isURL: request type: @"file"]) return YES;
		return NO;
	}
	return YES;
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
	self.htmlView = nil;
}

- (void)dealloc {
    [super dealloc];
	[htmlView release];
}


@end
