//
//  AboutInternalViewController.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-25.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "AboutViewController.h"


@interface AboutInternalViewController : AboutViewController <UIWebViewDelegate> {
	IBOutlet UIWebView *htmlView;

}
@property (nonatomic, retain) IBOutlet UIWebView *htmlView;
@end
