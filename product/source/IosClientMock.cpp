#include <sstream>
#include <chrono>
#include <random>

#include "IosClientMock.h"
#include "OscMessage.h"

using namespace gloox;
using namespace std;

const int MINIMUM_MESSAGE_DELAY = 5; // in milliseconds

IosClientMock::IosClientMock() : mRunning(false)
{
	
}

IosClientMock::~IosClientMock()
{
	mRunning = false;
	
	if (mOscThread && mOscThread->joinable()) {
		mOscThread->join();
	}
	if (mXmppThread && mXmppThread->joinable()) {
		mXmppThread->join();
	}
}

void IosClientMock::start( const Configuration config )
{
	mConfig = config;
	
	using namespace std::placeholders;
	
	mClients.clear();
	
	std::string name_prefix = "mock-client-test-account-";
	while (mClients.size() < mConfig.totalClients) {
		xmpp::Client client;
		std::string testAccountName = name_prefix + to_string(mClients.size());
		client.setSignalConnect( std::bind(&IosClientMock::onConnect, this) );
		client.setSignalTlsConnect( std::bind(&IosClientMock::onTlsConnect, this) );
		client.setSignalDisconnect( std::bind(&IosClientMock::onDisconnect, this, _1) );
		client.setSignalMessage( std::bind(&IosClientMock::handleXmppMessage, this, _1, _2, _3) );
		client.setSignalRoster( std::bind(&IosClientMock::handleXmppRoster, this, _1) );
		client.setSignalRosterPresence( std::bind(&IosClientMock::handleXmppRosterPresence, this, _1, _2, _3, _4) );
		client.connect(testAccountName, testAccountName, mConfig.server);
		
		mClients.push_back(client);
	}
	
	mOscSender.setup(mConfig.server, mConfig.oscPort);
	
	mRunning = true;
	
	mOscThread = std::make_shared<std::thread>( std::bind(&IosClientMock::spawnOscThread, this) );
	mXmppThread = std::make_shared<std::thread>( std::bind(&IosClientMock::spawnXmppThread, this) );
}

void IosClientMock::onConnect()
{
	
}

void IosClientMock::onTlsConnect()
{
	
}

void IosClientMock::onDisconnect( const xmpp::error_t& error )
{
	
}

void IosClientMock::handleXmppMessage( const xmpp::Peer& peer, const std::string& body, const std::string& subject )
{
	
}

void IosClientMock::handleXmppRoster( const std::list<std::string>& roster )
{
	
}

void IosClientMock::handleXmppRosterPresence( const xmpp::Peer& peer, const std::string& resource, xmpp::status_t status, const std::string& message )
{
	
}

//void IosClientMock::handleOscMessage(const osc::Message* msg)
//{
//
//}

void IosClientMock::spawnOscThread()
{
	// setup pseudo-random number generator
	std::random_device rd;
	std::mt19937 engine(rd());
	std::normal_distribution<> engine_distr(-3.0f, 3.0f);
	int delay = static_cast<int>( 1000 / mConfig.oscFrequency / mConfig.totalClients );
	if (0 >= delay) delay = MINIMUM_MESSAGE_DELAY;
	
	std::cout << "  message delay set to: " << delay << std::endl;
	
	int client_index = 0;
	while (mRunning) {
		osc::Message msg;
		msg.setAddress("/x-accel/y-accel/z-accel/x-gyro/y-gyro/z-gyro/something/else");
		msg.addIntArg(client_index);
		client_index++;
		if (client_index >= mTotalClients) {
			client_index = 0;
		}
		msg.addFloatArg(engine_distr(engine));
		msg.addFloatArg(engine_distr(engine));
		msg.addFloatArg(engine_distr(engine));
		msg.addFloatArg(engine_distr(engine));
		msg.addFloatArg(engine_distr(engine));
		msg.addFloatArg(engine_distr(engine));
		msg.addStringArg("test");
		msg.addIntArg(client_index | mTotalClients);
		mOscSender.sendMessage(msg);
		
		std::this_thread::sleep_for(std::chrono::milliseconds( delay ));
	}
}

void IosClientMock::spawnXmppThread()
{
	// setup pseudo-random number generator
	//std::random_device rd;
	//std::mt19937 engine(rd());
	//std::normal_distribution<> engine_distr(-999999999, 999999999);
	
	int delay = static_cast<int>( 1000 / mConfig.xmppFrequency / mConfig.totalClients );
	if (0 >= delay) delay = MINIMUM_MESSAGE_DELAY;
	
	std::cout << "  message delay set to: " << delay << std::endl;
	
	int client_index = 0;
	while (mRunning) {
		std::string msg("testMessage:key0=valueA,key1=valueB,key2=valueC,key3=valueD,key4=valueE,key5=valueF");
		mClients.at(client_index).sendMessage(mConfig.server, msg);
		
		client_index++;
		if (client_index >= mTotalClients) {
			client_index = 0;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds( delay ));
	}
}