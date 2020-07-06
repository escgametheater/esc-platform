//
//  AccountEraser.cpp
//  ESC-Unity-Plugin
//
//  Created by Caleb Johnston on 1/27/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#pragma once

#include "gloox/gloox.h"
#include "gloox/client.h"
#include "gloox/registration.h"
#include "gloox/connectionlistener.h"
#include "gloox/registrationhandler.h"
#include "gloox/disco.h"
#include "gloox/loghandler.h"
#include "gloox/logsink.h"

#include "AccountRegistration.h"

#ifndef _WIN32
# include <unistd.h>
#endif

#include <iostream>
#include <string>

namespace xmpp {

/**
 * class AccountEraser
 *
 * Some edge cases are not handled:
 *  - If the server is unreachable, it will not be reported.
 *  - If a TLS connection cannot be established, it will not be reported.
 *  - Others?
 */
class AccountEraser : public gloox::ConnectionListener, gloox::LogHandler, gloox::RegistrationHandler
{
public:
	//! C-stor simply initializes members
	AccountEraser();
	//! D-stor frees resources
	~AccountEraser();
	
	/**
	 * Starts the account registration process using the given inputs
	 *
	 * @param username the account name (required)
	 * @param password the account password (required)
	 * @param host the server to register the account with (required)
	 * @param port optional port number (default is 5222)
	 * @param enableLogging optional flag to log messages (defaults to false)
	 */
	bool init( const std::string& username, const std::string& password, const std::string& host,
			  const int port = 5222, const bool enableLogging = false );
	
	//! returns INCOMPETE until the process is finished, then it returns SUCCESS or FAILURE
	OperationStatus status() const { return mStatus; }
	
protected:
	// satisfy interface for gloox::ConnectionListener
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e ) { /* no-op */ }
	virtual bool onTLSConnect( const gloox::CertInfo& info ) { /* no-op */ return true; }
	
	// satisfy interface for gloox::LogHandler
	virtual void handleLog( gloox::LogLevel level, gloox::LogArea area, const std::string& message );
	
	// satisfy interface for gloox::RegistrationHandler
	virtual void handleRegistrationFields( const gloox::JID& from, int fields, std::string instructions ) { /* no-op */ }
	virtual void handleAlreadyRegistered( const gloox::JID& from );
	virtual void handleRegistrationResult( const gloox::JID& from, gloox::RegistrationResult regResult );
	virtual void handleDataForm( const gloox::JID& from, const gloox::DataForm& form ) { /* no-op */ }
	virtual void handleOOB( const gloox::JID& from, const gloox::OOB& oob ) { /* no-op */ }
	
private:
	OperationStatus mStatus;//!< Stores operation status
	
	gloox::Client* mClient;	//!< Internal gloox client object to initiate connection
	gloox::Registration* mRegistration;	//!< Internal gloox registration object to create account
};

}	/* end namespace xmpp */
