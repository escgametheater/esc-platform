
#include "AccountRegistration.h"

using namespace gloox;

namespace xmpp {

AccountRegistration::AccountRegistration()
	: mStatus( INCOMPLETE ), mClient(0), mRegistration(0)
{
}

AccountRegistration::~AccountRegistration()
{
	delete mClient;
	delete mRegistration;
}

bool AccountRegistration::init( const std::string& username, const std::string& password,
								const std::string& host, const int port, const bool enableLogging )
{
	if (username.empty() || password.empty() || host.empty()) {
		return false;
	}
	
	mUsername = username;
	mPassword = password;
	mHostname = host;
	
	mStatus = INCOMPLETE;
	
	mClient = new gloox::Client( mHostname );
	mClient->setPort( port );
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

void AccountRegistration::onConnect()
{
	if (!mRegistration) mClient->disconnect();
		
	mRegistration->fetchRegistrationFields();
}

void AccountRegistration::handleLog( gloox::LogLevel level, gloox::LogArea area, const std::string& message )
{
	std::cout << "Xmpp Log Level: " << level << ", area: " << area << " : " << message.c_str() << std::endl;
}

void AccountRegistration::handleRegistrationFields( const gloox::JID& from, int fields, std::string instructions )
{
	if (!mRegistration) mClient->disconnect();
	
	gloox::RegistrationFields values;
	values.password = mPassword;
	values.username = mUsername;
	mRegistration->createAccount(fields, values);
}

void AccountRegistration::handleAlreadyRegistered( const gloox::JID& from )
{
	mStatus = FAILURE;
}
	
void AccountRegistration::handleRegistrationResult( const gloox::JID& from, gloox::RegistrationResult regResult )
{
	if (gloox::RegistrationSuccess == regResult) {
		mStatus = SUCCESS;
		mRegistration->removeRegistrationHandler();
	}
	else mStatus = FAILURE;
}

}