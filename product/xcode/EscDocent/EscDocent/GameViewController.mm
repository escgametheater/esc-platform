//
//  GameViewController.m
//  ESCDocent
//
//  Created by Andy So on 3/18/14.
//  Copyright (c) 2014 ESI. All rights reserved.
//

#import "GameViewController.h"
#import "GameDetailViewController.h"
#import "PluginInterface.h"
#import "GameModel.h"

NSString *const kConnected = @"Connected";
NSString *const kDisconnected = @"Not Connected";

int const kReconnectTag = 1;
int const kLoadTag = 2;

@interface GameViewController ()

@property (nonatomic, strong) NSMutableArray *gameTitle;
@property (nonatomic, strong) NSMutableArray *gameList;
@property (nonatomic, strong) NSString *gameListUpdate;


@property (nonatomic, strong) UILabel *currentConnection;
@property (nonatomic, strong) UITableView *tableView;

@property (nonatomic, strong) NSTimer *timer;

@property (nonatomic, weak) GameModel *model;

@property (nonatomic) BOOL isGameListLoaded;

@property (nonatomic, strong) NSString *titleToLoad;
@property (nonatomic, strong) NSString *gameidToLoad;

@property (nonatomic, strong) UIButton *reconnectButton;

@end

@implementation GameViewController

#pragma mark View

//! Creates the Docent view when it is first loaded
- (void)viewDidLoad
{
    [super viewDidLoad];
    self.model = [GameModel sharedInstance];
    [self addConnection];
    [self addReconnectButton];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(loadGameList)
                                                 name:kGameListChangedNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(loadGameDetailViewFromNotification)
                                                 name:kGamePreviouslyLoadedNotification
                                               object:nil];
    
    // Load the game list if the Game Model game list array is not nil
    if ([self.model.gameList count] > 0){
        self.gameList = [[NSMutableArray alloc]initWithArray:self.model.gameList];
        self.gameTitle = [[NSMutableArray alloc]initWithArray:self.model.gameTitle];
        [self.tableView reloadData];
        [self loadGameList];
    }
    
}

//! Creates a connection update timer when the view will appear
- (void)viewWillAppear:(BOOL)animated {
    
    // Create a timer to check the connection status once every second
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(checkConnection) userInfo:nil repeats:YES];
    
    // Compare game list
    if ((self.gameList != self.model.gameList) &&
        (self.gameTitle != self.model.gameTitle)){
        self.gameList = self.model.gameList;
        self.gameTitle = self.model.gameTitle;
        [self.tableView reloadData];
    }
}

//! Removes the check connection timer when the view will disappear
- (void)viewWillDisappear:(BOOL)animated {
    // Remove the timer
    [self.timer invalidate];
    self.timer = nil;
}

#pragma mark Connection Status

//! Create a Connection label that will show whether or not the Docent is connected to the server
- (void)addConnection {
    CGRect titleRect = CGRectMake(0, 44, self.view.frame.size.width, 40);
    self.currentConnection = [[UILabel alloc] initWithFrame:titleRect];
    self.currentConnection.textColor = [UIColor whiteColor];
    self.currentConnection.opaque = YES;
    self.currentConnection.font = [UIFont boldSystemFontOfSize:14];
    self.currentConnection.textAlignment = NSTextAlignmentCenter;
    [self.view addSubview:self.currentConnection];
}

//! Check connection and update the connection label
- (void)checkConnection {
    
    [self.model checkConnection];
    
    if (self.model.gameListDateLastUpdated)
        self.gameListUpdate = [NSString stringWithFormat: @"(Updated: %@)", self.model.gameListDateLastUpdated];
    else
        self.gameListUpdate = @"";
    
    if (isConnected()){
        // Change Connection label to green if its connected
        self.currentConnection.backgroundColor = [UIColor greenColor];
        self.currentConnection.text = [NSString stringWithFormat: @"%@ %@",kConnected, self.gameListUpdate] ;
    } else {
        // Change Connection label to red if its not connected
        self.currentConnection.backgroundColor = [UIColor redColor];
        self.currentConnection.text = [NSString stringWithFormat: @"%@ %@",kDisconnected, self.gameListUpdate] ;
    }
    
    if ( isDeviceCharging() ) {
        dimScreen();
    }
    else {
        if ( isDeviceWifiConnected() ) {
            brightenScreen();
        }
        else {
            dimScreen();
        }
    }

}

//! Load the game list in a table view
- (void)loadGameList {
    // Create the table view with the list of games
    if (!self.isGameListLoaded){
        self.tableView = [[UITableView alloc] initWithFrame:CGRectMake(0, 84, self.view.frame.size.width, self.view.frame.size.height - 154)];
        self.tableView.delegate = self;
        self.tableView.dataSource = self;
        [self.tableView reloadData];
        [self.view addSubview: self.tableView];
        self.isGameListLoaded = YES;
    } else {
        // Reload the table view
        self.gameList = self.model.gameList;
        self.gameTitle = self.model.gameTitle;
        [self.tableView reloadData];
    }
}

#pragma mark Table View Protocol

//! Return the number of items in the game list to use for the table view
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.gameList count];
}

//! Creates the layout of each cell
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *simpleTableIdentifier = @"GameItem";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
    }
    
    // Set the text for the table cell
    if ([self.model.gameTitle count] > 0){
        cell.textLabel.text = [self.model.gameTitle objectAtIndex:indexPath.row];
    }
    cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
    
    return cell;
}

//! Pushes the game detail view controller when a row is selected
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath{
    
    // Set the game title and game id to be loaded
    self.titleToLoad = self.gameTitle[indexPath.row];
    self.gameidToLoad = self.gameList[indexPath.row];
    
    if ([self.model.gameIDToBeLoaded isEqualToString:self.gameList[indexPath.row]]){
        // Load detail game view controller for a game that has already been loaded
        [self showGameDetailView];
        
    } else {
        // Show Load alert to confirm that they want to load a game
        [self showLoadAlert];
    }
}

#pragma mark Load Alert

//! Show Load alert
- (void)showLoadAlert {
    
    NSString *loadMessage = [NSString stringWithFormat:@"Do you want to load %@?", self.titleToLoad];
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Load Game"
                                                    message:loadMessage
                                                   delegate:self
                                          cancelButtonTitle:@"OK"
                                          otherButtonTitles:@"Cancel", nil];
    alert.tag = kLoadTag;
    [alert show];
}

//! Send game Load message
- (void)sendLoadMessage {
    const char recipient[] = "game-launcher";
    NSString *xmppMessage = [NSString stringWithFormat: @"command:game=%@,option=load", self.gameidToLoad];
    const char *message = [xmppMessage UTF8String];
    
    if (0 == dispatchEvent(recipient, message)) {
        //NSLog(@"load sent successfully");
    }
}

#pragma mark add Reconnect Button

//! Add the Reconnect Button
- (void)addReconnectButton {
    self.reconnectButton = [[UIButton alloc]initWithFrame:CGRectMake(10, self.view.frame.size.height - 60, 300, 50)];
    self.reconnectButton.backgroundColor = [UIColor lightGrayColor];
    [self.reconnectButton setTitle:@"Re-Connect" forState: UIControlStateNormal];
    [self.reconnectButton setTitleColor:[UIColor grayColor] forState: UIControlStateHighlighted];
    
    [self.view addSubview:self.reconnectButton];
    [self.reconnectButton addTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
}

//! Send a message if the button is pressed
- (void)buttonPressed:(id)sender {
    
    [self showReconnectAlert];
    
}

- (void)showReconnectAlert {
    
    NSString *reconnectMessage = [NSString stringWithFormat:@"Do you want to try to re-connect"];
    UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Re-Connect"
                                                    message:reconnectMessage
                                                   delegate:self
                                          cancelButtonTitle:@"OK"
                                          otherButtonTitles:@"Cancel", nil];
    alert.tag = kReconnectTag;
    [alert show];
}

#pragma mark Alert delegate


//! Handle Load alert response
- (void)alertView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex {
    
    if (alertView.tag == kReconnectTag){
        if (buttonIndex == 0) { // User clicked OK
            stopInterface();
            startDocentInterface("esc-game-server.local");
        }
    } else if (alertView.tag == kLoadTag){
        if (buttonIndex == 0) { // User clicked OK
            self.model.isGameStarted = NO;
            [self sendLoadMessage];
            [self showGameDetailView];
        }
    }
}

// Push the Game Detail View Controller
- (void)showGameDetailView {
    // Set the game running id
    self.model.gameIDToBeLoaded = self.gameidToLoad;
    
    // Push the Detail View Controller for the selected game with the game title
    GameDetailViewController *gameDetail = [[GameDetailViewController alloc]initWithGameTitle:self.titleToLoad andGameID:self.gameidToLoad];
    [[self navigationController]pushViewController:gameDetail animated:YES];
}

- (void)loadGameDetailViewFromNotification {
    
    // Push the Detail View Controller for the selected game with the game title
    GameDetailViewController *gameDetail = [[GameDetailViewController alloc]initWithGameTitle:nil andGameID:self.model.gameIDToBeLoaded];
    [[self navigationController]pushViewController:gameDetail animated:YES];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kGameListChangedNotification object:nil];
}

@end
