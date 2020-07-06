//
//  GameViewController.h
//  ESCDocent
//
//  Created by Andy So on 3/18/14.
//  Copyright (c) 2014 ESI. All rights reserved.
//

#import <UIKit/UIKit.h>

/**
 * The GameViewController is the root view of the Esc Docent. Its contains a table view that
 * consists of the list of games available to play. If Docent is connected to the server, it will 
 * show the list of games available, otherwise it will not show any games.
 */

@interface GameViewController : UIViewController <UITableViewDelegate, UITableViewDataSource>


@end
