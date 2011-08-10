//
//  CSCWebsiteViewController.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-01-31.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <MessageUI/MFMailComposeViewController.h>
#import "SqueakProxy.h"

@interface CSCWebsiteViewController : UIViewController  <UIWebViewDelegate,MFMailComposeViewControllerDelegate>  {
	IBOutlet UIWebView *webView;
	IBOutlet UIBarButtonItem *backArrow;
	IBOutlet UIBarButtonItem *forwardArrow;
	NSURL *webUrl;
	SqueakProxy *squeakProxy;
}
@property (nonatomic, retain) UIWebView *webView;
@property (nonatomic, retain) NSURL *webUrl;
@property (nonatomic, assign) UIBarButtonItem *backArrow;
@property (nonatomic, assign) UIBarButtonItem *forwardArrow;
@property (nonatomic, assign) SqueakProxy *squeakProxy;

- (IBAction) goBackToPlayer:(id)sender;
- (IBAction) helpSystem:(id)sender;
- (id) initWithNibName:(NSString *)name bundle:(NSBundle *)bundle website:(NSURL *)aWebURL;
- (id) initWithNibName:(NSString *)name bundle:(NSBundle *)bundle website:(NSURL *)aWebURL squeakProxy: aProxy;
- (void) grabURLInBackground: (NSURL *) url;
- (BOOL)holdsIntegerValue: (NSString *) check;

@end
