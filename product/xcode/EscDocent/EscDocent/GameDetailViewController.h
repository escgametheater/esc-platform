//
//  GameDetailViewController.h
//  ESCDocent
//
//  Created by Andy So on 3/18/14.
//  Copyright (c) 2014 ESI. All rights reserved.
//

#import <UIKit/UIKit.h>

/**
 * The GameDetailViewController shows the setings details of a game. It includes the game title, round selection
 * and difficulty selection. It also includes the ability to start/pause/quit games.
 */

@interface GameDetailViewController : UIViewController <UITableViewDelegate, UITableViewDataSource, UIActionSheetDelegate, UITextFieldDelegate>

//! Initialization method that includes the game title as a parameter
- (id)initWithGameTitle: (NSString *)title andGameID: (NSString *)gameID;


@end
