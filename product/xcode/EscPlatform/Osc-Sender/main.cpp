//
//  main.cpp
//  Osc-Sender
//
//  Created by Caleb Johnston on 2/13/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "OscSender.h"

int main(int argc, const char * argv[])
{
	// collect command line arguments
	std::vector<std::string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( std::string( argv[arg] ) );
	}
	
	bool commandlineArgsValid = commandLineArgs.size() >= 3;
	if (!commandlineArgsValid) {
		std::cout << "Usage: " << commandLineArgs.at(0) << " [host-ip] [port]" << std::endl;
		return -1;
	}
	
	// setup OSC sender
	osc::Sender sender;
	std::string host = commandLineArgs.at(1);
	int port = atoi(commandLineArgs.at(2).c_str());
	sender.setup(host, port);
	
	while (true) {
		osc::Message msg;
		msg.setAddress("rotation-x/rotation-y/rotation-z/joystick/dpad-x/dpad-y");
		msg.addIntArg(5);
		msg.addFloatArg(0.21f);
		msg.addFloatArg(0.0019f);
		msg.addFloatArg(0.5f);
		msg.addFloatArg(21.0f);
		msg.addFloatArg(0.0f);
		msg.addFloatArg(-13.5f);
		sender.sendMessage(msg);
		
		std::cout << "sent message : " << msg.getAddress() << std::endl;
		for (int i=0; i < msg.getNumArgs(); i++) {
			switch (msg.getArgType(i)) {
				case osc::TYPE_INT32:
					std::cout << "  int: " << msg.getArgAsInt32(i) << std::endl;
					break;
				case osc::TYPE_FLOAT:
					std::cout << "  float: " << msg.getArgAsFloat(i) << std::endl;
					break;
				case osc::TYPE_STRING:
					std::cout << "  str: " << msg.getArgAsString(i) << std::endl;
					break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds( 750 ));
	}
	
	return 0;
}

