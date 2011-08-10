//
//  CSCAlertListDialog.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-01-31.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface CSCAlertListDialog : NSObject <UIAlertViewDelegate> {
	UIAlertView *alertView;
	NSMutableDictionary *buttonIndexs;
	NSString	*resultString;
	NSInteger semaIndex;
}
@property (nonatomic,retain) UIAlertView *alertView;
@property (nonatomic,retain) NSMutableDictionary *buttonIndexs;
@property (nonatomic,retain) NSString	*resultString;

- (CSCAlertListDialog *) initTitle: (NSString *) title message: (NSString *) message semaIndex: (NSInteger) si;
- (void) addButtonWithTitle: (NSString *) buttonString;
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex;
- (void) show;
@end
