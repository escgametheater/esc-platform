//
//  main.cpp
//  MockForUnity
//
//  Created by Caleb Johnston on 2/16/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//
//  The MockForGame provides a set of mock clients that will dispatch game messages
//  over XMPP in addition to a stream of UDP messages formatted using OSC.
//

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "IosClientMock.h"

int main(int argc, const char * argv[])
{
	// collect command line arguments
	std::vector<std::string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( std::string( argv[arg] ) );
	}
	
	bool commandlineArgsValid = commandLineArgs.size() >= 4;
	if (!commandlineArgsValid) {
		std::cout << "Usage: " << commandLineArgs.at(0) << " [server] [echo-client] [osc-port] [number-of-clients]" << std::endl;
		return -1;
	}
	
	// parse command line inputs
	std::string server = commandLineArgs.at(1);
	std::string peer = commandLineArgs.at(2);
	//peer = peer.substr(0, commandLineArgs.at(2).find("@"));
	int server_port = atoi(commandLineArgs.at(3).c_str());
	int client_count;
	if (5 == commandLineArgs.size()) {
		client_count = atoi(commandLineArgs.at(4).c_str());
	}
	else {
		client_count = 30;
	}
	
	// start program...
	IosClientMock mock;
	IosClientMock::Configuration config;
	config.server = server;
	config.oscPort = server_port;
	config.oscFrequency = 30;
	config.xmppFrequency = 1;
	config.oscPayloadSize = 100;
	config.xmppPayloadSize = 128;
	mock.start( config );
	
    return 0;
}

