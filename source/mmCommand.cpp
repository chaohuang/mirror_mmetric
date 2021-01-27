// ************* COPYRIGHT AND CONFIDENTIALITY INFORMATION *********
// Copyright © 20XX InterDigital All Rights Reserved
// This program contains proprietary information which is a trade secret/business
// secret of InterDigital R&D france is protected, even if unpublished, under 
// applicable Copyright laws (including French droit d’auteur) and/or may be 
// subject to one or more patent(s).
// Recipient is to retain this program in confidence and is not permitted to use 
// or make copies thereof other than as permitted in a written agreement with 
// InterDigital unless otherwise expressly allowed by applicable laws or by 
// InterDigital under express agreement.
//
// Author: jean-eudes.marvie@interdigital.com
// *****************************************************************

#include "mmCommand.h"

// create the command store
std::map<std::string, Command*> Command::_commands;


bool Command::addCommand(Command* cmd) {
	if (Command::_commands.find(cmd->name()) != Command::_commands.end()) {
		std::cout << "Error: cannot register new commmand " << cmd->name() << " already exists" << std::endl;
		return false;
	}
	_commands[cmd->name()] = cmd;
	return true;
}

int Command::execute(std::string app, std::string cmd, int argc, char* argv[]) {
	std::map<std::string, Command*>::iterator it = Command::_commands.find(cmd);
	if (it == Command::_commands.end()) {
		std::cout << "Error: unknown command " << cmd << std::endl;
		return 1;
	}
	//
	return it->second->main(app, argc, argv);
}

void Command::logCommands(void) {
	std::map<std::string, Command*>::iterator it;
	for (it = Command::_commands.begin(); it != _commands.end(); ++it) {
		std::cout << "  " << it->second->name() << "\t" << it->second->brief() << std::endl;
	}
}