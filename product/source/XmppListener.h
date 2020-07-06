//
//  PluginInterface.cpp
//  ESC-Unity-Plugin
//
//  Created by Caleb Johnston on 1/27/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#pragma once

#include "gloox/gloox.h"
#include "gloox/client.h"
#include "gloox/message.h"
#include "gloox/registration.h"
#include "gloox/rostermanager.h"
#include "gloox/messagehandler.h"
#include "gloox/messagesession.h"
#include "gloox/rosterlistener.h"
#include "gloox/connectionlistener.h"
#include "gloox/registrationhandler.h"
#include "gloox/presencehandler.h"
#include "gloox/messagesessionhandler.h"
#include "gloox/messageeventhandler.h"
#include "gloox/messageeventfilter.h"
#include "gloox/chatstatehandler.h"
#include "gloox/chatstatefilter.h"
#include "gloox/disco.h"
#include "gloox/lastactivity.h"
#include "gloox/loghandler.h"
#include "gloox/logsink.h"
#include "gloox/connectiontcpclient.h"
#include "gloox/connectionsocks5proxy.h"
#include "gloox/connectionhttpproxy.h"

#ifndef _WIN32
# include <unistd.h>
#endif

#include <iostream>
#include <string>

namespace xmpp {	

class Client;

/**
 * class Listener
 *
 * The Listener class manipulates a gloox::Client instance and abstracts
 * connection details. It also implements numerous interfaces for gloox
 * asynchronous operations (message handling, roster handling, presence 
 * handling, etc). 
 *
 */
class Listener : public gloox::ConnectionListener, gloox::MessageHandler, gloox::RosterListener,
	gloox::MessageSessionHandler, gloox::LogHandler, gloox::MessageEventHandler,
	gloox::ChatStateHandler, gloox::PresenceHandler, gloox::RegistrationHandler
{
public:
	//! Listener C-stor
	Listener(xmpp::Client* delegate = 0);
	//! Listener D-stor
	virtual ~Listener();

	//! Setup function initializes the internal gloox members. Does not start connection with server.
	void setup( const std::string& username, const std::string& password, const std::string& server, const int port = 5222, const bool enableLogging = false );

	//! Starts a thread-blocking while loop to receive data on the TCP socket connection using internal gloox client
	void openConnection(bool toggle = false);
	
	//! Disconnects internal gloox client from TCP socket
	void closeConnection();
	
	/**
	 * Dispatches XMPP CHAT message with the given input parameters
	 *
	 * @param recipient the fully qualified XMPP identifier to send the message to
	 * @param message the message body formatted as a string
	 * @param subject the message subject (optional)
	 * @return TRUE if able to send message, FALSE otherwise
	 */
	virtual bool sendMessage( const gloox::JID& recipient, const std::string& message, const std::string& subject = "" );
	
	/**
	 * Dispatches XMPP subscription request with the given JID
	 *
	 * @param recipient the fully qualified XMPP identifier to send the message to
	 * @return TRUE if able to send a request, FALSE otherwise
	 */
	virtual bool subscribeTo( const gloox::JID& recipient );
	
	//! Returns TRUE if internal gloox client is connected, FALSE otherwise
	bool isConnected() const { return mIsConnected; }
	
	//! XMPP server accessor method
	std::string server() const { return mServer; }
	
	//! XMPP account username accessor method
	std::string username() const { return mUsername; }
	
	//! internal gloox member accessor method
	gloox::Client& getClient() const { return *mClient; }
	
protected:
	// satisfy interface for gloox::ConnectionListener
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual bool onTLSConnect( const gloox::CertInfo& info );
	
	// satisfy interface for gloox::MessageHandler
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession * /*session*/ );
	
	// satisfy interface for gloox::MessageEventHandler
	virtual void handleMessageEvent( const gloox::JID& from, gloox::MessageEventType event );
	
	// satisfy interface for gloox::ChatStateHandler
	virtual void handleChatState( const gloox::JID& from, gloox::ChatStateType state );
	
	// satisfy interface for gloox::MessageSessionHandler
	virtual void handleMessageSession( gloox::MessageSession* session );
	
	// satisfy interface for gloox::LogHandler
	virtual void handleLog( gloox::LogLevel level, gloox::LogArea area, const std::string& message );
	
	// satisfy interface for gloox::PresenceHandler
	virtual void handlePresence( const gloox::Presence& presence );

	// satisfy interface for gloox::RosterListener
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource,
									   gloox::Presence::PresenceType presence, const std::string& msg );
	virtual void handleRosterError( const gloox::IQ& iq ) { /* no-op */ }
	virtual void handleItemAdded( const gloox::JID& jid ) { /* no-op */ }
	virtual void handleItemSubscribed( const gloox::JID& jid ) { /* no-op */ }
	virtual void handleItemRemoved( const gloox::JID& jid ) { /* no-op */ }
	virtual void handleItemUpdated( const gloox::JID& jid ) { /* no-op */ }
	virtual void handleItemUnsubscribed( const gloox::JID& jid ) { /* no-op */ }
	virtual void handleSelfPresence( const gloox::RosterItem& item, const std::string& resource,
									 gloox::Presence::PresenceType presence, const std::string& msg ) { /* no-op */ }
	virtual bool handleSubscriptionRequest( const gloox::JID& jid, const std::string& msg );
	virtual bool handleUnsubscriptionRequest( const gloox::JID& jid, const std::string& msg );
	virtual void handleNonrosterPresence( const gloox::Presence& presence ) { /* no-op */ }
	
	// satisfy interface for gloox::RegistrationHandler
	virtual void handleRegistrationFields( const gloox::JID& from, int fields, std::string instructions );
	virtual void handleAlreadyRegistered( const gloox::JID& from );
	virtual void handleRegistrationResult( const gloox::JID& from, gloox::RegistrationResult regResult );
	virtual void handleDataForm( const gloox::JID& from, const gloox::DataForm& form ) { /* no-op */ }
	virtual void handleOOB( const gloox::JID& from, const gloox::OOB& oob ) { /* no-op */ }

private:
	std::string mServer;
	std::string mUsername;
	
	bool mIsConnected;
	
	gloox::Client* mClient;
	gloox::MessageSession* mSession;
	gloox::MessageEventFilter* mMessageEventFilter;
	gloox::ChatStateFilter* mChatStateFilter;
	gloox::Registration* mRegistration;
	
	xmpp::Client* mDelegate;
};

}	/* end namespace xmpp */
