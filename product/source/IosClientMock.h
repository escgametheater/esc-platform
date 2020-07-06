//
//  IosClientMock.h
//  ESC-Unity-Plugin
//
//  Created by Caleb Johnston on 1/27/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//
#pragma once

#include <string>
#include <list>
#include <vector>
#include <functional>
#include <thread>
#include <memory>

#include "gloox/gloox.h"
#include "gloox/jid.h"

#include "OscSender.h"
#include "XmppClient.h"

/**
 * class IosClientMock 
 *
 * This class is designed to provide an interface to an iOS mobile device to
 * demonstrate the communication protocol.
 *
 */
class IosClientMock {
public:
	typedef struct config_t {
		std::string server;		//!< the endpoint string server address
		int oscPort;			//!< the endpoint OSC port to connect to
		int totalClients;		//!< total number of clients to create
		int oscFrequency;		//!< messages per second for each client
		int xmppFrequency;		//!< messages per second for each client
		int oscPayloadSize;		//!< size in bytes for each message
		int xmppPayloadSize;	//!< size in bytes for each message
	} Configuration;
public:
	/**
	 * Constructor 
	 */
	IosClientMock();
	
	/**
	 * Destructor 
	 */
	virtual ~IosClientMock();
	
	/**
	 * begin protocol mock
	 */
	void start( const Configuration config );
	
protected:
//	virtual void handleOscMessage( const osc::Message* msg );
	virtual void onConnect();
	virtual void onTlsConnect();
	virtual void onDisconnect( const xmpp::error_t& error );
	virtual void handleXmppMessage( const xmpp::Peer& peer, const std::string& body, const std::string& subject );
	virtual void handleXmppRoster( const std::list<std::string>& roster );
	virtual void handleXmppRosterPresence( const xmpp::Peer& peer, const std::string& resource, xmpp::status_t status, const std::string& message );
	
	void spawnOscThread();
	void spawnXmppThread();
	
private:
	bool mRunning;
	int mTotalClients;
	
	Configuration mConfig;
	osc::Sender mOscSender;
	std::vector<xmpp::Client> mClients;
	
	std::shared_ptr<std::thread> mOscThread;
	std::shared_ptr<std::thread> mXmppThread;
};
