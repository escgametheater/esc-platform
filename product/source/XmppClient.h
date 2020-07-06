//
//  PluginInterface.cpp
//  ESC-Unity-Plugin
//
//  Created by Caleb Johnston on 1/27/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//
#pragma once

#include <string>
#include <list>
#include <map>
#include <deque>
#include <functional>
#include <pthread.h>

#include "gloox/gloox.h"
#include "gloox/jid.h"

#include "AccountRegistration.h"
#include "XmppListener.h"
#include "Config.h"
#include "Logger.h"

//int system_log(const char* name, const char* message);

namespace xmpp {

typedef pthread_t ThreadRef;


/**
 * Structure for describing XMPP events
 */
typedef struct event_t {
	std::string username;
	std::string message;
} Event;

/**
 * Structure for describing XMPP presence updates
 */
typedef struct presence_event_t {
	std::string username;
	int status;
} PresenceEvent;

/**
 * Enum for describing XMPP status changes
 */
typedef enum state_t {
	CONNECTED,
	TLS_CONNECTED,
	DISCONNECTED,
	ROSTER_LOADED,
	ERROR
} State;

/**
 * Enum for describing XMPP connection status changes
 */
enum error_t {
	NoError,				//!< everything was fine, no error
	GeneralError,			//!< non-descript
	AuthenticationError,	//!< problem with auth info
	ProtocolError,			//!< problem with following protocol
	StreamError,			//!< problem stream writing or reading
	ConnectionError			//!< connection dropped
};

/**
 * Enum for describing XMPP user status info
 */
enum status_t {
	Available,    //!< online
	Chat,         //!< available for chat
	Away,         //!< away
	DND,          //!< DND (Do Not Disturb)
	XA,           //!< eXtended Away
	Unavailable,  //!< offline
	Invalid       //!< invalid
};

//! Takes gloox PresenceType and maps it to a status enum
status_t presenceToStatus(const gloox::Presence::PresenceType value);

//! Takes status enum and maps it to an STL string
std::string statusString(const status_t value);
	
/**
 * class Peer
 *
 * The Peer class contains the data necessary to represent a peer client
 * that is on the roster of the client connected to the server (this one).
 *
 */
class Peer {
public:
	/**
	 * Constructor only initializes members
	 */
	Peer(const std::string& user = "unknown", bool conn = false, status_t status = Invalid) : mUsername(user), mConnected(conn), mStatus(status) {}
	~Peer() {}
	
	// connection status accessor methods
	bool isConnected() const { return mConnected; }
	void setConnected(const bool value) { mConnected = value; }
	
	// username accessor methods
	const std::string& username() const { return mUsername; }
	void setUsername(const std::string& value) { mUsername = value; }
	
	// state status accessor methods
	status_t status() const { return mStatus; }
	void setStatus(const status_t value)
	{
		mStatus = value;
		mConnected = mStatus < 5;
	}
	
private:
	bool mConnected;
	std::string mUsername;
	status_t mStatus;
};
	
#if defined( _LIBCPP_VERSION ) 
	
	typedef std::function<void()> SignalCallback;
	typedef std::function<void(const xmpp::error_t&)> SignalCallbackDisconnect;
	typedef std::function<void(const std::list<std::string>&)> SignalCallbackRoster;
	typedef std::function<void(const xmpp::Peer&, const std::string&, const std::string&)> SignalCallbackMsg;
	typedef std::function<void(const xmpp::Peer&, const std::string&, xmpp::status_t, const std::string&)> SignalCallbackPresence;

#endif

/**
 * class Client
 *
 * The Client class manipulates an xmpp::Listener instance and abstracts
 * connection details. It also stores a string-to-Peer map representing
 * the client's roster. 
 *
 */
class Client {
public:
	/**
	 * Constructor initializes listener member
	 */
	Client();
	
	/**
	 * Destructor joins thread if necessary
	 */
	~Client();
	
	/** 
	 * Attempt to establish a connection using the specified credentials and server endpoint.
	 * Returns FALSE if the input data is invalid. Also uses the mInitialConnection flag to
	 * determine whether or not it should attempt a one-time account registration after the 
	 * first failure to authenticate using the given account credentials. This check is
	 * performed asynchronously so it must be stored statefully.
	 * 
	 * @param xmppUser the username for the account to which the client will connect
	 * @param xmppPassword the password that corresponds to the xmppUser account
	 * @param xmppServer the server address to which the xmppUser account belongs
	 * @param xmppPort the port number on the server to connect on (defaults to 5222)
	 * @param enableLogging will write logs if enabled (defaults to FALSE)
	 */
	bool	connect(const std::string& xmppUser, const std::string& xmppPassword, const std::string& xmppServer, const int xmppPort = 5222, const bool enableLogging = false);
	
	/** 
	 * Disconnects if connected. Returns FALSE if disconnection isn't possible. 
	 *
	 * NOTE: also invalidates the mInitialConnection flag.
	 */
	bool	disconnect();
	
	/**
	 * Attempts to send message using the input data. Returns TRUE if capable of sending.
	 *
	 * WARNING: Assumes recipient is on the same server hosting the current connection!
	 */
	bool	sendMessage( const std::string& recipient, const std::string& message, const std::string& subject = "" );
	
	/* Returns status for Peer given the input username */
	xmpp::status_t getStatusForPeer(const std::string& username) const;
	
	/* Returns TRUE if Peer given by the input username is connected */
	bool	isPeerConnected(const std::string& username) const;
	
	/* Returns status of client connection */
	bool	connected() const { return mListener->isConnected(); }
	
	/* returns const reference to internally stored roster */
	const	std::map<std::string, xmpp::Peer>& getRoster() const { return mRoster; }
	
	std::deque<Event>& getEventQueue() { return mEventQueue; }
	std::deque<State>& getStateQueue() { return mStateQueue; }
	std::deque<PresenceEvent>& getPresenceQueue() { return mPresenceQueue; }
	
#if defined( _LIBCPP_VERSION )
	
	void	setSignalConnect(SignalCallback cb) { mSignalConnect = cb; }
	void	setSignalTlsConnect(SignalCallback cb) { mSignalTlsConnect = cb; }
	void	setSignalDisconnect(SignalCallbackDisconnect cb) { mSignalDisconnect = cb; }
	void	setSignalRoster(SignalCallbackRoster cb) { mSignalRoster = cb; }
	void	setSignalMessage(SignalCallbackMsg cb) { mSignalHandleMsg = cb; }
	void	setSignalRosterPresence(SignalCallbackPresence cb) { mSignalRosterPresence = cb; }
	void	setSignalRegistration(SignalCallback cb) { mSignalRegistration = cb; }
	
#endif
	
protected:
	friend class xmpp::Listener;
	
	//! Handler for the connection status. Invalidates internal mInitialConnection flag.
	virtual void onConnect();
	
	/**
	 * Handler for socket disconnection. Checks internal mInitialConnection flag in combination
	 * with the connectionError parameter to determine if it should attempt to create an ccount.
	 * Initiates the account creation process using the AccountRegistration member if necessary.
	 */
	virtual void onDisconnect( gloox::ConnectionError e );
	virtual void onTLSConnect( const gloox::CertInfo& info );
	virtual void onRegistration( gloox::RegistrationResult result );
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession *sess );
	virtual void handleRoster( const gloox::Roster& roster );
	virtual void handleRosterPresence( const gloox::RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& message );
	
private:
	//! local state flag used to decide whether or not to create an account on authentication failure
	bool mInitialConnection;
	//! instance used to create a new account if necessary (used in conjunction with mInitialConnection)
	xmpp::AccountRegistration mAccountCreator;
	//! account credentials used to login or create a new account on login failure
	std::string mUsername, mPassword, mServer;
	
	//! Used as an interface to gloox
	xmpp::Listener* mListener;
	//! pthread object used to open connection upon the mListener member
	ThreadRef mListenerThread;
	//! local representation of the remote roster list
	std::map<std::string, xmpp::Peer> mRoster;

	//! Queue used to store all events waiting to be processed
	std::deque<Event> mEventQueue;
	//! Queue used to store all status changes waiting to be processed
	std::deque<State> mStateQueue;
	//! Queue used to store all presence changes waiting to be processed
	std::deque<PresenceEvent> mPresenceQueue;
	
#if defined( _LIBCPP_VERSION )
	
	SignalCallback mSignalConnect, mSignalTlsConnect, mSignalRegistration;
	SignalCallbackDisconnect mSignalDisconnect;
	SignalCallbackRoster mSignalRoster;
	SignalCallbackMsg mSignalHandleMsg;
	SignalCallbackPresence mSignalRosterPresence;
	
#endif
	
	//! anonymous function pointer used with pthreads
	static void* spawnThread( void* instance )
	{
		((Client*) instance)->mListener->openConnection(false);
		
		return NULL;
	}
};

#pragma mark comparison operators

bool operator==( const xmpp::Peer& lhs, const xmpp::Peer& rhs);
bool operator!=( const xmpp::Peer& lhs, const xmpp::Peer& rhs);

}	/* end namespace xmpp */
