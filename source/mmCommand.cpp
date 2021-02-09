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

#include "mmCommand.h"

// create the command store
std::map<std::string, std::pair<Command::Creator, std::string>> Command::_cmdCreators;

//
bool Command::addCreator(const std::string& cmdName, const std::string& cmdBrief, Command::Creator cmdCreator) {
	if (Command::_cmdCreators.find(cmdName) != Command::_cmdCreators.end()) {
		std::cout << "Error: cannot register new commmand " << cmdName << " already exists" << std::endl;
		return false;
	}
	_cmdCreators[cmdName] = make_pair(cmdCreator, cmdBrief);
	return true;
}

//
Command* Command::create(const std::string& app, const std::string& cmd) {
	std::map<std::string, std::pair<Command::Creator, std::string>>::iterator it = Command::_cmdCreators.find(cmd);
	if (it == Command::_cmdCreators.end()) {
		std::cout << "Error: unknown command " << cmd << std::endl;
		return NULL;
	}
	//
	return it->second.first();
}

//
void Command::logCommands(void) {
	std::map<std::string, std::pair<Command::Creator, std::string>>::iterator it;
	for (it = Command::_cmdCreators.begin(); it != _cmdCreators.end(); ++it) {
		std::cout << "  " << it->first << "\t" << it->second.second << std::endl;
	}
}