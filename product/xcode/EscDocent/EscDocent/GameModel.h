//
//  GameModel.h
//  EscDocent
//
//  Created by Andy So on 3/19/14.
//  Copyright (c) 2014 ESI. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 * The GameModel is a singleton that establishes a Docent connection with the server and handles
 * the initialization of the Games list, lets the Proctor know games will be available to play.
 */

//! The Game list notification will post it receives a new list from the server
extern NSString *const kGameListChangedNotification;

//! The Game End notification will post it receives game end from the server
extern NSString *const kGameEndNotification;

//! The Game Loaded notification will post it receives game loaded from the server
extern NSString *const kGameLoadedNotification;

//! The Game Loaded notification will post it receives game loaded from the server
extern NSString *const kGamePreviouslyLoadedNotification;

//! Advanced Settings Notification
extern NSString *const kGameAdvancedSettingsNotification;

//! Advanced Settings Key
extern NSString *const kGameAdvancedSettingsKey;

@interface GameModel : NSObject

//! Set to YES if the Docent is connected to the server, NO otherwise
@property (nonatomic) BOOL isConnected;

//! Set to YES if a game is currently running, NO otherwise
@property (nonatomic) BOOL isGameStarted;

//! The gameID of the current game that is loaded
@property (nonatomic, strong) NSString *gameIDToBeLoaded;

//! Game is paused
@property (nonatomic) BOOL gamePaused;

//! Game is loaded
@property (nonatomic) BOOL isGameLoaded;

//! An array that holds a list of the game's ID
@property (nonatomic, strong) NSMutableArray *gameList;

//! An array that holds a list of the game's title
@property (nonatomic, strong) NSMutableArray *gameTitle;

// String date value of the updated game list
@property (nonatomic, strong) NSString *gameListDateLastUpdated;

// String date value of the updated game list
@property (nonatomic, strong) NSString *advancedParam1;

// String date value of the updated game list
@property (nonatomic, strong) NSString *advancedParam2;

// String date value of the updated game list
@property (nonatomic, strong) NSString *advancedParam3;

//! Returns the singleton instance of GameModel
+ (id)sharedInstance;

//! Checks the connection to the server
- (void)checkConnection;

@end
