//
//  AboutViewController.m
//
//  Created by John M McIntosh on 10-01-18.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "AboutViewController.h"
#import "Reachability.h"
#import "ScratchIPhoneAppDelegate.h"
#warning  must look at "Remote sensor connections enabled" issue

extern ScratchIPhoneAppDelegate *gDelegateApp;

@implementation AboutViewController
@synthesize website;

 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
	return self;
}



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void) viewWillAppear:(BOOL)animated {
	if(!(UIUserInterfaceIdiomPad == UI_USER_INTERFACE_IDIOM())) 
		[gDelegateApp.viewController setNavigationBarHidden: NO animated: YES];			
	[super viewWillAppear: animated];	
}

- (IBAction) clickWebSite:(id)sender {
	[[UIApplication sharedApplication] openURL: [NSURL URLWithString: @"http://scratch.mit.edu/signup"]];
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	[super viewDidUnload];
	self.website = nil;
}


- (void)dealloc {
    [super dealloc];
	[website release];
}


@end
