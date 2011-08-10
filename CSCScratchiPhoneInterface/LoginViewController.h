//
//  LoginViewController.h
//
//  Copyright 2010 Corporate Smalltalk Consulting Ltd. All rights reserved.
//

#import "CSCNagDialogForScratchAccount.h"

@interface LoginViewController : UIViewController <UITextFieldDelegate> {	
	// UI Components
	IBOutlet UITextField *usernameIn;
	IBOutlet UITextField *passwordIn;
	
	IBOutlet UILabel *rememberMe;
	IBOutlet UILabel *username;
	IBOutlet UILabel *password;
	IBOutlet UIButton *checkButton;
	IBOutlet UIButton *loginButton;
	
	IBOutlet UIScrollView *scrollTextFieldAndLogo;
	CSCNagDialogForScratchAccount *nagMessageAlert;
	BOOL	squeakVMIsReady;
	BOOL	uiThinksLoginButtonCouldBeEnabled;
	BOOL	fieldWasAdjustUpwardsAlready;
}

@property (nonatomic, retain) IBOutlet UIButton *checkButton;
@property (nonatomic, retain) UITextField *usernameIn;
@property (nonatomic, retain) UITextField *passwordIn;
@property (nonatomic, retain) UILabel *username;
@property (nonatomic, retain) UILabel *password;
@property (nonatomic, retain) UIScrollView *scrollTextFieldAndLogo;
@property (nonatomic, retain) UILabel *rememberMe;
@property (nonatomic, retain) IBOutlet UIButton *loginButton;
@property (nonatomic, retain) CSCNagDialogForScratchAccount *nagMessageAlert;

- (IBAction) clickLoginButton:(id)sender;
- (IBAction) clickCheckButton:(id)sender; 
- (IBAction) clickAbout:(id)sender;
- (void) putTextBackToOriginalPlace;
- (void) squeakVMIsReadyNowOnMainThread;

- (void) squeakVMIsReadyNow;
@end
