//
//  CSCNagDialogForScratchAccount.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-20.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "CSCNagDialogForScratchAccount.h"


@implementation CSCNagDialogForScratchAccount

+ (id) showNagMessageIfRequired {
	CSCNagDialogForScratchAccount *nag = [CSCNagDialogForScratchAccount new];
	[nag nagMessage];
	return [nag autorelease];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == 0) 
		return;
	[[UIApplication sharedApplication] openURL: [NSURL URLWithString: @"http://scratch.mit.edu/signup"]];
}

- (void) nagMessage {
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	BOOL notFirstTimeUser = [standardUserDefaults boolForKey:@"firstTimeUsage"];
	if (notFirstTimeUser == YES) 
		return;
	[standardUserDefaults setBool: YES forKey:@"firstTimeUsage"];
	[standardUserDefaults synchronize];
	UIAlertView *alertView = [UIAlertView alloc];
	NSString *checking = NSLocalizedString(@"checking",nil);
	NSString *checkingMsg = NSLocalizedString(@"checkingMsg",nil);
	NSString *play = NSLocalizedString(@"play",nil);
	NSString *create = NSLocalizedString(@"create",nil);
	alertView = [alertView initWithTitle: checking 
					 message: checkingMsg delegate: self cancelButtonTitle: play otherButtonTitles: create,nil];
	[alertView show];
	[alertView release];
}

@end
