//
//  PluginInterface.h
//  ESC-Unity-Plugin
//
//  Created by Caleb Johnston on 1/27/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#pragma once

extern "C" {
    
    /**
     * Uses OSX Launch Services commands to start an application that has previously been
     * registered with a URL scheme matching the input URL parameter.
     *
     * @param URL the uniform resource locator representing the application to be started
     */
    int launchGameForUrl(const char* URL);
    
    /**
     * Hides application's main NSWindow by assigning its alphaValue to zero.
     *
     * @return -1 if input pointer is NULL, 0 otherwise.
     */
    int setWindowHidden();
    
    /**
     * Applies full screen mode to the calling application. Stores existing
     * application window rectangle for use with the exitFullscreen function.
     * Updates NSWindow alphaValue by setting it to 1.0
     *
     * @return zero always, has no failure codes yet
     *
     * @see exitFullscreen
     */
    int setFullscreen();
    
    /**
     * Applies full screen mode to the calling application without disabling keyboard
     * access. Achieves this by avoiding use of the NSBorderlessWindowMask setting.
     *
     * @see exitFullscreen
     * @return zero always, has no failure codes yet
     */
    int setFullscreenDeveloperMode();
    
    /**
     * Applies original window rectangular coordinates prior to using full screen mode
     *
     * @return zero always, has no failure codes yet
     *
     * @see setFullscreen
     * @see setFullscreenDeveloperMode
     */
    int exitFullscreen();
    
    /**
     * Finds rectangle coordinates
     *
     * @return -1 if either input is zero
     */
    int accessWindow(char* win, const unsigned win_length);
    
    /**
     * Writes output parameter to system log immediately.
     *
     * @return -1 if input pointer is NULL, 0 otherwise.
     */
    int writeLog(const char* message);
    
    /**
     * Opens connection to XMPP server using the given hostname.
     *
     * NOTE: Internally invokes startInterface using the given hostname instead
     * of the defualt. Automatically attempts to create account if the login
     * is unsuccessful due to authentication failure. If account creation is
     * successful, will subsequently login as the newly registered user.
     */
    int startServerInterfaceWithHost(const char* hostname);
    
    /**
     * Opens connection with the XMPP server as the game launcher user.
     *
     * NOTE: Uses local hostname as target XMPP server! Thus, it must be running already!
     */
    int startLauncherInterface();
    
    /**
     * Opens connection with the XMPP server as the docent user.
     *
     * NOTE: Uses default hostname as target XMPP server! Thus, it must be running already!
     */
    int startDocentInterface(const char* hostname);
    
    /**
     * Opens connection to XMPP server using the default account credentials.
     *
     * NOTE: Uses local hostname as target XMPP server! Thus, it must be running already!
     */
    int startServerInterface();
    
    /**
     * Opens connection to XMPP server using the udid as a username and password.
     *
     * NOTE: Automatically attempts to create account if the login is unsuccessful
     * due to authentication failure. If account creation is successful, will
     * subsequently login as the newly registered user.
     */
    int startClientInterface(const char* hostname);
    
    /**
     * Opens connection to XMPP server using the given credentials.
     * It is required for the username to be available on an XMPP
     * server denoted by the hostname parameter.
     */
    int startInterface(const char* username, const char* password, const char* hostname);
    
    /**
     * Disconnects the client (if already connected). Returns -1 if not yet connected
     */
    int stopInterface();
    
    /**
     * Returns TRUE if client is connected, false otherwise.
     */
    bool isConnected();
    
    //! Announces the start of the game to all clients. Returns -1 if client is not connected.
    int startGame();
    //! Cancels the current the game for all clients. Returns -1 if client is not connected.
    int cancelGame();
    //! Cancels the current the game for all clients. Returns -1 if client is not connected.
    int endGame();
    //! Starts the next round in the game for all clients. Returns -1 if client is not connected.
    int startRound();
    //! Cancels the current round for all clients. Returns -1 if client is not connected.
    int cancelRound();
    //! Ends the current round for all clients. Returns -1 if client is not connected.
    int endRound();
    //! Updates paused status of the game with clients. Returns -1 if client is not connected.
    int pauseGame(const bool state);
    
    //! Returns TRUE if more events are waiting to be processed
    bool hasMoreEvents();
    //! Returns the information for the next event. Subsequently destroys that event.
    int getNextEvent(char* username, const unsigned username_length, char* message, const unsigned message_length);
    //! Returns TRUE if more presence updates are waiting to be processed
    bool hasMorePresenceEvents();
    //! Returns the information for the next presence change event. Subsequently destroys that event.
    int getNextPresenceEvent(char* username, const unsigned username_length, int& presence);
    //! Returns TRUE if more status changes are waiting to be processed
    bool hasMoreStatusChanges();
    //! Returns the next unprocessed status update. Subsequently removes that status from the queue.
    int getNextStatusChange(int& status);
    
    //! Sends the input message event to all clients currently connected.
    void broadcastEvent(const char* body);
    //! Sends the input message event the username provided by the "to" parameter.
    int dispatchEvent(const char* to, const char* body);
    
    //! Sends text contents of launcher's Info.plist
    int getLauncherInfoPlist(const char* filename, char* contents, const unsigned contents_length);
    
    /**
     * Returns a unique identifier for the local device using the given inputs
     *
     * @param udid the destination buffer to copy the identifier into
     * @param udid_length the length of the buffer to which the id will be copied
     * @return zero on success, returns -1 if not able to determine a unique ID
     */
    int getUniqueDeviceIdenifier(char* udid, const unsigned udid_length);
    
    /**
     * Returns the string value for the CFBundleExecutable name stored in the applications plist
     *
     * @param contents the destination buffer to copy the identifier into
     * @param contents_length the length of the buffer to which the id will be copied
     * @return zero on success, returns -1 if not able to determine a unique ID
     */
    int getGameIdentifier(char* contents, const unsigned contents_length);
    
    /**
     * Returns the string value for the ESC Google Analytics identifier
     *
     * @param contents the destination buffer to copy the identifier into
     * @param contents_length the length of the buffer to which the id will be copied
     * @return zero on success, or -1 if contents buffer is invalid
     */
    int getGoogleAnalyticsIdentifier(char* contents, const unsigned contents_length);
    
    /**
     * Returns TRUE if the device battery is charging
     */
    bool isDeviceCharging();
    
    /**
     * Returns TRUE if the device is connected via local wifi
     */
    bool isDeviceWifiConnected();
    
    /**
     * Dims the device display screen
     */
    bool dimScreen();
    
    /**
     * Brightens the device display screen
     */
    bool brightenScreen();
}
