//
//  AccountRegistration.cpp
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

#ifndef _WIN32
# include <unistd.h>
#endif

#include <iostream>
#include <string>

namespace xmpp {
	
typedef enum operation_status_t {
	INCOMPLETE,
	SUCCESS,
	FAILURE
} OperationStatus;
	
/**
 * class AccountRegistration
 *
 * The AccountRegistration class is designed to completely handle 
 * the asynchronous process of creating a new account on the XMPP
 * server using the XEP-0077 feature. We expect that this will only
 * be performed by the iOS clients when they are being initialized.
 *
 * Some edge cases are not handled: 
 *  - If the server requires more than a username and password, then the process will always fail.
 *  - If the server is unreachable, it will not be reported.
 *  - If a TLS connection cannot be established, it will not be reported.
 *  - If the server does not implement XEP-0077, the failure will not be specified.
 */
class AccountRegistration : public gloox::ConnectionListener, gloox::LogHandler, gloox::RegistrationHandler
{
public:
	//! C-stor simply initializes members
	AccountRegistration();
	//! D-stor frees resources
	~AccountRegistration();
	
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
	virtual void handleRegistrationFields( const gloox::JID& from, int fields, std::string instructions );
	virtual void handleAlreadyRegistered( const gloox::JID& from );
	virtual void handleRegistrationResult( const gloox::JID& from, gloox::RegistrationResult regResult );
	virtual void handleDataForm( const gloox::JID& from, const gloox::DataForm& form ) { /* no-op */ }
	virtual void handleOOB( const gloox::JID& from, const gloox::OOB& oob ) { /* no-op */ }
	
private:
	OperationStatus mStatus;		//!< Stores operation status
	
	std::string mHostname;	//!< Stores hostname temporarily
	std::string mUsername;	//!< Stores username temporarily
	std::string mPassword;	//!< Stores password temporarily
	
	gloox::Client* mClient;	//!< Internal gloox client object to initiate connection
	gloox::Registration* mRegistration;	//!< Internal gloox registration object to create account
};

}	/* end namespace xmpp */
