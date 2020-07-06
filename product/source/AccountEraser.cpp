
#include "AccountEraser.h"

#define DEBUG 1

using namespace gloox;

namespace xmpp {

AccountEraser::AccountEraser()
: mStatus( INCOMPLETE ), mClient(0), mRegistration(0)
{
}

AccountEraser::~AccountEraser()
{
	delete mClient;
	delete mRegistration;
}

bool AccountEraser::init( const std::string& username, const std::string& password,
							   const std::string& host, const int port, const bool enableLogging )
{
	if (username.empty() || password.empty() || host.empty()) {
		return false;
	}
	
	mStatus = INCOMPLETE;
	
	mClient = new gloox::Client( host );
	mClient->disableRoster();
	mClient->setPort( port );
	mClient->setUsername(username);
	mClient->setPassword(password);
	mClient->registerConnectionListener( this );
	mClient->disco()->setVersion( "messageTest", gloox::GLOOX_VERSION, "Linux" );
	mClient->disco()->setIdentity( "client", "bot" );
	mClient->disco()->addFeature( gloox::XMLNS_REGISTER );
	mClient->disco()->addFeature( gloox::XMLNS_STREAM_IQREGISTER );
	
	mRegistration = new gloox::Registration( mClient );
	mRegistration->registerRegistrationHandler( this );
	
#ifdef DEBUG
	if (enableLogging)
		mClient->logInstance().registerLogHandler( gloox::LogLevelDebug, gloox::LogAreaAll, this );
#endif
	
	// make connection to initiate account creation...
	if ( mClient->connect( false ) ) {
		gloox::ConnectionError ce = gloox::ConnNoError;
		while ( ce == gloox::ConnNoError && mClient ) {
			ce = mClient->recv();
			if (mStatus == SUCCESS || mStatus == FAILURE) {
				break;
			}
		}
	}
	
	return true;
}

void AccountEraser::onConnect()
{
	if (!mRegistration) mClient->disconnect();
	
	mRegistration->removeAccount();
}

void AccountEraser::handleLog( gloox::LogLevel level, gloox::LogArea area, const std::string& message )
{
	std::cout << "Xmpp Log Level: " << level << ", area: " << area << " : " << message.c_str() << std::endl;
}

void AccountEraser::handleAlreadyRegistered( const gloox::JID& from )
{
	mStatus = FAILURE;
}

void AccountEraser::handleRegistrationResult( const gloox::JID& from, gloox::RegistrationResult regResult )
{
	if (gloox::RegistrationSuccess == regResult) {
		mStatus = SUCCESS;
		mRegistration->removeRegistrationHandler();
	}
	else mStatus = FAILURE;
}

}