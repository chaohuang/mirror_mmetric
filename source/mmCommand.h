// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright 2021 - InterDigital
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissionsand
// limitations under the License.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

#ifndef _MM_COMMAND_H_
#define _MM_COMMAND_H_

#include <string>
#include <map>
#include <iostream>
#include <sstream> 

// mathematics
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

//
#include "mmContext.h"

class Command {

public: // Command API, to be specialized by command implementation

	// must be overloaded to parse arguments and init the command
	virtual bool initialize(Context*, std::string app, int argc, char* argv[])=0;

	// must be overloaded to execute command for given frame
	virtual bool process(uint32_t frame) = 0;

	// must be overloaded to collect temporal results after all frames processing
	virtual bool finalize( void ) = 0;

public: // Command managment API

	// command creator function type
	typedef Command* (*Creator)(void);

	// invoke to register a new command 
	static bool addCreator(const std::string& cmdName, const std::string& cmdBrief, Creator cmdCreator);

	// print the list of registered commands
	static void logCommands(void);

	// invoke to create a new command
	static Command* create(const std::string& app, const std::string& cmd);

private:

	// command name -> (command creator, brief description)
	static std::map<std::string, std::pair<Command::Creator, std::string>> _cmdCreators;
	
};

//
inline bool parseVec2(const std::string& s, glm::vec2& res) {
	glm::vec2 tmp;
	std::istringstream stream(s);
	stream.exceptions(std::istringstream::failbit | std::istringstream::badbit);
	try {
		stream >> tmp.x;
		stream >> tmp.y;
		res = tmp;
		return true;
	}
	catch (std::istringstream::failure) {
		return false;
	}
}

inline bool parseVec3(const std::string& s, glm::vec3& res) {
	glm::vec3 tmp;
	std::istringstream stream(s);
	stream.exceptions(std::istringstream::failbit | std::istringstream::badbit);
	try {
		stream >> tmp.x;
		stream >> tmp.y;
		stream >> tmp.z;
		res = tmp;
		return true;
	}
	catch (std::istringstream::failure) {
		return false;
	}
}

inline bool parseVec4(const std::string& s, glm::vec4& res) {
	glm::vec4 tmp;
	std::istringstream stream(s);
	stream.exceptions(std::istringstream::failbit | std::istringstream::badbit);
	try {
		stream >> tmp.x;
		stream >> tmp.y;
		stream >> tmp.z;
		stream >> tmp.w;
		res = tmp;
		return true;
	}
	catch (std::istringstream::failure) {
		return false;
	}
}

#endif
