//
//  main.cpp
//  OscEchoClient
//
//  Created by Caleb Johnston on 2/14/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "OscListener.h"
#include "OscSender.h"

int main(int argc, const char * argv[])
{
	// collect command line arguments
	std::vector<std::string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( std::string( argv[arg] ) );
	}
	
	bool commandlineArgsValid = commandLineArgs.size() >= 2;
	if (!commandlineArgsValid) {
		std::cout << "Usage: " << commandLineArgs.at(0) << " [listen-port]" << std::endl;
		return -1;
	}
	
	// parse input
	int port = atoi(commandLineArgs.at(1).c_str());
	
	// setup OSC listener
	osc::Listener listener;
	listener.setup(port);
	std::cout << "Listening on port: " << port << std::endl;
	
	// setup OSC sender
	osc::Sender sender;
	while (true) {
		while (listener.hasWaitingMessages()) {
			osc::Message msg;
			listener.getNextMessage(&msg);
			sender.setup(msg.getRemoteIp(), msg.getRemotePort());
			sender.sendMessage(msg);
			std::cout << "  sent message to " << msg.getRemoteIp() << ":" << msg.getRemotePort() << msg.getAddress() << std::endl;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds( 2 ));
	}
	
	return 0;
}

