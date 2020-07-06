//
//  Logger.h
//  EscPlatform
//
//  Created by Caleb Johnston on 3/14/14.
//  Copyright (c) 2014 com.esc. All rights reserved.
//

#pragma once

#include <iostream>

namespace esc {
namespace io {
	typedef enum level_t {
		ERROR    = 1,
		WARNING  = 2,
		INFO     = 4,
		DEV      = 8,
		LOG      = 16
	} LogLevel;
	
	const int LEVEL_FLOOR = ERROR;
	const int ALL = 255;
	const int DEFAULT_REPORTING_LEVEL = 4;
	
	bool canLog(const LogLevel compare);

	void setLevel(const LogLevel level);

	LogLevel getLevel();

	void enable(bool toggle);

	bool isEnabled();

	bool log(const LogLevel level, const std::string& message);

	bool info(const std::string& message);

	bool dev(const std::string& message);

	bool warning(const std::string& message);

	bool error(const std::string& message);
	
};	// namespace io
};	// namespace esc