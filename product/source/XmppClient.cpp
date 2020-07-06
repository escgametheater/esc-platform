#include <string>
#include <sstream>
#include <fstream>
#include <time.h>

#include "XmppClient.h"

//int system_log(const char* name, const char* message)
//{
//	time_t rawtime = time(0);
//	std::ofstream logfile;
//	logfile.open("com.esi.EscPlatform.log", std::ios_base::ate);
//	logfile << name << " : " << message << "  " << ctime(&rawtime);
//	logfile.close();
//	
//	return 0;
//}

using namespace esc;
using namespace gloox;

namespace xmpp {

status_t presenceToStatus(const gloox::Presence::PresenceType value)
{
	if ((int) value <= 6) {
		return xmpp::status_t((int)value);
	}
	return xmpp::Invalid;
}

std::string statusString(const status_t value)
{
	switch (value) {
		case Available: return "available";
			break;
		case Chat: return "chat";
			break;
		case Away: return "away";
			break;
		case DND: return "do not disturb";
			break;
		case XA: return "extended away";
			break;
		case Unavailable: return "unavailable";
			break;
			
		case Invalid: return "invalid";
		default:
			break;
	}
}

Client::Client() : mInitialConnection( false )
{
	mListener = new xmpp::Listener(this);
}

Client::~Client()
{
	//this->disconnect();
	delete mListener;
}

bool Client::connect(const std::string& xmppUser, const std::string& xmppPassword, const std::string& xmppServer, const int xmppPort, const bool enableLogging)
{
	if (xmppUser.empty() || xmppServer.empty()) return false;
	
	mListener->setup(xmppUser, xmppPassword, xmppServer, xmppPort, enableLogging);
	
	mEventQueue.clear();
	mStateQueue.clear();
	mPresenceQueue.clear();
	
	// spawn listening thread...
    if (pthread_create(&mListenerThread, NULL, &Client::spawnThread, this)) {
        io::info("xmpp::Client - Could not create thread");
        return false;
    }
	
	// in case we need to register this account ...
	mUsername = xmppUser;
	mPassword = xmppPassword;
	mServer = xmppServer;
	mInitialConnection = true;
	
	return true;
}
	
bool Client::disconnect()
{
	if (mListener->isConnected()) {
		mInitialConnection = false;
		mListener->closeConnection();
		
		int join_result = pthread_join(mListenerThread, NULL);
		if (0 == join_result) {
			return true;
		}
		else {
			char buf[3];
			std::sprintf(buf, "%d", join_result);
			std::string return_code(buf);
			io::info("Client::disconnect - thread failed to join: " + return_code);
			return false;
		}
	}
	
	return false;
}

void Client::onConnect()
{
	State s = CONNECTED;
	mStateQueue.push_back(s);

#if defined( _LIBCPP_VERSION )
	
	if (mSignalConnect)
		mSignalConnect();
	
#endif
}

void Client::onDisconnect( gloox::ConnectionError e )
{
	bool shouldRegister = false;
	
	error_t err;
	switch (e) {
		case ConnNoError:
			err = NoError;
			break;
		
		case ConnStreamError:
		case ConnStreamClosed:
		case ConnIoError:
			err = StreamError;
			break;
			
		case ConnProxyAuthRequired:
		case ConnProxyAuthFailed:
		case ConnProxyNoSupportedAuth:
		case ConnTlsFailed:
		case ConnTlsNotAvailable:
		case ConnNoSupportedAuth:
		case ConnAuthenticationFailed:
			err = AuthenticationError;
			shouldRegister = mInitialConnection;
			break;
			
		case ConnParseError:
		case ConnStreamVersionError:
			err = ProtocolError;
			break;
			
		case ConnUserDisconnected:
		case ConnNotConnected:
		case ConnDnsError:
		case ConnConnectionRefused:
			err = ConnectionError;
			break;
			
		case ConnOutOfMemory:
		case ConnCompressionFailed:
		default:
			err = GeneralError;
	}
	
	if (shouldRegister) {
		mAccountCreator.init(mUsername, mPassword, mServer);
		mInitialConnection = false;
		if (mAccountCreator.status() == xmpp::SUCCESS) {
			this->connect(mUsername, mPassword, mServer);
		}
	}
	else {
		State s = DISCONNECTED;
		mStateQueue.push_back(s);
		
#if defined( _LIBCPP_VERSION )
		
		if (mSignalDisconnect)
			mSignalDisconnect(err);
		
#endif
	}
}

void Client::onTLSConnect( const CertInfo& info )
{
	State s = TLS_CONNECTED;
	mStateQueue.push_back(s);
	
#if defined( _LIBCPP_VERSION )
	
	if (mSignalTlsConnect)
		mSignalTlsConnect();
	
#endif
}

void Client::onRegistration( gloox::RegistrationResult result )
{
	// necessary??
	
#if defined( _LIBCPP_VERSION )
	
	if (mSignalRegistration)
		mSignalRegistration();
	
#endif
}

void Client::handleMessage( const Message& msg, MessageSession* sess )
{
	// process information from message
	std::string name = msg.from().username();

	std::map<std::string, xmpp::Peer>::const_iterator peer_itr = mRoster.find(name);
	xmpp::Peer peer;
	if (peer_itr != mRoster.end()) 
		peer = peer_itr->second;

	Event e;
	e.username = name;
	e.message = msg.body();
	mEventQueue.push_back(e);
	
#if defined( _LIBCPP_VERSION )
	
	if (mSignalHandleMsg)
		mSignalHandleMsg( peer, msg.subject(), msg.body() );
	
#endif
}

void Client::handleRoster( const Roster& roster )
{
	// we will only want to ensure that the game-engine user is in the
	// roster for clients that are NOT the docent remote...
	bool subscribeToGame = (mUsername != kDocentUsername);

#if defined( _LIBCPP_VERSION )
	std::list<std::string> userlist;
#endif
	
	mRoster.clear();
	
	Roster::const_iterator itr;
	for ( itr = roster.begin(); itr != roster.end(); ++itr ) {
		const std::string& username = (*itr).second->jidJID().username();
		
		// check to see that we have a game engine in our roster...
		if (kGameEngineUsername == username) subscribeToGame = false;
		
		// create roster of Peer instances...
		mRoster.insert(std::pair<std::string, xmpp::Peer>(username, xmpp::Peer(username)));

#if defined( _LIBCPP_VERSION )
		userlist.push_back(username);
#endif
		
	}
	
	if (subscribeToGame) {
		std::string recipient(kGameEngineUsername);
		recipient += "@";
		recipient += mListener->server();
		JID jid(recipient);
		mListener->subscribeTo( jid );
	}
	
	State s = ROSTER_LOADED;
	mStateQueue.push_back(s);
	
#if defined( _LIBCPP_VERSION )

	if (mSignalRoster)
		mSignalRoster(userlist);
	
#endif
}

void Client::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& message )
{	
	const std::string& name = item.jidJID().username();

	std::map<std::string, xmpp::Peer>::iterator peer_itr = mRoster.find(name);
	if (peer_itr != mRoster.end()) {
		peer_itr->second.setStatus(xmpp::presenceToStatus(presence));
	}

	PresenceEvent e;
	e.username = name;
	e.status = xmpp::presenceToStatus(presence);
	mPresenceQueue.push_back(e);
	
#if defined( _LIBCPP_VERSION )
	
	if (mSignalRosterPresence)
		mSignalRosterPresence( peer_itr->second, resource, xmpp::presenceToStatus(presence), message );
	
#endif
	
}

bool Client::sendMessage(const std::string& recipient, const std::string& message, const std::string& subject)
{
	if (recipient.empty()) return false;
	
	std::string r(recipient);
	r += "@";
	r += mListener->server();
	JID jid(r);
	
	return mListener->sendMessage(jid, message, subject);
}

xmpp::status_t Client::getStatusForPeer(const std::string& username) const
{
	std::map<std::string, xmpp::Peer>::const_iterator iter = mRoster.find(username);
	if (iter != mRoster.end())
		return iter->second.status();
	
	return xmpp::Invalid;
}

bool Client::isPeerConnected(const std::string& username) const
{
	std::map<std::string, xmpp::Peer>::const_iterator iter = mRoster.find(username);
	if (iter != mRoster.end())
		return iter->second.isConnected();
	
	return false;
}

bool operator==( const xmpp::Peer& lhs, const xmpp::Peer& rhs)
{
	return lhs.username() == rhs.username();
}

bool operator!=( const xmpp::Peer& lhs, const xmpp::Peer& rhs)
{
	return lhs.username() != rhs.username();
}

}