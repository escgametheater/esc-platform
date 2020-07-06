//
//  main.cpp
//  XmppEchoClient
//
//  Created by Caleb Johnston on 2/13/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#if defined( _LIBCPP_VERSION )
#include <thread>
#endif

#include "XmppClient.h"
#include "XmppEchoListener.h"

using namespace xmpp;
using namespace std;

std::thread listener_thread;

int main(int argc, const char * argv[])
{
	// collect command line arguments
	vector<string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( string( argv[arg] ) );
	}
	
	// verify command line inputs
	bool commandlineArgsValid = commandLineArgs.size() >= 3;
	if (!commandlineArgsValid) {
		std::cout << "Usage: " << commandLineArgs.at(0) << " [username] [password]" << std::endl;
		std::cout << " The username must be fully qualified. The echo client must be connected to the same server." << std::endl;
		return -1;
	}
	
	// parse command line inputs
	string raw_username = commandLineArgs.at(1);
	string username = raw_username.substr(0, raw_username.find("@"));
	string password = commandLineArgs.at(2);
	string server = raw_username.substr(raw_username.find("@") + 1);
	
	// create the echo client
	std::cout << "Starting connection as " << username << "@" << server << " on port 5222..." << std::endl;
	EchoListener listener;
	listener.setup(username, password, server, 5222, false);
	listener.openConnection();
	
    return 0;
}

