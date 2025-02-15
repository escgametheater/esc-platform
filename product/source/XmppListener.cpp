

#include "XmppListener.h"
#include "XmppClient.h"
#include "Logger.h"

using namespace gloox;
using namespace esc;

namespace xmpp {

Listener::Listener(xmpp::Client* delegate)
:	mDelegate(delegate), mClient( 0 ), mSession( 0 ), mMessageEventFilter( 0 ),
	mChatStateFilter( 0 ), mIsConnected(false), mServer(""), mUsername(""), mRegistration(0)
{
}

Listener::~Listener()
{
	if (mClient) mClient->disconnect();
	
	// TODO: Free resources!!
	
	delete mClient;
	mClient = 0;
}

void Listener::setup( const std::string& username, const std::string& password, const std::string& server, const int port, const bool enableLogging )
{
	std::string address = username;
	address += "@";
	if (server.empty()){
		address += mServer;
	}
	else {
		mServer = server;
		address += server;
	}
	
	JID jid( address );
	
	mClient = new gloox::Client( jid, password, port );
	mClient->registerPresenceHandler( this );
	mClient->registerConnectionListener( this );
	mClient->registerMessageSessionHandler( this, 0 );
	mClient->rosterManager()->registerRosterListener(this);
	mClient->disco()->setVersion( "EscMessanger", GLOOX_VERSION, "Darwin" );
	mClient->disco()->setIdentity( "client", "bot" );
	mClient->disco()->addFeature( XMLNS_CHAT_STATES );
	
#ifdef DEBUG
	if (enableLogging) 
		mClient->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
#endif
}

void Listener::openConnection(bool toggle)
{
	if (!mIsConnected && mClient) {
		if ( mClient->connect( toggle ) ) {
			gloox::ConnectionError ce = ConnNoError;
			while ( ce == ConnNoError && mClient ) {
				ce = mClient->recv();
			}
		}
	}
}
	
void Listener::closeConnection()
{
	if (mIsConnected && mClient) {
		mClient->disconnect();
	}
}

void Listener::onConnect()
{
	mIsConnected = true;
	
	if (mRegistration) mRegistration->fetchRegistrationFields();
	
	if (mDelegate) mDelegate->onConnect();
}

void Listener::onDisconnect( gloox::ConnectionError e )
{
	mIsConnected = false;
	
	if (mDelegate) mDelegate->onDisconnect(e);
}

bool Listener::onTLSConnect( const CertInfo& info )
{
	mIsConnected = true;
	
	// info.status
	// info.issuer
	// info.server
	// info.protocol
	// info.mac
	// info.cipher
	// info.compression
	// time_t from( info.date_from );
	// time_t to( info.date_to );
	// ctime( &from )
	// ctime( &to )
		
	if (mDelegate) mDelegate->onTLSConnect(info);
		
	return true;
}

void Listener::handleMessage( const Message& msg, MessageSession* sess )
{
	// msg.subtype()
	// msg.subject()
	// msg.body()
	// msg.thread()

	if (mDelegate) mDelegate->handleMessage( msg, sess );
}

void Listener::handleMessageEvent( const JID& from, MessageEventType event ) {}

void Listener::handleChatState( const JID& from, ChatStateType state ) {}

void Listener::handleMessageSession( MessageSession* session )
{
	// this example can handle only one session. so we get rid of the old session
	mClient->disposeMessageSession( mSession );
	
	mSession = session;
	mSession->registerMessageHandler( this );
	mMessageEventFilter = new MessageEventFilter( mSession );
	mMessageEventFilter->registerMessageEventHandler( this );
	mChatStateFilter = new ChatStateFilter( mSession );
	mChatStateFilter->registerChatStateHandler( this );
}

void Listener::handleRoster( const Roster& roster )
{	
	if (mDelegate) mDelegate->handleRoster(roster);
}

void Listener::handleRosterPresence(const RosterItem& item, const std::string& resource,
										Presence::PresenceType presence, const std::string& msg )
{
	//item.jid()
	//presence
	
	if (mDelegate) mDelegate->handleRosterPresence(item, resource, presence, msg);
}

void Listener::handlePresence( const Presence& presence ) {}

void Listener::handleLog( gloox::LogLevel level, LogArea area, const std::string& message )
{
	std::cout << "Xmpp Log Level: " << level << ", area: " << area << " : " << message.c_str() << std::endl;
}

bool Listener::handleSubscriptionRequest( const gloox::JID& jid, const std::string& msg )
{
	// subscribe to the subscriber...
	gloox::Subscription s( Subscription::Subscribe, jid );
	mClient->send(s);
	
	return true;
}

bool Listener::handleUnsubscriptionRequest( const gloox::JID& jid, const std::string& msg )
{
	return true;
}

void Listener::handleRegistrationFields( const gloox::JID& from, int fields, std::string instructions )
{
	gloox::RegistrationFields values;
	values.password = mUsername;
	values.username = mUsername;
	mRegistration->createAccount(fields, values);
}

void Listener::handleAlreadyRegistered( const gloox::JID& from )
{
	std::cout << "  Listener::handleAlreadyRegistered " << std::endl;
}

void Listener::handleRegistrationResult( const gloox::JID& from, gloox::RegistrationResult regResult )
{
	if (gloox::RegistrationSuccess == regResult) {
		mRegistration->removeRegistrationHandler();
		//delete mRegistration;
		if (mDelegate) mDelegate->onRegistration(regResult);
	}
}


bool Listener::sendMessage( const JID& recipient, const std::string& message, const std::string& subject )
{
	if (!mIsConnected || message.empty() || !mClient) return false;
	
	Message m(Message::Chat, recipient, message, subject);
	mClient->send(m);
	
	return true;
}
	
bool Listener::subscribeTo( const JID& recipient )
{
	if (!mIsConnected || !mClient) return false;
	
	gloox::Subscription s( Subscription::Subscribe, recipient );
	mClient->send(s);
	
	return true;
}

}