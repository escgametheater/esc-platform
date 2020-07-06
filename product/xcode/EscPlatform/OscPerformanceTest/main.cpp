//
//  main.cpp
//  OscPerformanceTest
//
//  Created by Caleb Johnston on 2/13/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <chrono>
#include <list>
#include <numeric>
#include <algorithm>

#include "OscSender.h"
#include "OscListener.h"

using namespace std;
using namespace osc;

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
	float number_sum = std::accumulate(numbers.begin(), numbers.end(), 0);
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

// callback handler for OSC message recieved (used for bandwidth testing)
void latency_messageReturned()
{
	timer_end = chrono::system_clock::now();
	
	std::chrono::duration<float> elapsed_seconds = timer_end - timer_start;
	float time = elapsed_seconds.count() * 0.001;
	latencies.push_back(time);
	
	std::cout << "  osc ping round trip response time: " << time << "\tms" << std::endl;
}

// callback handler for OSC message recieved (used for bandwidth testing)
void bandwidth_messageReturned()
{
	returned_messages++;
}

///////////////////////////////////////////////
int main(int argc, const char * argv[])
{
	// collect command line arguments
	vector<string> commandLineArgs;
	for (int arg = 0; arg < argc; ++arg ) {
		commandLineArgs.push_back( string( argv[arg] ) );
	}
	
	// verify command line inputs
	bool commandlineArgsValid = commandLineArgs.size() >= 4;
	if (!commandlineArgsValid) {
		std::cout << "Usage: " << commandLineArgs.at(0) << " [echo-peer] [peer-port] [listen-port] ..." << std::endl;
		std::cout << "    -l  perform latency test" << std::endl;
		std::cout << "    -b  perform bandwidth test" << std::endl;
		return -1;
	}
	
	// parse command line inputs
	string peer_endpoint = commandLineArgs.at(1);
	int peer_port = atoi(commandLineArgs.at(2).c_str());
	int listen_port = atoi(commandLineArgs.at(3).c_str());
	
	// determine which tests to perform
	bool performLatency = true;
	bool performBandwidth = true;
	if (5 == commandLineArgs.size()) {
		if ("-l" == commandLineArgs.at(4)) {
			performBandwidth = false;
		}
		else if ("-b" == commandLineArgs.at(4)) {
			performLatency = false;
		}
	}
	
	// start listening...
	osc::Listener listener;
	listener.setup(listen_port);
	listener.setMessageReceived(std::bind(latency_messageReturned));
	std::cout << "Listening on port: " << listen_port << std::endl;
	
	// setup sender
	osc::Sender sender;
	sender.setup(peer_endpoint, peer_port);
	
	// perform latency test...
	latencies.clear();
	int sent_messages = 0;
	int latency_message_limit = 10;	// TODO: make customizable
	while (performLatency) {
		while (listener.hasWaitingMessages()) {
			if (sent_messages >= latency_message_limit) {
				// end test...
				float min = computeMin(latencies);
				float max = computeMax(latencies);
				float mean = computeMean(latencies);
				std::cout << "Latency results:" << std::endl;
				std::cout << "    round trip minimum latency: " << min << " ms" << std::endl;
				std::cout << "    round trip maximum latency: " << max << " ms" << std::endl;
				std::cout << "    round trip mean latency:    " << mean << " ms" << std::endl;
				performLatency = false;
			}
			
			if (latencies.size() == sent_messages) {
				timer_start = chrono::system_clock::now();
				std::cout << "sending test message..." << std::endl;
				osc::Message msg;
				msg.setAddress("/test/latency");
				msg.addStringArg("string argument");
				sender.sendMessage(msg);
				sent_messages++;
			}
			
			// wait to send next message...
			std::this_thread::sleep_for(std::chrono::milliseconds( 1000 ));
		}
	}
	
	// perform bandwidth test...
	listener.setMessageReceived(std::bind(bandwidth_messageReturned));
	returned_messages = 0;
	sent_messages = 0;
	int bandwidth_message_limit = 1000;	// TODO: make customizable
	while (performBandwidth) {
		while (listener.hasWaitingMessages()) {
			// send boat load of messages
			while (sent_messages < bandwidth_message_limit) {
				timer_start = chrono::system_clock::now();
				osc::Message msg;
				msg.setAddress("/test/bandwidth");
				msg.addStringArg("string argument");
				sender.sendMessage(msg);
				sent_messages++;
				
				// must add some delay, gloox library explodes after a while otherwise...
				std::this_thread::sleep_for(std::chrono::milliseconds( 2 ));
			}
			
			if (sent_messages == returned_messages) {
				// end test...
				timer_end = chrono::system_clock::now();
				std::chrono::duration<float> elapsed_seconds = timer_end - timer_start;
				std::cout << "Bandwidth results for " << sent_messages << " messages" << std::endl;
				std::cout << "    total elapsed time: " << elapsed_seconds.count() << " ms" << std::endl;
				performBandwidth = false;
			}
		}
	}
	
	// FIN
	sender.shutdown();
	
	return 0;
}

