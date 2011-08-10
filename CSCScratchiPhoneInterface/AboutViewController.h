//
//  AboutViewController.h
//
//  Created by John M McIntosh on 10-01-18.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import <MessageUI/MFMailComposeViewController.h>

@interface AboutViewController : UIViewController   {
	IBOutlet UIButton *website;
}

@property (nonatomic, retain) IBOutlet UIButton *website;

- (IBAction) clickWebSite:(id)sender;

@end
