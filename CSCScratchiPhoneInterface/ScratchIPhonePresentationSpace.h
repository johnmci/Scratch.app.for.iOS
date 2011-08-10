//
//  ScratchIPhonePresentationSpace.h
//  SqueakPureObjc
//
//  Created by John M McIntosh on 10-02-15.
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "SqueakUIController.h"
#import "ScratchRepeatKeyMetaData.h"
#import "ScratchPresentationUITextField.h"

@interface ScratchIPhonePresentationSpace : UIViewController <UIScrollViewDelegate, UITextFieldDelegate, UIAlertViewDelegate, UIPopoverControllerDelegate> {
	IBOutlet UIScrollView *scrollView;
	IBOutlet UITextView *textView;
	IBOutlet SqueakUIController *scrollViewController;
	IBOutlet UIButton *infoButton;
	IBOutlet UIButton *shoutGoButton;
	IBOutlet UIButton *stopAllButton;
	IBOutlet UIButton *shoutGoLandscapeButton;
	IBOutlet UIButton *stopAllLandscapeButton;
	IBOutlet UIButton *webButton;
	IBOutlet UIButton *padLockButton;
	IBOutlet ScratchPresentationUITextField *textField;
	IBOutlet UIView *landscapeToolBar;
	IBOutlet UIView *landscapeToolBar2;
	UIPopoverController *popUpInfoViewController;
	NSString*	pathToProjectFile;
	NSTimer *activityTimer;
	BOOL	firstView;
	BOOL	projectIsLive;
	NSInteger	characterCounter;
	unichar	debugCharacters[5];
	NSMutableDictionary *repeatKeyDict;
	NSString *rememberTextOnMemoryWarning;
	BOOL	thereIsNoInternet;
}
@property (nonatomic, retain) IBOutlet UIScrollView *scrollView;
@property (nonatomic, retain) IBOutlet UITextView *textView;
@property (nonatomic, retain) IBOutlet SqueakUIController *scrollViewController;
@property (nonatomic, retain) IBOutlet UIButton *infoButton;
@property (nonatomic, retain) IBOutlet UIButton *shoutGoButton;
@property (nonatomic, retain) IBOutlet UIButton *stopAllButton;
@property (nonatomic, retain) IBOutlet UIButton *shoutGoLandscapeButton;
@property (nonatomic, retain) IBOutlet UIButton *stopAllLandscapeButton;
@property (nonatomic, retain) IBOutlet UIButton *webButton;
@property (nonatomic, retain) IBOutlet UIButton *padLockButton;
@property (nonatomic, retain) IBOutlet ScratchPresentationUITextField *textField;
@property (nonatomic, retain) NSTimer *activityTimer;
@property (nonatomic, retain) NSString*	pathToProjectFile;
@property (nonatomic, retain) NSMutableDictionary *repeatKeyDict;
@property (nonatomic, retain) NSString *rememberTextOnMemoryWarning;
@property (nonatomic, retain) UIView *landscapeToolBar;
@property (nonatomic, retain) UIView *landscapeToolBar2;
@property (nonatomic, assign) BOOL thereIsNoInternet;
@property (nonatomic, retain) 	UIPopoverController *popUpInfoViewController;

- (IBAction) shoutGo:(id)sender;
- (IBAction) stopAll:(id)sender;
- (IBAction) clickInfo:(id)sender;
- (IBAction) takeMeBackToTheWeb:(id)sender;
- (IBAction) keyUpArrow:(id)sender;
- (IBAction) keyDownArrow:(id)sender;
- (IBAction) keyLeftArrow:(id)sender;
- (IBAction) keyRightArrow:(id)sender;
- (IBAction) keyTouchUp:(id)sender;
- (IBAction) keySpace: (id) sender;
- (IBAction) operatePadLock: (id) sender;
- (void) fireAlphaToOne;
- (void) deletePendingTimer;
- (void) pushCharacter: (NSString*) string;
- (void) chooseThisProjectDontRun: (NSString *) file;
- (void) chooseThisProjectRun: (NSString *) file;
- (void) goBackToPlayerNoAlphaAlter: (NSString*) pathToProjectFile;
- (void) goBackToPlayer;
- (void) showAlertViaTimer;
- (void) startRepeatKeyProcess: (unichar) character for: (id) sender;
- (void) startRepeatKeyAction: (NSString*) string  for: (id) sender;
@end
