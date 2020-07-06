//
//  main.cpp
//  EscPlatform-Import-Test
//
//  Created by Caleb Johnston on 3/4/14.
//  Copyright (c) 2014 com.esc. All rights reserved.
//

#include <iostream>

#include "PluginInterface.h"
#include "Logger.h"

int main(int argc, const char * argv[])
{
	esc::io::enable(false);
	
	std::string username("account-test-user");
	std::string password("account-test-password");
	std::string hostname("cgs-MacBook-Pro-4.local");
	startInterface(username.c_str(), password.c_str(), hostname.c_str());
	
	//startClientInterface(hostname.c_str());
//	startDocentInterface(hostname.c_str());
//	startLauncherInterface();
//	startServerInterface();
	
	//int count = 0;
	bool connected = true;
	while (connected) {
		if (hasMoreEvents()) {
			char user[96];
			char msg[256];
			if (getNextEvent(user, 96, msg, 256) == 0) {
				std::cout << "next event from " << user << " : " << msg << std::endl;
			}
		}
		
		if (hasMorePresenceEvents()) {
			char user[96];
			int presence;
			if (getNextPresenceEvent(user, 96, presence) == 0) {
				std::cout << "presence from " << user << " : " << presence << std::endl;
			}
		}
		
		if (hasMoreStatusChanges()) {
			int status;
			if (getNextStatusChange(status) == 0) {
				std::cout << "status change: " << status << std::endl;
			}
		}
		
//		sleep(1);
//		count++;
//		if (40 == count) {
//			std::cout << " ** stopping interface **" << std::endl;
//			stopInterface();
//		}
//		
//		if (count > 20) {
//			connected = isConnected();
//		}
	}
	
	stopInterface();
}

