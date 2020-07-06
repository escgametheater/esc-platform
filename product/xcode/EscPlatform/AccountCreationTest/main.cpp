//
//  main.cpp
//  AccountCreationTest
//
//  Created by Caleb Johnston on 3/22/14.
//  Copyright (c) 2014 com.esc. All rights reserved.
//

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "AccountEraser.h"
#include "AccountRegistration.h"

int main(int argc, const char * argv[])
{
	// collect command line arguments
	std::vector<std::string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( std::string( argv[arg] ) );
	}
	
	std::string first_param;
	
	// parse command line inputs starting with username...
	std::string username, password, server;
	if (commandLineArgs.size() >= 2) {
		first_param = commandLineArgs.at(1);
		if ("--help" == first_param) {
			std::cout << "Usage: " << commandLineArgs.at(0) << " [username] [password] [server]" << std::endl;
			return 0;
		}
		else {
			username = first_param;
		}
	}
	else {
		username = "account-test-user1";
	}
	
	// parse input password string
	if (commandLineArgs.size() >= 3) {
		password = commandLineArgs.at(2);
	}
	else {
		password = "account-test-user1";
	}
	
	// parse input server string
	if (commandLineArgs.size() >= 4) {
		server = commandLineArgs.at(3);
	}
	else {
		server = "cgs-macbook-pro-4.local";
	}
	
	// create timing facilities
	std::chrono::time_point<std::chrono::system_clock> start, end;
	
	// test creating the account...
	xmpp::AccountRegistration registration;
	start = std::chrono::system_clock::now();
	registration.init(username, password, server);
	while (xmpp::INCOMPLETE == registration.status()) {
		std::this_thread::sleep_for(std::chrono::milliseconds( 100 ));
		continue;
	}
	
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	if (xmpp::SUCCESS == registration.status()) {
		std::cout << "Account creation successful!" << std::endl;
		std::cout << "  registration took " << elapsed_seconds.count() << " seconds." << std::endl;
	}
	else {
		std::cout << "Account creation FAILED!" << std::endl;
	}
	 
	// test removing the account...
	xmpp::AccountEraser eraser;
	start = std::chrono::system_clock::now();
	eraser.init(username, password, server);
	while (xmpp::INCOMPLETE == eraser.status()) {
		std::this_thread::sleep_for(std::chrono::milliseconds( 100 ));
		continue;
	}
	
	end = std::chrono::system_clock::now();
	elapsed_seconds = end - start;
	if (xmpp::SUCCESS == eraser.status()) {
		std::cout << "Account deletion successful!" << std::endl;
		std::cout << "  deactivation took " << elapsed_seconds.count() << " seconds." << std::endl;
	}
	else {
		std::cout << "Account deletion FAILED!" << std::endl;
	}
	
    return 0;
}

