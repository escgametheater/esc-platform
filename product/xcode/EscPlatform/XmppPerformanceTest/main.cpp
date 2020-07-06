//
//  main.cpp
//  XmppPerformanceTest
//
//  Created by Caleb Johnston on 2/13/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#include <iostream>
#include <csignal>
#include <memory>
#include <thread>
#include <vector>
#include <chrono>
#include <list>
#include <numeric>
#include <algorithm>

#include "XmppClient.h"

using namespace std;
using namespace xmpp;

bool reported_latencies = false;
bool is_connected = true;
list<float> latencies;
chrono::time_point<chrono::system_clock> timer_start;
chrono::time_point<chrono::system_clock> timer_end;
int returned_messages = 0;

///////////////////////////////////////////////
// MATHS
///////////////////////////////////////////////

// utility function for computing mean of a sequence of numbers
float computeMean( const list<float>& numbers )
{
	float number_count = static_cast<float>(numbers.size());
	float number_sum = std::accumulate(numbers.begin(), numbers.end(), 0.0f);
	if (0 == number_count) number_count = 1.0;
    return number_sum / number_count;
}

// utility function for computing the min of a sequence of numbers
float computeMin( const list<float>& numbers )
{
	auto iter = std::min_element(numbers.begin(), numbers.end(), [=](float a, float b){ return a < b; });
	return *iter;
}

// utility function for computing max of a sequence of numbers
float computeMax( const list<float>& numbers )
{
	auto iter = std::max_element(numbers.begin(), numbers.end(), [=](float a, float b){ return a < b; });
	return *iter;
}


///////////////////////////////////////////////
// EXIT HANDLERS
///////////////////////////////////////////////

void latency_messaging_results()
{
	float min = computeMin(latencies);
	float max = computeMax(latencies);
	float mean = computeMean(latencies);
	std::cout << std::endl << "Latency results:" << std::endl;
	std::cout << "    round trip minimum latency: " << min << " ms" << std::endl;
	std::cout << "    round trip maximum latency: " << max << " ms" << std::endl;
	std::cout << "    round trip mean latency:    " << mean << " ms" << std::endl;
	
	reported_latencies = true;
}

void signal_handler(int signal) 
{
	if (!reported_latencies && !latencies.empty()) {
		latency_messaging_results();
	}
	
	std::exit( signal );
}

void exit_handler() 
{
	if (!reported_latencies && !latencies.empty()) {
		latency_messaging_results();
	}
}

///////////////////////////////////////////////
// CALLBACKS
///////////////////////////////////////////////

// callback handler for the XMPP client connection
void connected()
{
	std::cout << " Connected to server." << std::endl;
	is_connected = true;
}

// callback handler for the XMPP client disconnection
void disconnect( const xmpp::error_t& err )
{
	if (is_connected) {
		std::cout << " Disconnected from server. Error: " << err << std::endl;
		is_connected = false;
	}
}

// callback handler for XMPP message recieved (used for bandwidth testing)
void latency_messageReturned()
{
	timer_end = chrono::system_clock::now();
	
	chrono::duration<float> elapsed_seconds = timer_end - timer_start;
	float time = elapsed_seconds.count() * 1000;
	latencies.push_back(time);
	
	std::cout << "  XMPP ping round trip response time: " << time << "\tms" << std::endl;
}

// callback handler for XMPP message recieved (used for bandwidth testing)
void bandwidth_messageReturned()
{
	returned_messages++;
}

// returns the integer value evaluated from a user-submitted CLI parameter of the form: param=100
int parameter_search( const vector<string>& args, const string& comparator )
{
	for (const string& element : args) {
		int output;
		bool param_included = std::includes(element.begin(), element.end(), comparator.begin(), comparator.end());
		if (param_included) {
			string count = element.substr(element.find("=") + 1);
			try {
				output = std::stoi( count );
				return output;
			}
			catch ( std::exception& exc ) {
				std::cout << "Could not parse comparator " << comparator << " from element '" << element << "' - Error: " << exc.what() << std::endl;
			}
		}
	}
	
	return 0;
}

///////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////

int main(int argc, const char * argv[])
{
	std::atexit(exit_handler);
	std::signal(SIGINT, signal_handler);
	//std::signal(SIGPIPE, SIG_IGN);
	
	// collect command line arguments
	vector<string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( string( argv[arg] ) );
	}
	
	// verify command line inputs
	bool commandlineArgsValid = commandLineArgs.size() >= 4;
	if (!commandlineArgsValid) {
		std::cout << "Usage: " << commandLineArgs.at(0) << " [username] [password] [echo-client] ..." << std::endl;
		std::cout << " Optional parameters:" << std::endl;
		std::cout << "    -l  perform latency test" << std::endl;
		std::cout << "    -b  perform bandwidth test" << std::endl;
		std::cout << "    -lm=<integer>  specify message count for latency test" << std::endl;
		std::cout << "    -bm=<integer>  specify message count for bandwidth test" << std::endl;
		std::cout << " The username must be fully qualified. The echo client must be connected to the same server." << std::endl;
		return -1;
	}
	
	// parse command line inputs
	string raw_username = commandLineArgs.at(1);
	string username = raw_username.substr(0, raw_username.find("@"));
	string password = commandLineArgs.at(2);
	string server = raw_username.substr(raw_username.find("@") + 1);
	string peer = commandLineArgs.at(3);
	peer = peer.substr(0, commandLineArgs.at(3).find("@"));
	
	// determine which tests to perform
	int latency_message_limit = 1000;
	int bandwidth_message_limit = 1000;
	bool performLatency = false;
	bool performBandwidth = false;
	
	// check for bandwidth test...
	auto param = std::find(commandLineArgs.begin(), commandLineArgs.end(), "-b");
	performBandwidth = (param != commandLineArgs.end());
	
	// check for latency test...
	param = std::find(commandLineArgs.begin(), commandLineArgs.end(), "-l");
	performLatency = (param != commandLineArgs.end());
	
	if (!performLatency && !performBandwidth) {
		std::cout << "No test to perform. Neither latency nor bandwidth toggles submitted." << std::endl;
		return 1;
	}

	// check for latency test message total count...
	latency_message_limit = parameter_search(commandLineArgs, "-lm");
	bandwidth_message_limit = parameter_search(commandLineArgs, "-bm");
	
	// create XMPP client connection
	Client client;
	client.setSignalConnect(std::bind(connected));
	client.setSignalDisconnect(std::bind(disconnect, std::placeholders::_1));
	client.setSignalMessage(std::bind(latency_messageReturned));
	client.connect(username, password, server);
	
	// block until client gets connected...
	// wait to get started otherwise, responses are lost (for some reason).
	this_thread::sleep_for(chrono::milliseconds( 1000 ));
	
	// perform latency test...
	latencies.clear();
	int sent_messages = 0;
	while (is_connected && performLatency) {
		if (client.connected()) {
			static bool peer_connected = true;// = client.isPeerConnected(peer);
			if (!peer_connected) {
				std::cerr << "Echo peer is not connected." << std::endl;
				return -1;
			}
			
			if (sent_messages >= latency_message_limit) {
				latency_messaging_results();
				break;
			}
			
			if (latencies.size() == sent_messages) {
				timer_start = chrono::system_clock::now();
				std::cout << "sending test message..." << std::endl;
				client.sendMessage(peer, "body", "subject");
				sent_messages++;
			}
			
			// wait to send next message...
			this_thread::sleep_for(chrono::milliseconds( 1000 ));
		}
	}
	
	// perform bandwidth test...
	client.setSignalMessage(std::bind(bandwidth_messageReturned));
	returned_messages = 0;
	sent_messages = 0;
	while (is_connected && performBandwidth) {
		if (client.connected()) {
			static bool peer_connected = true;// = client.isPeerConnected(peer);
			if (!peer_connected) {
				std::cerr << "Echo peer is not connected." << std::endl;
				return -1;
			}
			
			// send boat load of messages
			while (sent_messages < bandwidth_message_limit) {
				if (sent_messages <= 0) timer_start = chrono::system_clock::now();
				client.sendMessage(peer, "body", "subject");
				sent_messages++;
				
				// must add some delay, gloox library explodes after a while otherwise...
				//this_thread::sleep_for(chrono::milliseconds( 1 ));
			}
			
			if (sent_messages == returned_messages) {
				// end test...
				timer_end = chrono::system_clock::now();
				chrono::duration<float> elapsed_seconds = timer_end - timer_start;
				std::cout << "Bandwidth results for " << sent_messages << " messages" << std::endl;
				std::cout << "    total elapsed time: " << elapsed_seconds.count() << " s" << std::endl;
				break;
			}
		}
	}
	
	// FIN
	client.disconnect();
	
	return 0;
}

