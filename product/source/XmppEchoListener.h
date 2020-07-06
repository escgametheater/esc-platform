//
//  PluginInterface.cpp
//  ESC-Unity-Plugin
//
//  Created by Caleb Johnston on 1/27/14.
//  Copyright (c) 2014 Control Group. All rights reserved.
//

#pragma once

#include <iostream>
#include <string>

#include "XmppListener.h"

namespace xmpp {

class Client;

class EchoListener : public xmpp::Listener
{
public:
	EchoListener(xmpp::Client* delegate = 0);
	virtual ~EchoListener();
	
protected:
	virtual void handleMessage( const gloox::Message& msg, gloox::MessageSession* session );
	
	long int mEchoedMessages;

};

}	/* end namespace xmpp */
