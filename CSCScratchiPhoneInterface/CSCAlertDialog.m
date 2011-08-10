//
//  CSCAlertDialog.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-01-31.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "CSCAlertDialog.h"
#import "sq.h"

extern struct	VirtualMachine* interpreterProxy;


@implementation CSCAlertDialog
@synthesize buttonPicked;

- (CSCAlertDialog *) initTitle: (NSString *) title message: (NSString *) message yes: (BOOL) yesFlag no: (BOOL) noFlag 
						  okay: (BOOL) okFlag cancel: (BOOL) cancelFlag semaIndex: (NSInteger) si {
	UIAlertView *alertView = [UIAlertView alloc];
	NSString *yes = NSLocalizedString(@"Yes",nil);
	NSString *no = NSLocalizedString(@"No",nil);
	NSString *cancel = NSLocalizedString(@"Cancel",nil);
	NSString *ok = NSLocalizedString(@"Ok",nil);
	alertView = [alertView initWithTitle: title message: message delegate: self cancelButtonTitle: (cancelFlag ? cancel : nil) otherButtonTitles: nil];
	yesIndex = yesFlag ? [alertView addButtonWithTitle: yes] : -1;
	noIndex = noFlag ? [alertView addButtonWithTitle: no] : -1;
	okIndex = okFlag ? [alertView addButtonWithTitle: ok] : -1;
	semaIndex = si;
	[alertView show];
	[alertView release];
	return self;
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (alertView.cancelButtonIndex == buttonIndex) {
		self.buttonPicked = -1;
	}
	if (yesIndex == buttonIndex) {
		self.buttonPicked = 1;
	}
	if (noIndex == buttonIndex) {
		self.buttonPicked = 0;
	}
	if (okIndex == buttonIndex) {
		self.buttonPicked = 1;
	}
	
	interpreterProxy->signalSemaphoreWithIndex(semaIndex);
}


- (void)dealloc {
	[super dealloc];
}

@end
