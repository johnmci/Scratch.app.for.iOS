//
//  ScratchRepeatKeyMetaData.m
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-03-10.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "ScratchRepeatKeyMetaData.h"


@implementation ScratchRepeatKeyMetaData
@synthesize string, senderHash;

- (void)dealloc {
	[super dealloc];
	[string release];
	[senderHash release];
}
@end
