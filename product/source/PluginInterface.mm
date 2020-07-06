//
//  PluginInterface.cpp
//  ESC-Unity-Plugin
//
//  Created by Caleb Johnston on 1/27/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#if ! defined( TARGET_OSX )
#import <UIKit/UIKit.h>
#import "PdKeychainBindings.h"
#else
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#import <IOKit/IOKitLib.h>
#endif
#endif

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <deque>
#include <functional>
#include <map>
#include <algorithm>
#include <pthread.h>

#include "XmppClient.h"
#include "AccountRegistration.h"
#include "PluginInterface.h"
#include "Config.h"

#import "Reachability.h"

static xmpp::Client* clientPtr = 0;

#pragma mark utility functions

#if defined( TARGET_OSX )
NSRect originalWindowSize;

std::string getHostName()
{
    std::string output;
    
    NSString* name = [[NSHost currentHost] name];
    if (name) {
        output = std::string([name UTF8String]);
        std::transform(output.begin(), output.end(), output.begin(), ::tolower);
    }
    
    return output;
}

std::string uniqueDeviceIdentifier()
{
    NSString *serial = nil;
    io_service_t platform = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOPlatformExpertDevice"));
    if (platform) {
        CFTypeRef serialNumberAsCFString =
        IORegistryEntryCreateCFProperty(platform, CFSTR(kIOPlatformSerialNumberKey), kCFAllocatorDefault, 0);
        if (serialNumberAsCFString) {
            serial = CFBridgingRelease(serialNumberAsCFString);
        }
        
        IOObjectRelease(platform);
    }
    
    std::string output = [serial UTF8String];
    std::transform(output.begin(), output.end(), output.begin(), ::tolower);
    return output;
}

NSRect getDisplayCanvas()
{
    uint32_t maximum = 12;
    CGDirectDisplayID* displays = (CGDirectDisplayID*) calloc(maximum, sizeof(uint32_t));
    uint32_t total;
    CGError err = CGGetOnlineDisplayList(maximum, displays, &total);
    
    // iterate through all "online" displays and capture their global screen-space boundary
    CGRect canvasBounds;
    if (kCGErrorSuccess == err) {
        for (size_t index = 0; index < total; index++) {
            CGRect displayBounds = CGDisplayBounds(displays[index]);
            if (0 == index) {
                canvasBounds = displayBounds;
            }
            else {
                canvasBounds = CGRectUnion(canvasBounds, displayBounds);
            }
        }
    }
    
    // return output NSRect...setFullscreen
    return NSRectFromCGRect(canvasBounds);
}

int launchGameForUrl(const char* URL)
{
    CFStringRef stringUrl = CFStringCreateWithCString(kCFAllocatorDefault, URL, kCFStringEncodingASCII);
    CFURLRef inURL = CFURLCreateWithString(kCFAllocatorDefault, stringUrl, nil);
    LSRolesMask inRoleMask = kLSRolesAll;
    FSRef outAppRef;
    CFURLRef outAppURL;
    OSStatus ret = 0;
    if ((ret = LSGetApplicationForURL(inURL, inRoleMask, &outAppRef, &outAppURL)) == 0) {
        ret = LSOpenCFURLRef(outAppURL, nil);
        if (0 != ret) {
            //			std::stringstream err_ss;
            //			err_ss << "No application represented by url: ";
            //			err_ss << std::string(URL) + ", error: ";
            //			err_ss << ret;
            //			writeLog(err_ss.str().c_str());
        }
        else {
            CFRelease(outAppURL);
        }
    } // silently fail...
    
    CFRelease(inURL);
    CFRelease(stringUrl);
    
    return 0;
}

int accessWindow(char* win, const unsigned win_length)
{
    if (win_length == 0 || win == nullptr) {
        return -1;
    }
    
    // Get Window Frame
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* window = [app mainWindow];
    NSView* view = [window contentView];
    NSString *windowString = NSStringFromRect(view.frame);
    
    // Copy string
    std::string output = [windowString UTF8String];
    strncpy(win, output.c_str(), win_length);
    
    return 0;
}

int setWindowHidden ()
{
    [[NSApplication sharedApplication] setPresentationOptions: NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock];
    
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* window = [app mainWindow];
    window.alphaValue = 0.0;
    
    [window orderBack:nil];
    [window setLevel:NSStatusWindowLevel];
    
    return 0;
}

int setFullscreen()
{
    // Access the window
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* window = [app mainWindow];
    
    [window makeKeyAndOrderFront:nil];
    [window setLevel:NSScreenSaverWindowLevel];
    
    // Save original size
    NSView* view = [window contentView];
    originalWindowSize = view.frame;
    
    // Change the window style to borderless
    window.styleMask = NSBorderlessWindowMask;
    
    // Set the window frame to dual full screen
    NSRect frame = [window frame];
    frame = getDisplayCanvas();
    
    [window setFrame: frame display: YES animate: YES];
    window.alphaValue = 1.0;
    
    return 0;
}

int setFullscreenDeveloperMode()
{
    // Access the window
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* window = [app mainWindow];
    
    [window makeKeyAndOrderFront:nil];
    [window setLevel:NSScreenSaverWindowLevel];
    
    // Save original size
    NSView* view = [window contentView];
    originalWindowSize = view.frame;
    
    // Set the window frame to dual full screen
    NSRect frame = [window frame];
    frame = getDisplayCanvas();
    
    [window setFrame: frame display: YES animate: YES];
    window.alphaValue = 1.0;
    
    return 0;
}

int exitFullscreen()
{
    // Access the window
    NSApplication* app = [NSApplication sharedApplication];
    NSWindow* window = [app mainWindow];
    
    // Set window frame to original frame size
    [window setFrame: originalWindowSize display: YES animate: NO];
    
    return 0;
}

#else
std::string getHostName() { return ""; }

std::string uniqueDeviceIdentifier()
{
    PdKeychainBindings *keychain = [PdKeychainBindings sharedKeychainBindings];
    NSString *uniqueIdentifier = [keychain objectForKey:@"esc-platform-udid"];
    
    if (!uniqueIdentifier || !uniqueIdentifier.length) {
        NSUUID *udid = [[UIDevice currentDevice] identifierForVendor];
        uniqueIdentifier = [udid UUIDString];
        [keychain setObject:uniqueIdentifier forKey:@"esc-platform-udid"];
    }
    
    std::string output = [uniqueIdentifier UTF8String];
    std::transform(output.begin(), output.end(), output.begin(), ::tolower);
    return output;
}

int setFullscreen() { return 0; } /* no-op */
int exitFullscreen() { return 0; } /* no-op */
int setWindowHidden() { return 0; } /* no-op */
int setFullscreenDeveloperMode() { return 0; } /* no-op */
int accessWindow(char* win, const unsigned win_length) { return 0; } /* no-op */

bool dimScreen() {
    if ( [UIScreen mainScreen].brightness != 0.01 ) {
        [[UIScreen mainScreen] setBrightness:0.01];
		return true;
	}
	
	return false;
}

bool brightenScreen() {
    if ( [UIScreen mainScreen].brightness != 1 ) {
        [[UIScreen mainScreen] setBrightness:1.0];
		return true;
	}
	
	return false;
}

bool isDeviceCharging() {
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
    
    if ( [[UIDevice currentDevice] batteryState] == UIDeviceBatteryStateUnplugged ) {
        return false;
    }
    else {
        return true;
    }
}

bool isDeviceWifiConnected() {
    Reachability *reachability = [Reachability reachabilityForLocalWiFi];
    NetworkStatus networkStatus = [reachability currentReachabilityStatus];
    
    switch (networkStatus) {
        case NotReachable: {
            return false;
            break;
        }
        case ReachableViaWWAN: {
            return false;
            break;
        }
        case ReachableViaWiFi: {
            return true;
            break;
        }
    }
}

#endif

int writeLog(const char* message)
{
    if (nullptr == message) return -1;
    
    esc::io::info(std::string(message));
    
    return 0;
}

bool xmppClientCreated()
{
    return nullptr != clientPtr;
}

bool createXmppClient()
{
    if (xmppClientCreated()) return true;
    
    clientPtr = new (std::nothrow) xmpp::Client();
    
    return xmppClientCreated();
}

#pragma mark plugin interface

int startServerInterfaceWithHost( const char* hostname )
{
    if (nullptr == hostname) return -1;
    
    return startInterface(kGameEngineUsername.c_str(), kGameEngineUsername.c_str(), hostname);
}

int startLauncherInterface()
{
    return startInterface(kGameLauncherUsername.c_str(), kGameLauncherUsername.c_str(), getHostName().c_str());
}

int startDocentInterface(const char* hostname)
{
    return startInterface(kDocentUsername.c_str(), kDocentUsername.c_str(), hostname);
}

int startServerInterface()
{
    return startInterface(kGameEngineUsername.c_str(), kGameEngineUsername.c_str(), getHostName().c_str());
}

int startClientInterface(const char* hostname)
{
    if (nullptr == hostname) return -1;
    
    createXmppClient();
    
    std::string udid = uniqueDeviceIdentifier();
    bool success = clientPtr->connect(udid, udid, std::string(hostname));
    
    return success? 0 : 1;
}

int startInterface(const char* username, const char* password, const char* hostname)
{
    if (nullptr == username || nullptr == password || nullptr == hostname) {
        return -1;
    }
    
    createXmppClient();
    
    bool success = clientPtr->connect(std::string(username), std::string(password), std::string(hostname));
    
    return success? 0 : 1;
}

int stopInterface()
{
    if (!xmppClientCreated()) return -1;
    
    if (clientPtr->disconnect()) {
        delete clientPtr;
        clientPtr = 0;
        
        return 0;
    }
    
    return 1;
}

bool isConnected()
{
    return xmppClientCreated() ? clientPtr->connected() : false;
}

int startGame()
{
    if (!xmppClientCreated()) return -1;
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            clientPtr->sendMessage((*iter).second.username(), "gameStart");
        }
    }
    
    return 0;
}

int cancelGame()
{
    if (!xmppClientCreated()) return -1;
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            clientPtr->sendMessage((*iter).second.username(), "gameCancel");
        }
    }
    
    return 0;
}

int endGame()
{
    if (!xmppClientCreated()) return -1;
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            clientPtr->sendMessage((*iter).second.username(), "gameEnd");
        }
    }
    
    return 0;
}

int startRound()
{
    if (!xmppClientCreated()) return -1;
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            clientPtr->sendMessage((*iter).second.username(), "startRound");
        }
    }
    
    return 0;
}

int cancelRound()
{
    if (!xmppClientCreated()) return -1;
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            clientPtr->sendMessage((*iter).second.username(), "cancelRound");
        }
    }
    
    return 0;
}

int endRound()
{
    if (!xmppClientCreated()) return -1;
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            clientPtr->sendMessage((*iter).second.username(), "endRound");
        }
    }
    
    return 0;
}

int pauseGame(const bool state)
{
    if (!xmppClientCreated()) return -1;
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            std::string bool_str = state ? "true" : "false";
            clientPtr->sendMessage((*iter).second.username(), "pause:" + bool_str);
        }
    }
    
    return 0;
}

#pragma mark polling events

bool hasMoreEvents()
{
    return (xmppClientCreated() && !clientPtr->getEventQueue().empty());
}

int getNextEvent(char* username, const unsigned username_length, char* message, const unsigned message_length)
{
    if (nullptr == username || nullptr == message) return -1;
    
    if (!xmppClientCreated() || clientPtr->getEventQueue().empty()) return -1;
    
    xmpp::Event& evt = clientPtr->getEventQueue().front();
    strncpy(username, evt.username.c_str(), username_length);
    strncpy(message, evt.message.c_str(), message_length);
    clientPtr->getEventQueue().pop_front();
    
    return 0;
}

bool hasMorePresenceEvents()
{
    return (xmppClientCreated() && !clientPtr->getPresenceQueue().empty());
}

int getNextPresenceEvent(char* username, const unsigned username_length, int& presence)
{
    if (nullptr == username) return -1;
    
    if (!xmppClientCreated() || clientPtr->getPresenceQueue().empty()) return -1;
    
    xmpp::PresenceEvent& evt = clientPtr->getPresenceQueue().front();
    strncpy(username, evt.username.c_str(), username_length);
    presence = evt.status;
    clientPtr->getPresenceQueue().pop_front();
    
    return 0;
}

bool hasMoreStatusChanges()
{
    return (xmppClientCreated() && !clientPtr->getStateQueue().empty());
}

int getNextStatusChange(int& status)
{
    if (!xmppClientCreated() || clientPtr->getStateQueue().empty()) return -1;
    
    status = clientPtr->getStateQueue().front();
    clientPtr->getStateQueue().pop_front();
    
    return 0;
}

int dispatchEvent(const char* to, const char* body)
{
    if (nullptr == to || nullptr == body) return -1;
    
    if (!xmppClientCreated()) return -1;
    
    std::string recipient(to);
    std::string msg(body);
    
    // NOTE: recipient and message body are validated later...
    
    bool success = clientPtr->sendMessage(recipient, msg);
    
    return success? 0 : 1;
}

void broadcastEvent(const char* body)
{
    if (nullptr == body) return;
    
    std::string msg_body(body);
    
    // NOTE: message body are validated later...
    
    const std::map<std::string, xmpp::Peer>& roster = clientPtr->getRoster();
    for (auto iter = roster.begin(); iter != roster.end(); iter++) {
        if ( (*iter).second.isConnected() ) {
            clientPtr->sendMessage((*iter).second.username(), msg_body);
        }
    }
}

int getLauncherInfoPlist(const char* filename, char* contents, const unsigned contents_length)
{
    if (nullptr == filename || nullptr == contents) return -1;
    
    std::string str;
    std::ifstream input(filename, std::ios::in | std::ios::binary);
    if (input) {
        str = std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        strncpy(contents, str.c_str(), contents_length);
        return 0;
    }
    else {
        return -1;
    }
}

int getUniqueDeviceIdenifier(char* udid, const unsigned udid_length)
{
    if (nullptr == udid) return -1;
    
    std::string unique_id = uniqueDeviceIdentifier();
    
    if (!unique_id.empty()) {
        strncpy(udid, unique_id.c_str(), udid_length);
        return 0;
    }
    
    return -1;
}

int getGameIdentifier(char* contents, const unsigned contents_length)
{
    if (nullptr == contents) return -1;
    
    NSDictionary *infoDictionary = [[NSBundle mainBundle] infoDictionary];
    NSString* stringValue = [infoDictionary valueForKey:@"CFBundleExecutable"];
    std::string str = [stringValue UTF8String];
    strncpy(contents, str.c_str(), contents_length);
    
    return 0;
}

int getGoogleAnalyticsIdentifier(char* contents, const unsigned contents_length)
{
    if (nullptr == contents) return -1;
    
    std::string GA_identifier = "UA-53071948-1";
    strncpy(contents, GA_identifier.c_str(), contents_length);
    return 0;
}