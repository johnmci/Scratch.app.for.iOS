//
//  ScratchRepeatKeyMetaData.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-03-10.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface ScratchRepeatKeyMetaData : NSObject {
	NSString *string;
	NSNumber *senderHash;
}
@property (nonatomic, retain)  NSString *string;
@property (nonatomic, retain)  NSNumber *senderHash;

@end
