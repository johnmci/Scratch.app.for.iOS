//
//  CSCAlertDialog.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-01-31.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface CSCAlertDialog : NSObject <UIAlertViewDelegate> {
	NSInteger yesIndex;
	NSInteger noIndex;
	NSInteger okIndex;
	NSInteger buttonPicked;
	NSInteger semaIndex;
}

@property (nonatomic,assign) NSInteger buttonPicked;

- (CSCAlertDialog *) initTitle: (NSString *) title message: (NSString *) message yes: (BOOL) yesFlag no: (BOOL) noFlag 
						  okay: (BOOL) okFlag cancel: (BOOL) cancelFlag semaIndex: (NSInteger) semaIndex;
@end
