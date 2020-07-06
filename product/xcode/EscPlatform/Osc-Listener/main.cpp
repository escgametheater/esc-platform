//
//  main.cpp
//  Osc-Listener
//
//  Created by Caleb Johnston on 2/12/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#include <iostream>

#include "OscListener.h"

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
	
	// parse port input
	int port = atoi(commandLineArgs.at(1).c_str());
	
	// start listening...
	osc::Listener listener;
	listener.setup(port);
	std::cout << "Listening on port: " << port << std::endl;
	while (true) {
		while (listener.hasWaitingMessages()) {
			osc::Message message;
			listener.getNextMessage(&message);
			std::cout << message.getAddress() << std::endl;
			for (int i = 0; i < message.getNumArgs(); i++) {
				osc::ArgType arg = message.getArgType(i);
				std::string type = message.getArgTypeName(i);
				switch (arg) {
					case osc::TYPE_FLOAT:
						std::cout << "   " << type << " : " << message.getArgAsFloat(i) << std::endl;
						break;
					case osc::TYPE_INT32:
						std::cout << "   " << type << " : " << message.getArgAsInt32(i) << std::endl;
						break;
					case osc::TYPE_STRING:
						std::cout << "   " << type << " : " << message.getArgAsString(i) << std::endl;
						break;
						
					default:
						std::cout << "   unknown argument type";
						break;
				}
			}
		}
	}
	
	return 0;
}

