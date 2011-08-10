//
//  CSCAlertListDialog.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-01-31.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "CSCAlertListDialog.h"
#import "sq.h"

extern struct	VirtualMachine* interpreterProxy;


@implementation CSCAlertListDialog
@synthesize alertView,buttonIndexs,resultString;

- (CSCAlertListDialog *) initTitle: (NSString *) title message: (NSString *) message semaIndex: (NSInteger) si {
	alertView = [UIAlertView alloc];
	[self.alertView initWithTitle: title message: message delegate: self cancelButtonTitle: nil otherButtonTitles: nil];
	semaIndex = si;
	self.buttonIndexs = [NSMutableDictionary dictionaryWithCapacity: 10];
	return self;
}

- (void) show {
	[self.alertView show];
}

- (void) addButtonWithTitle: (NSString *) buttonString {
	NSInteger buttonNumber = [self.alertView addButtonWithTitle: buttonString];
	[self.buttonIndexs setObject: buttonString forKey: [NSNumber numberWithInteger: buttonNumber]];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
	self.resultString = [self.buttonIndexs objectForKey: [NSNumber numberWithInteger: buttonIndex]];
	interpreterProxy->signalSemaphoreWithIndex(semaIndex);
}

- (void)dealloc {
	[alertView release];
	[buttonIndexs release];
	[resultString release];
	[super dealloc];
}

@end
