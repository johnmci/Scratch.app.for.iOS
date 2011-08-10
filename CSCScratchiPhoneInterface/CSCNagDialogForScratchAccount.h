//
//  CSCNagDialogForScratchAccount.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-20.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

@interface CSCNagDialogForScratchAccount : NSObject <UIAlertViewDelegate> {

}

+ (id) showNagMessageIfRequired;
- (void) nagMessage;

@end
