//
//  GameDetailViewController.m
//  ESCDocent
//
//  Created by Andy So on 3/18/14.
//  Copyright (c) 2014 ESI. All rights reserved.
//

#import "GameDetailViewController.h"
#import "PluginInterface.h"
#import "GameModel.h"

typedef NS_ENUM(NSUInteger, DifficultyType) {
    kEasiest = 0,
    kEasy,
    kMedium,
    kHard,
    kHardest
};

typedef NS_ENUM(NSUInteger, RoundType) {
    kRound1 = 0,
    kRound2,
    kRound3,
    kRound4,
    kRound5
};

float const kStartingHeightSettings = 200;
float const kStartingHeightRound = 198;
float const kStartingHeightDifficulty = 241;
float const kStartingHeightAdvancedSettings = 640;
float const kKeyboardHeight = 216.0;

int const kRoundsTag = 1;
int const kDifficultyTag = 2;
int const kStartButtonTag = 3;
int const kStopButtonTag = 4;
int const kPauseButtonTag = 5;
int const kApplyButtonTag = 6;
int const kNextButtonTag = 7;

int const kModeOneTag = 8;
int const kModeTwoTag = 9;
int const kModeThreeTag = 10;

NSString *const kConnected = @"Connected";
NSString *const kDisconnected = @"Not Connected";

NSString *const kEasiestText = @"Easiest";
NSString *const kEasyText = @"Easy";
NSString *const kMediumText = @"Medium";
NSString *const kHardText = @"Hard";
NSString *const kHardestText = @"Hardest";

NSString *const kRound1Text = @"1";
NSString *const kRound2Text = @"2";
NSString *const kRound3Text = @"3";
NSString *const kRound4Text = @"4";
NSString *const kRound5Text = @"5";

NSString *const kNoModeSelected = @"-1";
NSString *const kNoParams = @"";

@interface GameDetailViewController ()

// Game Details Properties

@property (nonatomic, strong) NSString *gameTitle;
@property (nonatomic, strong) NSString *gameID;
@property (nonatomic, strong) UILabel *gameTitleLabel;

@property (nonatomic, strong) UIScrollView *scrollView;

@property (nonatomic, weak) GameModel *model;
@property (nonatomic, strong) NSTimer *timer;

// Connection Details Properties
@property (nonatomic, strong) UILabel *currentConnection;

// Loading Settings
@property (nonatomic, strong) UILabel *loadingLabel;
@property (nonatomic, strong) UIActivityIndicatorView *spinner;


// Basic Settings Properties
@property (nonatomic) RoundType selectedRound;
@property (nonatomic) DifficultyType selectedDifficulty;

@property (nonatomic) BOOL gameStarted;

@property (nonatomic, strong) UILabel *selectedRoundLabel;
@property (nonatomic, strong) UILabel *selectedDifficultyLabel;

@property (nonatomic, strong) UIButton *startButton;
@property (nonatomic, strong) UIButton *pauseButton;
@property (nonatomic, strong) UIButton *stopButton;
@property (nonatomic, strong) UIButton *nextButton;
@property (nonatomic, strong) UIButton *applyButton;
@property (nonatomic, strong) UIButton *resetButton;

@property (nonatomic, strong) UIButton *roundSelectButton;
@property (nonatomic, strong) UIButton *difficultySelectButton;

@property (nonatomic, strong) NSArray *settingsList;
@property (nonatomic, strong) UITableView *tableView;

// Advanced Settings Properties
@property (nonatomic, strong) UILabel *modeOneLabel;
@property (nonatomic, strong) UILabel *modeTwoLabel;
@property (nonatomic, strong) UILabel *modeThreeLabel;

@property (nonatomic, strong) NSArray *modeOneList;
@property (nonatomic, strong) NSArray *modeTwoList;
@property (nonatomic, strong) NSArray *modeThreeList;

@property (nonatomic, strong) NSString *modeOneSelected;
@property (nonatomic, strong) NSString *modeTwoSelected;
@property (nonatomic, strong) NSString *modeThreeSelected;

@property (nonatomic, strong) UISegmentedControl *modeOne;
@property (nonatomic, strong) UISegmentedControl *modeTwo;
@property (nonatomic, strong) UISegmentedControl *modeThree;

@property (nonatomic, strong) UILabel *parameterOneLabel;
@property (nonatomic, strong) UILabel *parameterTwoLabel;
@property (nonatomic, strong) UILabel *parameterThreeLabel;

@property (nonatomic, strong) UITextField *parameterOneTextField;
@property (nonatomic, strong) UITextField *parameterTwoTextField;
@property (nonatomic, strong) UITextField *parameterThreeTextField;

@property (nonatomic, strong) UILabel *advanceSettingsDivider;
@property (nonatomic) CGPoint offset;

@end

@implementation GameDetailViewController

// Initialize the view controller with a game title and create the settings view
- (id)initWithGameTitle: (NSString *)title andGameID: (NSString *)gameID
{
    self = [super initWithNibName:nil bundle:nil];
    if (self) {
        // Custom initialization
        self.view.backgroundColor = [UIColor whiteColor];
        
        [self addConnectionView];

        self.scrollView = [[UIScrollView alloc]initWithFrame:CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height)];
        self.scrollView.contentSize = CGSizeMake(self.view.frame.size.width, self.view.frame.size.height * 2);
        self.scrollView.pagingEnabled = YES;
        self.scrollView.delegate = self;
        [self.view addSubview:self.scrollView];
//        NSLog(@"My view frame: %@", NSStringFromCGRect(self.scrollView.frame));
//        NSLog(@"My content size: %@", NSStringFromCGSize(self.scrollView.contentSize));
        
        self.gameTitle = title;
        self.gameID = gameID;
        self.model = [GameModel sharedInstance];

        self.title = @"Settings";
        
        if (self.model.isGameLoaded && [self.model.gameIDToBeLoaded isEqualToString: self.gameID]){
            [self showSettingsView];
        } else {
            [self showLoadingView];
        }

        [self checkConnection];
        
        self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"< Menu"
                                                                                 style:UIBarButtonItemStylePlain
                                                                                target:self
                                                                                action:@selector(dismissView)];
        
        self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Quit"
                                                                                  style:UIBarButtonItemStylePlain
                                                                                 target:self
                                                                                 action:@selector(showQuitAlert)];
        
        //[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector (textFieldFinished:) name: UIKeyboardDidHideNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector (showSettingsView) name: kGameLoadedNotification object:nil];

        // Listen to game end message notification
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(sendQuitMessage) name:kGameEndNotification object:nil];
        
        // Listen for advanced settings update
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector (updateAdvancedSettingsParameters) name: kGameAdvancedSettingsNotification object:nil];

    }
    return self;
}

- (void)showLoadingView {
    
    self.spinner = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
    [self.spinner setCenter:CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2)];
    [self.view addSubview:self.spinner];
    [self.spinner startAnimating];
    
    float loadingLabelWidth = 200;
    self.loadingLabel = [[UILabel alloc]initWithFrame:CGRectMake((self.view.frame.size.width - loadingLabelWidth)/2, (self.view.frame.size.height/2) + 20, loadingLabelWidth, 50)];
    NSString *loadingText = [NSString stringWithFormat:@"Loading %@ ...", self.gameTitle];
    self.loadingLabel.text = loadingText;
    [self.view addSubview:self.loadingLabel];
}


- (void)showSettingsView {
    [self.loadingLabel removeFromSuperview];
    [self.spinner stopAnimating];
    [self.spinner removeFromSuperview];
    
    [self addSettings];
    [self addAdvancedSettings];
    [self addConnection];
    
    if (self.model.isGameStarted && [self.model.gameIDToBeLoaded isEqualToString: self.gameID]){
        [self showGameRunningState];
    } else {
        [self showStartButton];
    }

}


//! Creates a connection update timer when the view will appear
- (void)viewWillAppear:(BOOL)animated {
    
    // Check for Connection Status
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(checkConnection) userInfo:nil repeats:YES];

}

//! Removes the check connection timer when the view will disappear
- (void)viewWillDisappear:(BOOL)animated {
    [self.timer invalidate];
    self.timer = nil;

}

//! Return to the root Docent view controller
- (void)dismissView {
    [self.navigationController popToRootViewControllerAnimated:YES];
}

#pragma mark Connection Status

//! Check connection and update the connection label
- (void)checkConnection {
    [self.model checkConnection];
    
    if (isConnected()){
        self.currentConnection.backgroundColor = [UIColor greenColor];
        self.currentConnection.text = kConnected;
        
    } else {
        self.currentConnection.backgroundColor = [UIColor redColor];
        self.currentConnection.text = kDisconnected;
    }
}

//! Create a Connection label that will show whether or not the Docent is connected to the server
- (void)addConnection {
    CGRect titleRect = CGRectMake(0, 44, self.view.frame.size.width, 40);
    self.currentConnection = [[UILabel alloc] initWithFrame:titleRect];
    self.currentConnection.textColor = [UIColor whiteColor];
    self.currentConnection.opaque = YES;
    self.currentConnection.font = [UIFont boldSystemFontOfSize:18];
    self.currentConnection.textAlignment = NSTextAlignmentCenter;
    [self.view addSubview:self.currentConnection];
}

- (void)addConnectionView {
    CGRect titleRect = CGRectMake(0, 44, self.view.frame.size.width, 40);
    UILabel *connectLabel = [[UILabel alloc] initWithFrame:titleRect];
    [self.view addSubview:connectLabel];
}

#pragma mark Settings

//! Show the game settings in a table view
- (void)addSettings {
    
    NSArray *settingsArray = @[@"Select Round", @"Select Difficulty"];
    self.settingsList = [[NSArray alloc]initWithArray:settingsArray];
    
    UILabel *titleLabel = [[UILabel alloc]initWithFrame:CGRectMake(12, 100, self.view.frame.size.width, 50)];
    if (!self.gameTitle)
        titleLabel.text = self.gameID;
    else
        titleLabel.text = self.gameTitle;
    titleLabel.textColor = [UIColor blackColor];
    titleLabel.textAlignment = NSTextAlignmentLeft;
    [self.scrollView addSubview:titleLabel];
    
    self.tableView = [[UITableView alloc] initWithFrame:CGRectMake(0,kStartingHeightSettings,self.view.frame.size.width, 100)style:UITableViewStylePlain];
    self.tableView.delegate = self;
    self.tableView.dataSource = self;
    self.tableView.scrollEnabled = NO;
    [self.tableView reloadData];
    [self.scrollView addSubview: self.tableView];
    
    [self addSelectedRoundsLabel];
    [self addSelectedDifficultyLabel];
    
    [self addStartGameButton];
    [self addStopGameButton];
    [self addPauseGameButton];
    [self addNextButton];
;
}

#pragma mark Buttons

//! Add the Select Round Label
- (void)addSelectedRoundsLabel {
    self.selectedRoundLabel = [[UILabel alloc]initWithFrame:CGRectMake(self.view.frame.size.width - 100, kStartingHeightRound, 100, 50)];
    self.selectedRoundLabel.text = kRound1Text;
    self.selectedRound = kRound1;
    self.selectedRoundLabel.textColor = [UIColor blackColor];
    self.selectedRoundLabel.textAlignment = NSTextAlignmentLeft;
    [self.scrollView addSubview:self.selectedRoundLabel];
    
}

//! Add the Select Difficulty Label
- (void)addSelectedDifficultyLabel {
    
    self.selectedDifficultyLabel = [[UILabel alloc]initWithFrame:CGRectMake(self.view.frame.size.width - 100, kStartingHeightDifficulty, 100, 50)];
    self.selectedDifficultyLabel.text = kEasyText;
    self.selectedDifficulty = kEasy;

    self.selectedDifficultyLabel.textColor = [UIColor blackColor];
    self.selectedDifficultyLabel.textAlignment = NSTextAlignmentNatural;
    [self.scrollView addSubview:self.selectedDifficultyLabel];
    
}

//! Add the Next Button
- (void)addNextButton {
    self.nextButton = [[UIButton alloc]initWithFrame:CGRectMake(10, self.startButton.frame.origin.y + 60, 300, 50)];
    self.nextButton.backgroundColor = [UIColor grayColor];
    [self.nextButton setTitle:@"Next" forState: UIControlStateNormal];
    self.nextButton.tag = kNextButtonTag;
    [self.nextButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];

    [self.scrollView addSubview:self.nextButton];
    [self.nextButton addTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
    
}

//! Add the Start Button
- (void)addStartGameButton {
    self.startButton = [[UIButton alloc]initWithFrame:CGRectMake(10, self.view.frame.size.height - 115, 300, 50)];
    self.startButton.backgroundColor = [UIColor greenColor];
    [self.startButton setTitle:@"Start Game" forState: UIControlStateNormal];
    self.startButton.tag = kStartButtonTag;
    self.startButton.alpha = 0.0;

    [self.scrollView addSubview:self.startButton];
    [self.startButton addTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
    
}

//! Add Pause Button
- (void)addPauseGameButton {
    self.pauseButton = [[UIButton alloc]initWithFrame:CGRectMake(10, self.startButton.frame.origin.y, 300, 50)];
    self.pauseButton.backgroundColor = [UIColor purpleColor];
    
    self.pauseButton.tag = kPauseButtonTag;
    self.pauseButton.alpha = 0.0;

    [self.scrollView addSubview:self.pauseButton];
    [self.pauseButton addTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
    
}

//! Add the Stop Button
- (void)addStopGameButton {
    
    self.stopButton = [[UIButton alloc]initWithFrame:CGRectMake(10, self.startButton.frame.origin.y - 60, 300, 50)];
    self.stopButton.backgroundColor = [UIColor redColor];
    [self.stopButton setTitle:@"Stop Game" forState: UIControlStateNormal];
    self.stopButton.tag = kStopButtonTag;
    
    [self.scrollView addSubview:self.stopButton];
    self.stopButton.alpha = 0.0;
    [self.stopButton addTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
    
}

//! Add the Apply Button
- (void)addApplyButton {
    self.applyButton = [[UIButton alloc]initWithFrame:CGRectMake(10, self.scrollView.contentSize.height - 115, 300, 50)];
    self.applyButton.backgroundColor = [UIColor purpleColor];
    [self.applyButton setTitle:@"Apply" forState: UIControlStateNormal];
    self.applyButton.tag = kApplyButtonTag;
    
    [self.applyButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];

    [self.scrollView addSubview:self.applyButton];
    [self.applyButton addTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
}

//! Add the Reset Button
- (void)addResetButton {
    self.resetButton = [[UIButton alloc]initWithFrame:CGRectMake(10, self.scrollView.contentSize.height - 55, 300, 50)];
    self.resetButton.backgroundColor = [UIColor grayColor];
    [self.resetButton setTitle:@"Reset" forState: UIControlStateNormal];
    
    [self.resetButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];
    
    [self.scrollView addSubview:self.resetButton];
    [self.resetButton addTarget:self action:@selector(resetSettings:) forControlEvents:UIControlEventTouchUpInside];
}

- (void)showStartButton {
    self.startButton.alpha = 1.0;

}

- (void)showStopButton {
    self.stopButton.alpha = 1.0;

}

- (void)showPauseButton {
    self.pauseButton.alpha = 1.0;
    
    if (self.model.gamePaused){
        [self.pauseButton setTitle:@"Unpause Game" forState: UIControlStateNormal];
    } else {
        [self.pauseButton setTitle:@"Pause Game" forState: UIControlStateNormal];
    }
    
}

- (void)hideStartButton {
    self.startButton.alpha = 0.0;

}

- (void)hideStopButton {
    self.stopButton.alpha = 0.0;
    
}

- (void)hidePauseButton {
    self.pauseButton.alpha = 0.0;
    
}

//! Send a message if the button is pressed
- (void)buttonPressed:(UIButton *)sender {
    if (sender.tag == kStartButtonTag){
        [self sendStartMessage];
    } else if (sender.tag == kPauseButtonTag){
        [self sendPauseMessage];
    } else if (sender.tag == kStopButtonTag){
        [self sendStopMessage];
    } else if (sender.tag == kApplyButtonTag){
        [self sendApplyMessage];
    } else if (sender.tag == kNextButtonTag){
        [self sendNextMessage];
    }
}

#pragma mark Advanced Settings

- (void)addAdvancedSettings {

    // Add labels
    self.modeOneLabel = [[UILabel alloc]initWithFrame:CGRectMake(20, kStartingHeightAdvancedSettings + 20, 200, 50)];
    self.modeOneLabel.text = @"Mode 1";
    
    self.modeTwoLabel = [[UILabel alloc]initWithFrame:CGRectMake(20, kStartingHeightAdvancedSettings + 80, 200, 50)];
    self.modeTwoLabel.text = @"Mode 2";

    
    self.modeThreeLabel = [[UILabel alloc]initWithFrame:CGRectMake(20, kStartingHeightAdvancedSettings + 140, 200, 50)];
    self.modeThreeLabel.text = @"Mode 3";

    [self.scrollView addSubview: self.modeOneLabel];
    [self.scrollView addSubview: self.modeTwoLabel];
    [self.scrollView addSubview: self.modeThreeLabel];
    
    self.parameterOneLabel = [[UILabel alloc]initWithFrame:CGRectMake(20, kStartingHeightAdvancedSettings + 200, 200, 50)];
    self.parameterOneLabel.text = @"Param 1";
    
    self.parameterTwoLabel = [[UILabel alloc]initWithFrame:CGRectMake(20, kStartingHeightAdvancedSettings + 260, 200, 50)];
    self.parameterTwoLabel.text = @"Param 2";
    
    self.parameterThreeLabel = [[UILabel alloc]initWithFrame:CGRectMake(20, kStartingHeightAdvancedSettings + 320, 200, 50)];
    self.parameterThreeLabel.text = @"Param 3";
    
    [self.scrollView addSubview: self.parameterOneLabel];
    [self.scrollView addSubview: self.parameterTwoLabel];
    [self.scrollView addSubview: self.parameterThreeLabel];
    
    //! Mode 1
    self.modeOneList = @[@"A", @"B", @"C"];
    self.modeOne = [[UISegmentedControl alloc]initWithItems:self.modeOneList];
    self.modeOne.frame = CGRectMake(self.view.frame.size.width / 3, kStartingHeightAdvancedSettings + 20, 200 , 50);
    self.modeOne.tintColor = [UIColor blackColor];
    [self.scrollView addSubview:self.modeOne];
    self.modeOne.tag = kModeOneTag;
    [self.modeOne addTarget:self
                         action:@selector(modeSelected:)
               forControlEvents:UIControlEventValueChanged];
    
    //! Mode 2
    self.modeTwoList = @[@"α", @"β", @"γ"];
    self.modeTwo = [[UISegmentedControl alloc]initWithItems:self.modeTwoList];
    self.modeTwo.frame = CGRectMake(self.view.frame.size.width / 3, kStartingHeightAdvancedSettings + 80, 200 , 50);
    self.modeTwo.tintColor = [UIColor blackColor];
    [self.scrollView addSubview:self.modeTwo];
    self.modeTwo.tag = kModeTwoTag;
    [self.modeTwo addTarget:self
                     action:@selector(modeSelected:)
           forControlEvents:UIControlEventValueChanged];

    //! Mode 3
    self.modeThreeList = @[@"I", @"II", @"III"];
    self.modeThree = [[UISegmentedControl alloc]initWithItems:self.modeThreeList];
    self.modeThree.frame = CGRectMake(self.view.frame.size.width / 3, kStartingHeightAdvancedSettings + 140, 200 , 50);
    self.modeThree.tintColor = [UIColor blackColor];
    [self.scrollView addSubview:self.modeThree];
    self.modeThree.tag = kModeThreeTag;
    [self.modeThree addTarget:self
                     action:@selector(modeSelected:)
           forControlEvents:UIControlEventValueChanged];
    
    
    self.modeOneSelected = kNoModeSelected;
    self.modeTwoSelected = kNoModeSelected;
    self.modeThreeSelected = kNoModeSelected;

    //! Parameter 1 Text Field
    self.parameterOneTextField = [[UITextField alloc]initWithFrame:CGRectMake(self.view.frame.size.width / 3, kStartingHeightAdvancedSettings + 200, 200 , 50)];
    self.parameterOneTextField.backgroundColor = [UIColor lightGrayColor];
    [self.scrollView addSubview:self.parameterOneTextField];

    self.parameterOneTextField.delegate = self;
    self.parameterOneTextField.leftViewMode = UITextFieldViewModeAlways;
    [self.parameterOneTextField setReturnKeyType:UIReturnKeyDone];
    self.parameterOneTextField.textAlignment = NSTextAlignmentCenter;
    [self.parameterOneTextField addTarget:self
                                   action:@selector(textFieldExit:)
                         forControlEvents:UIControlEventEditingDidEndOnExit];
    
    //! Parameter 2 Text Field
    self.parameterTwoTextField = [[UITextField alloc]initWithFrame:CGRectMake(self.view.frame.size.width / 3, kStartingHeightAdvancedSettings + 260, 200 , 50)];
    self.parameterTwoTextField.backgroundColor = [UIColor lightGrayColor];
    [self.scrollView addSubview:self.parameterTwoTextField];
    
    self.parameterTwoTextField.delegate = self;

    self.parameterTwoTextField.leftViewMode = UITextFieldViewModeAlways;
    [self.parameterTwoTextField setReturnKeyType:UIReturnKeyDone];
    self.parameterTwoTextField.textAlignment = NSTextAlignmentCenter;

    [self.parameterTwoTextField addTarget:self
                                   action:@selector(textFieldExit:)
                         forControlEvents:UIControlEventEditingDidEndOnExit];
    
    //! Parameter 3 Text Field
    self.parameterThreeTextField = [[UITextField alloc]initWithFrame:CGRectMake(self.view.frame.size.width / 3, kStartingHeightAdvancedSettings + 320, 200 , 50)];
    self.parameterThreeTextField.backgroundColor = [UIColor lightGrayColor];
    [self.scrollView addSubview:self.parameterThreeTextField];
    
    self.parameterThreeTextField.delegate = self;

    self.parameterThreeTextField.leftViewMode = UITextFieldViewModeAlways;
    [self.parameterThreeTextField setReturnKeyType:UIReturnKeyDone];
    self.parameterThreeTextField.textAlignment = NSTextAlignmentCenter;

    [self.parameterThreeTextField addTarget:self
                                   action:@selector(textFieldExit:)
                         forControlEvents:UIControlEventEditingDidEndOnExit];
    
    // If game model has parameters, update the text field
    [self updateAdvancedSettingsParameters];
    
    //! Apply button to send advanced settings changes
    [self addApplyButton];
    
    //! Reset button to reset advanced settings changes
    [self addResetButton];
    
    self.offset = CGPointMake(0, self.view.frame.size.height);
}

- (void)updateAdvancedSettingsParameters{
    
    self.parameterOneTextField.text = self.model.advancedParam1;
    self.parameterTwoTextField.text = self.model.advancedParam2;
    self.parameterThreeTextField.text =  self.model.advancedParam3;
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView{
    
    if (self.scrollView.contentOffset.y >= self.view.frame.size.height){
        // Show Advance Settings title if we scrolled down to advanced settings area
        self.title = @"Advanced Settings";
    } else {
        self.title = @"Settings";

    }
}


- (void)modeSelected:(id)sender {
    UISegmentedControl *segmentedControl = (UISegmentedControl *)sender;
    
    if (segmentedControl.tag == kModeOneTag){
        //self.modeOneSelected = [self.modeOne titleForSegmentAtIndex:self.modeOne.selectedSegmentIndex];
        self.modeOneSelected = [NSString stringWithFormat:@"%i", self.modeOne.selectedSegmentIndex];
        NSLog(@"Mode: %@",self.modeOneSelected);

    } else if (segmentedControl.tag == kModeTwoTag) {
        //        self.modeTwoSelected = [self.modeTwo titleForSegmentAtIndex:self.modeTwo.selectedSegmentIndex];
        self.modeTwoSelected = [NSString stringWithFormat:@"%i", self.modeTwo.selectedSegmentIndex];
        NSLog(@"Mode: %@",self.modeTwoSelected);

    } else if (segmentedControl.tag == kModeThreeTag){
        //        self.modeThreeSelected = [self.modeThree titleForSegmentAtIndex:self.modeThree.selectedSegmentIndex];
        self.modeThreeSelected = [NSString stringWithFormat:@"%i", self.modeThree.selectedSegmentIndex];
        NSLog(@"Mode: %@",self.modeThreeSelected);

    }
}

- (void)textFieldDidBeginEditing:(UITextField *)textField {
    
    CGPoint scrollPoint = CGPointMake(0, textField.frame.origin.y - kKeyboardHeight);
    self.scrollView.contentSize = CGSizeMake(self.scrollView.contentSize.width, self.scrollView.contentSize.height + kKeyboardHeight);
    [self.scrollView setContentOffset:scrollPoint animated:YES];
    
}

- (void)textFieldDidEndEditing:(UITextField *)textField {
    
    self.scrollView.contentSize = CGSizeMake(self.scrollView.contentSize.width, self.scrollView.contentSize.height - kKeyboardHeight);
    [self.scrollView setContentOffset:self.offset animated:YES];

}

- (void)textFieldExit:(id)sender
{
    NSLog(@"textFieldExit");
}

- (void)resetSettings: (id)sender {
    [self.modeOne setSelectedSegmentIndex:UISegmentedControlNoSegment];
    [self.modeTwo setSelectedSegmentIndex:UISegmentedControlNoSegment];
    [self.modeThree setSelectedSegmentIndex:UISegmentedControlNoSegment];
    
    self.modeOneSelected = kNoModeSelected;
    self.modeTwoSelected = kNoModeSelected;
    self.modeThreeSelected = kNoModeSelected;

    self.parameterOneTextField.text = kNoParams;
    self.parameterTwoTextField.text = kNoParams;
    self.parameterThreeTextField.text = kNoParams;
    
}


#pragma mark XMPP Messages

//! Send game start message
- (void)sendNextMessage {
    
    const char recipient[] = "game-engine";
    NSString *xmppMessage = [NSString stringWithFormat: @"next:"];
    const char *message = [xmppMessage UTF8String];
    
    if (0 == dispatchEvent(recipient, message)) {
        //NSLog(@"sent successfully");
    }
}

//! Send game start message
- (void)sendStartMessage {
    
    const char recipient[] = "game-launcher";
    NSString *xmppMessage = [NSString stringWithFormat: @"command:game=%@,option=start,round=%i,difficulty=%i",
                             self.gameID, self.selectedRound, self.selectedDifficulty];
    const char *message = [xmppMessage UTF8String];
    
    if (0 == dispatchEvent(recipient, message)) {
        //NSLog(@"sent successfully");
        [self showGameRunningState];
        self.model.isGameStarted = YES;

        [self hideStartButton];
        [self showStopButton];
        [self showPauseButton];

    }
}

//! Send game stop message
- (void)sendStopMessage {
    const char recipient[] = "game-launcher";
    NSString *xmppMessage = [NSString stringWithFormat: @"command:game=%@,option=stop", self.gameID];
    const char *message = [xmppMessage UTF8String];
    
    if (0 == dispatchEvent(recipient, message)) {
        
        self.model.isGameStarted = NO;
        self.model.gamePaused = NO;
        
        [self hidePauseButton];
        [self hideStopButton];
        [self showStartButton];
        [self showGameReadyState];
    }
}

//! Send game pause message
- (void)sendPauseMessage {
    const char recipient[] = "game-launcher";
    NSString *xmppMessage = [NSString stringWithFormat: @"command:game=%@,option=pause", self.gameID];
    const char *message = [xmppMessage UTF8String];
    if (0 == dispatchEvent(recipient, message)) {
        if (!self.model.gamePaused){
            self.model.gamePaused = YES;
            [self.pauseButton setTitle:@"Unpause Game" forState: UIControlStateNormal];
        } else {
            self.model.gamePaused = NO;
            [self.pauseButton setTitle:@"Pause Game" forState: UIControlStateNormal];
        }
    }
}

//! Send game quit message
- (void)sendQuitMessage {
    const char recipient[] = "game-launcher";
    NSString *xmppMessage = [NSString stringWithFormat: @"command:game=%@,option=quit", self.gameID];
    const char *message = [xmppMessage UTF8String];
    
    if (0 == dispatchEvent(recipient, message)) {
        //NSLog(@"quit sent successfully");
        self.model.isGameStarted = NO;
        self.model.gamePaused = NO;
        self.model.gameIDToBeLoaded = nil;
    }

    [self.navigationController popToRootViewControllerAnimated:YES];
}

//! Send game apply message
- (void)sendApplyMessage {
    const char recipient[] = "game-engine";
    NSString *xmppMessage = [NSString stringWithFormat: @"applyParams:mode1=%@,mode2=%@,mode3=%@,param1=%@,param2=%@,param3=%@",
                             self.modeOneSelected, self.modeTwoSelected, self.modeThreeSelected,
                             self.parameterOneTextField.text, self.parameterTwoTextField.text, self.parameterThreeTextField.text];
    const char *message = [xmppMessage UTF8String];
    
    if (0 == dispatchEvent(recipient, message)) {
        NSLog(@"apply sent successfully: %@", xmppMessage);
    }
}

#pragma mark Game State

//! Show game state after a game is started
- (void)showGameRunningState {
    self.gameStarted = YES;
    
    // After game starts, lock options in place and change text color to gray
    for (int i = 0; i < [self.tableView.visibleCells count]; i++){
        UITableViewCell *cell = self.tableView.visibleCells[i];
        cell.textLabel.textColor = [UIColor lightGrayColor];
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    
    self.selectedRoundLabel.textColor = [UIColor lightGrayColor];
    self.selectedDifficultyLabel.textColor = [UIColor lightGrayColor];
    
    [self showPauseButton];
    [self showStopButton];
}

//! Show game state after a game is started
- (void)showGameReadyState {
    self.gameStarted = NO;
    [self showStartButton];
    
    // After game starts, lock options in place and change text color to gray
    for (int i = 0; i < [self.tableView.visibleCells count]; i++){
        UITableViewCell *cell = self.tableView.visibleCells[i];
        cell.textLabel.textColor = [UIColor blackColor];
        cell.selectionStyle = UITableViewCellSelectionStyleDefault;
    }
    
    self.selectedRoundLabel.textColor = [UIColor blackColor];
    self.selectedDifficultyLabel.textColor = [UIColor blackColor];
}

#pragma mark Alert

//! Show Quit alert
- (void)showQuitAlert {
     
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Quit"
                                                    message:@"Do you want to quit the Game?"
                                                   delegate:self
                                          cancelButtonTitle:@"OK"
                                          otherButtonTitles:@"Cancel", nil];
    [alert show];
}

//! Handle Quit alert response
- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
    if (buttonIndex == 0) { // User clicked OK
        self.model.isGameStarted = NO;
        self.model.gameIDToBeLoaded = nil;
        [self sendQuitMessage];
        [self.navigationController popToRootViewControllerAnimated:YES];
    } else if (buttonIndex == 1){ // User clicked Cancel
        // NSLog(@"Cancel");
    }
}

#pragma mark Action Sheet

//! Show the Select Round action sheet
- (void)showRoundSelect {
    NSString *actionSheetTitle = @"Select Round"; //Action Sheet Title
    NSString *other1 = kRound1Text;
    NSString *other2 = kRound2Text;
    NSString *other3 = kRound3Text;
    NSString *other4 = kRound4Text;
    NSString *other5 = kRound5Text;
    NSString *cancelTitle = @"Cancel";
    UIActionSheet *actionSheet = [[UIActionSheet alloc]
                                  initWithTitle:actionSheetTitle
                                  delegate:self
                                  cancelButtonTitle:cancelTitle
                                  destructiveButtonTitle:nil
                                  otherButtonTitles:other1, other2, other3, other4, other5, nil];
    actionSheet.tag = kRoundsTag;
    [actionSheet showInView:self.view];
}

//! Show the Select Difficulty action sheet
- (void)showDifficultySelect {
    NSString *actionSheetTitle = @"Select Difficulty"; //Action Sheet Title
    NSString *other1 = kEasiestText;
    NSString *other2 = kEasyText;
    NSString *other3 = kMediumText;
    NSString *other4 = kHardText;
    NSString *other5 = kHardestText;
    NSString *cancelTitle = @"Cancel";
    UIActionSheet *actionSheet = [[UIActionSheet alloc]
                                  initWithTitle:actionSheetTitle
                                  delegate:self
                                  cancelButtonTitle:cancelTitle
                                  destructiveButtonTitle:nil
                                  otherButtonTitles:other1, other2, other3, other4, other5, nil];
    actionSheet.tag = kDifficultyTag;
    [actionSheet showInView:self.view];
}

//! Handle the action sheet response to update the Round and Difficulty labels
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (actionSheet.tag == kRoundsTag){
        switch (buttonIndex)
        {
            case 0:
                self.selectedRoundLabel.text = kRound1Text;
                self.selectedRound = kRound1;
                break;
            case 1:
                self.selectedRoundLabel.text = kRound2Text;
                self.selectedRound = kRound2;
                break;
            case 2:
                self.selectedRoundLabel.text = kRound3Text;
                self.selectedRound = kRound3;
                break;
            case 3:
                self.selectedRoundLabel.text = kRound4Text;
                self.selectedRound = kRound4;
                break;
            case 4:
                self.selectedRoundLabel.text = kRound5Text;
                self.selectedRound = kRound5;
                break;
        }
    } else if (actionSheet.tag == kDifficultyTag){
        switch (buttonIndex)
        {
            case 0:
                self.selectedDifficultyLabel.text = kEasiestText;
                self.selectedDifficulty = kEasiest;
                break;
            case 1:
                self.selectedDifficultyLabel.text = kEasyText;
                self.selectedDifficulty = kEasy;
                break;
            case 2:
                self.selectedDifficultyLabel.text = kMediumText;
                self.selectedDifficulty = kMedium;
                break;
            case 3:
                self.selectedDifficultyLabel.text = kHardText;
                self.selectedDifficulty = kHard;
                break;
            case 4:
                self.selectedDifficultyLabel.text = kHardestText;
                self.selectedDifficulty = kHardest;
                break;
        }
    }
}

#pragma mark Table View Protocol

//! Returns the number of rows for the settings
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.settingsList count];
}

//! Creates the layout of each cell
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *simpleTableIdentifier = @"SettingsItem";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
    }
    
    cell.textLabel.text = [self.settingsList objectAtIndex:indexPath.row];
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    
    return cell;
}

//! Shows an action sheet when a settings table cell is selected
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath{
    
    if (!self.gameStarted){
        if (indexPath.row == 0){
            // Show the action sheet for Round Select
            [self showRoundSelect];
        } else {
            // Show the action sheet for Difficulty Select
            [self showDifficultySelect];
        }
    }
}

- (void)dealloc {
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
}

@end
