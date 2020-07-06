
#include "XmppEchoListener.h"

using namespace gloox;

namespace xmpp {

EchoListener::EchoListener(xmpp::Client* delegate) : Listener(delegate), mEchoedMessages(0)
{
}

EchoListener::~EchoListener()
{
}

void EchoListener::handleMessage( const Message& msg, MessageSession* session )
{
	Listener::handleMessage( msg, session );
	
	std::cout << "  " << ++mEchoedMessages << " sending " << msg.body() << " to " << msg.from().username() << "@" << msg.from().server() << std::endl;
	
	Message m(msg.subtype(), msg.from(), msg.body(), msg.subject());
	this->getClient().send(m);
}

}