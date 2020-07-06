//
//  GameModel.m
//  EscDocent
//
//  Created by Andy So on 3/19/14.
//  Copyright (c) 2014 ESI. All rights reserved.
//

#import "GameModel.h"
#import "PluginInterface.h"

NSString *const kGameList = @"games:";
NSString *const kGameListKey = @"SavedGameList";
NSString *const kGameTitleKey = @"SavedGameTitles";
NSString *const kGameListUpdateKey = @"SavedGameListUpdate";
NSString *const kGameAdvancedSettingsKey = @"applyParams";
NSString *const kGamePreviouslyLoadedKey = @"gamePreviouslyLoaded:";

NSString *const kGameListChangedNotifcation = @"kGameListChangedNotifcation";
NSString *const kGameEnd = @"gameEnd";
NSString *const kGameLoaded = @"gameLoaded";
NSString *const kGameAdvancedSettings = @"applyParams";

NSString *const kGameListChangedNotification = @"kGameListChangedNotification";
NSString *const kGameLoadedNotification = @"kGameLoadedNotification";
NSString *const kGamePreviouslyLoadedNotification = @"kGamePreviouslYLoadedNotification";
NSString *const kGameAdvancedSettingsNotification = @"kGameAdvancedSettingsNotification";

NSString *const kGameEndNotification = @"kGameEndNotification";
const char serverAddress[] = "esc-game-server.local";

@interface GameModel ()

@property (nonatomic) BOOL isInitMessageSentToServer;
@property (nonatomic) BOOL isCheckGameRunningMessageSent;

@property (nonatomic, strong) NSString *gameMessage;

@end

@implementation GameModel

static GameModel *sharedInstance = nil;

//! Create a shared instance singleton
+ (GameModel *)sharedInstance {
    static dispatch_once_t once;
    static id sharedInstance;
    dispatch_once(&once, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}

//! Initialization method to establish a docent connection and initialize the gameTitle and gameList array
- (id)init {
    if (self = [super init]) {
        
        // Start Client Interface
        startDocentInterface(serverAddress);
        
        // Load the game titles and game id list from NSUserDefaults
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        NSArray *titleList = [defaults objectForKey:kGameTitleKey];
        NSArray *list = [defaults objectForKey:kGameListKey];
        NSString *date = [defaults objectForKey:kGameListUpdateKey];
        
        self.gameTitle = [[NSMutableArray alloc]initWithArray:titleList];
        self.gameList = [[NSMutableArray alloc]initWithArray:list];
        self.gameListDateLastUpdated = date;
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(appWillResignActive) name:UIApplicationWillResignActiveNotification object:nil];

    }
    return self;
}

//! Check connection and listen for game list event
- (void)checkConnection {
    
    if (isConnected()){
        self.isConnected = YES;
        
        if (!self.isCheckGameRunningMessageSent)
            [self checkIfGameRunning];
        
        if (!self.isInitMessageSentToServer){
            // Send init message once we are connected
            self.isInitMessageSentToServer = YES;
            [self performSelector:@selector(sendInitMessage) withObject:self afterDelay:1.0];
            
        }
        
        // Listen for Events
        while (hasMoreEvents()){
            char username[32] = "";
            char message[2048] = "";
            if (0 == getNextEvent(username, sizeof(username), message, sizeof(message))){
                NSString *eventMessage = [NSString stringWithUTF8String:message];
                NSLog(@"events: %@", eventMessage);
                // If message has the prefix for the Game List, we want to save the values into our array properties
                if ([eventMessage hasPrefix:kGameList]){
                    [self.gameTitle removeAllObjects];
                    [self.gameList removeAllObjects];
                    
                    NSString *newMessage = [eventMessage stringByReplacingOccurrencesOfString:kGameList withString:@""];
                    if (![newMessage isEqualToString:@""]){
                        NSArray *stringComponents = [newMessage componentsSeparatedByString:@","];
                        
                        for (NSString *keyValuePair in stringComponents)
                        {
                            NSArray *pairComponents = [keyValuePair componentsSeparatedByString:@"="];
                            NSString *key = [pairComponents objectAtIndex:0];
                            [self.gameTitle addObject:key];
                            
                            NSString *value = [pairComponents objectAtIndex:1];
                            [self.gameList addObject:value];
                        }
                    }
                    
                    NSDate *currentTime = [NSDate date];
                    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
                    [dateFormatter setDateFormat:@"MMM dd hh:mm"];
                    self.gameListDateLastUpdated = [dateFormatter stringFromDate: currentTime];
                    
                    NSLog(@"games title: %@", self.gameTitle);
                    NSLog(@"games list: %@", self.gameList);
                    NSLog(@"games date: %@", self.gameListDateLastUpdated);
                    
                    // Save the gameTitle and gameList array into NSUserDefaults
                    [[NSUserDefaults standardUserDefaults] setObject:self.gameTitle forKey:kGameTitleKey];
                    [[NSUserDefaults standardUserDefaults] setObject:self.gameList forKey:kGameListKey];
                    [[NSUserDefaults standardUserDefaults] setObject:self.gameListDateLastUpdated forKey:kGameListUpdateKey];
                    
                    
                    // Post a notification that the game list has changed
                    [[NSNotificationCenter defaultCenter] postNotificationName:kGameListChangedNotification object:nil userInfo:nil];
                }
                
                // Send a game End nofication if it receives a game end message from the game-engine
                if ([eventMessage hasPrefix:kGameEnd]){

                    self.isGameStarted = NO;
                    self.isGameLoaded = NO;
                    
                    [self clearAdvancedParameters];
                    
                    // Post a notification that the game list has changed
                    [[NSNotificationCenter defaultCenter] postNotificationName:kGameEndNotification object:nil userInfo:nil];
                    [[NSNotificationCenter defaultCenter] postNotificationName:kGameAdvancedSettingsNotification object:nil userInfo:nil];
                }
                
                // Received Game Loaded Message
                if ([eventMessage hasPrefix:kGameLoaded]){
                    self.isGameLoaded = YES;

                    // Post a notification that the game loaded
                    [[NSNotificationCenter defaultCenter] postNotificationName:kGameLoadedNotification object:nil userInfo:nil];
                }
                
                // Received Game Previously Loaded Message
                if ([eventMessage hasPrefix:kGamePreviouslyLoadedKey]){
                    self.isGameLoaded = YES;
                    NSString *newMessage = [eventMessage stringByReplacingOccurrencesOfString:kGamePreviouslyLoadedKey withString:@""];
                    
                    if (![newMessage isEqualToString:@""]){
                        NSArray *stringComponents = [newMessage componentsSeparatedByString:@","];
                        
                        for (NSString *keyValuePair in stringComponents)
                        {
                            NSArray *pairComponents = [keyValuePair componentsSeparatedByString:@"="];
                            NSString *key = [pairComponents objectAtIndex:0];
                            NSString *value = [pairComponents objectAtIndex:1];
                            
                            if ([key rangeOfString:@"gameId"].location != NSNotFound){
                                self.gameIDToBeLoaded = value;
                            } else if([key rangeOfString:@"gameStarted"].location != NSNotFound){
                                if ([value isEqualToString:@"True"])
                                    self.isGameStarted = YES;
                                else if ([value isEqualToString:@"False"])
                                    self.isGameStarted = NO;
                            }
                        }
                    }
                    // Post a notification that the game has previously loaded
                    [[NSNotificationCenter defaultCenter] postNotificationName:kGamePreviouslyLoadedNotification object:nil userInfo:nil];
                }
                
                // Post Advanced Game Settings notification
                if ([eventMessage hasPrefix:kGameAdvancedSettings]){
                    
                    NSString *newMessage = [eventMessage stringByReplacingOccurrencesOfString:kGameAdvancedSettingsKey withString:@""];
                    if (![newMessage isEqualToString:@""]){
                        NSArray *stringComponents = [newMessage componentsSeparatedByString:@","];
                        
                        for (NSString *keyValuePair in stringComponents)
                        {
                            NSArray *pairComponents = [keyValuePair componentsSeparatedByString:@"="];
                            NSString *key = [pairComponents objectAtIndex:0];
                            NSString *value = [pairComponents objectAtIndex:1];
                            
                            if ([key rangeOfString:@"param1"].location != NSNotFound){
                                self.advancedParam1 = value;
                            } else if([key rangeOfString:@"param2"].location != NSNotFound){
                                self.advancedParam2 = value;
                            } else if([key rangeOfString:@"param3"].location != NSNotFound){
                                self.advancedParam3 = value;
                            }
                        }
                    }
                    
                    // Post a notification with advanced settings
                    [[NSNotificationCenter defaultCenter] postNotificationName:kGameAdvancedSettingsNotification object:nil userInfo:nil];
                }
            }
        }
        
    } else {
        self.isConnected = NO;
        self.isInitMessageSentToServer = NO;
        [self reconnectToServer];
    }
}

// Reconnect to server if no connection is established
- (void)reconnectToServer {
//    NSLog(@"reconnectToServer");
    stopInterface();
    startDocentInterface(serverAddress);
}

// When the app enters background, stop the interface
-(void)appWillResignActive{
//    NSLog(@"appWillResignActive");
    stopInterface();
    self.isInitMessageSentToServer = NO;
}

// Send init message to server once connection is established
- (void)sendInitMessage {
    const char recipient[] = "game-launcher";
    const char message[] = "init:";
    dispatchEvent(recipient, message);
    
}

// Send message to check if game engine is running
- (void)checkIfGameRunning {
    const char recipient[] = "game-engine";
    const char message[] = "checkIfLoaded";
    dispatchEvent(recipient, message);
    self.isCheckGameRunningMessageSent = YES;
    
}

- (void)clearAdvancedParameters {
    
    self.advancedParam1 = @"";
    self.advancedParam2 = @"";
    self.advancedParam3 = @"";

}


- (void)dealloc {
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];

}

@end
