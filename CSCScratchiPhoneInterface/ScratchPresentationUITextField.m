//
//  ScratchPresentationUITextField.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-03-11.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "ScratchPresentationUITextField.h"


@implementation ScratchPresentationUITextField
- (BOOL)canPerformAction:(SEL)action withSender:(id)sender {
	if (action == @selector(paste:))
		return YES;
    else
		return NO;
}
@end

//UIResponderStandardEditActions