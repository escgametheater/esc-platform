//
//  main.cpp
//  MockForController
//
//  Created by Caleb Johnston on 2/16/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//
//  The MockForController provides a mock server that will dispatch game messages
//  over XMPP. All messages being sent are required to be handled. All messages
//  being sent are stored in the test_messages array object (defined below).
//

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <thread>

#include "XmppClient.h"

bool is_connected = true;
bool is_registered = false;
bool is_initialized = false;
std::list<std::string> client_roster;
xmpp::Client client;
std::string ios_user;
std::array<std::string, 10> test_messages{ { "gameStart", "roundStart", "roundEnd", "roundStart", "roundEnd", "roundStart", "paused:true", "paused:false", "roundEnd", "gameEnd"} };

// callback handler for the XMPP client connection
void connected()
{
	std::cout << " connected to server." << std::endl;
	is_connected = true;
}

// callback handler for the XMPP client disconnection
void disconnect()
{
	if (is_connected) {
		std::cout << " disconnected from server." << std::endl;
		is_connected = false;
	}
}

// callback handler for XMPP message recieved (used for bandwidth testing)
void handleMessage(const xmpp::Peer& peer, const std::string& subject, const std::string& body)
{
	if (!is_registered) {
		std::string::size_type index = body.find("registered");
		if (index != std::string::npos) {
			is_registered = true;
			std::this_thread::sleep_for(std::chrono::milliseconds( 1000 ));
			std::string msg("init:key=0,var=message");
			client.sendMessage(peer.username(), msg, "");
			ios_user = peer.username();
		}
	}
	else if (!is_initialized) {
		std::string::size_type index = body.find("init");
		if (index != std::string::npos) {
			is_initialized = true;
		}
	}
}

void handleRoster(const std::list<std::string>& roster)
{
	std::cout << "handleRoster" << std::endl;
	client_roster = std::list<std::string>(roster);
	//	uint32_t index = 0;
	//	for (std::string peer_username : client_roster ) {
	//		if (xmpp::Available == client.getStatusForPeer(peer_username)) {
	//			std::string msg("registered:127.0.0.1," + std::to_string(index) + ",http://www.test.com/test");
	//			client.sendMessage(peer_username, msg, "");
	//			index++;
	//		}
	//	}
}

void handlePresence(const xmpp::Peer& peer, const std::string& resource, xmpp::status_t status, const std::string& message)
{
	std::cout << "handlePresence" << std::endl;
	uint32_t index = 0;
	if (xmpp::Available == status) {
		std::string msg("registered:127.0.0.1," + std::to_string(index) + ",test-game-id");
		client.sendMessage(peer.username(), msg, "");
		index++;
	}
}

int main(int argc, const char * argv[])
{
	// collect command line arguments
	std::vector<std::string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( std::string( argv[arg] ) );
	}
	
	bool commandlineArgsValid = commandLineArgs.size() >= 4;
	if (!commandlineArgsValid) {
		std::cout << "Usage: " << commandLineArgs.at(0) << " [username] [password] [osc-port]" << std::endl;
		return -1;
	}
	
	// parse command line inputs
	std::string raw_username = commandLineArgs.at(1);
	std::string username = raw_username.substr(0, raw_username.find("@"));
	std::string password = commandLineArgs.at(2);
	std::string server = raw_username.substr(raw_username.find("@") + 1);
	std::string peer = commandLineArgs.at(3);
	//	peer = peer.substr(0, commandLineArgs.at(3).find("@"));
	//	int osc_port = atoi(commandLineArgs.at(4).c_str());
	
	// start program...
	client.setSignalConnect(std::bind(connected));
	client.setSignalDisconnect(std::bind(disconnect));
	client.setSignalRoster(std::bind(handleRoster, std::placeholders::_1));
	client.setSignalMessage(std::bind(handleMessage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	client.setSignalRosterPresence(std::bind(handlePresence, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	client.connect(username, password, server);
	
	size_t index = 0;
	while (is_connected) {
		if (is_initialized && is_registered) {
			std::string next_msg(test_messages[index]);
			client.sendMessage(ios_user, next_msg, "");
			
			std::this_thread::sleep_for(std::chrono::milliseconds( 2500 ));
			
			index++;
			if (index >= test_messages.size()) index = 0;
		}
		
		continue;	// block...
	}
	
    return 0;
}

