//
//  Logger.cpp
//  EscPlatform
//
//  Created by Caleb Johnston on 3/14/14.
//  Copyright (c) 2014 com.esc. All rights reserved.
//

#if defined( __OBJC__ )
#import <Foundation/Foundation.h>
#endif

#include "Logger.h"

namespace esc {
namespace io {

static bool sEnabled = true;
static LogLevel sActiveLevel = (LogLevel) DEFAULT_REPORTING_LEVEL;

bool canLog(const LogLevel compare )
{
	if ((sActiveLevel > LOG) || (compare > LOG)) {
		return false;
	}
	
	return sActiveLevel >= compare;
}

void setLevel(const LogLevel level)
{
	sActiveLevel = level;
}

LogLevel getLevel()
{
	return sActiveLevel;
}

void enable(bool toggle)
{
	sEnabled = toggle;
}

bool isEnabled()
{
	return sEnabled;
}

bool log(const LogLevel level, const std::string& message)
{
	if (isEnabled() && canLog(level)) {
		NSString* msg = [NSString stringWithUTF8String:message.c_str()];
		NSLog(msg, @"@");
		
		return true;
	}
	
	return false;
}

bool info(const std::string& message)
{
	return log(INFO, message);
}

bool dev(const std::string& message)
{
	return log(DEV, message);
}

bool warning(const std::string& message)
{
	return log(WARNING, message);
}

bool error(const std::string& message)
{
	return log(ERROR, message);
}

}	// namespace io
}	// namespace esc