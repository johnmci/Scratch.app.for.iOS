//
//  LoginViewControlleriPad.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-03-02.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "LoginViewController.h"

@interface LoginViewControlleriPad : LoginViewController <UIWebViewDelegate> {
	IBOutlet UIButton *website;
	IBOutlet UIWebView *htmlView;
	IBOutlet UIWebView *scratchSiteWebView;
}
@property (nonatomic, retain) IBOutlet UIButton *website;
@property (nonatomic, retain) IBOutlet UIWebView *htmlView;
@property (nonatomic, retain) IBOutlet UIWebView *scratchSiteWebView;

- (IBAction) clickWebSite:(id)sender;

@end
