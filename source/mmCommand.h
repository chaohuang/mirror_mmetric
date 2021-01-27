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

#ifndef _MM_COMMAND_H_
#define _MM_COMMAND_H_

#include <string>
#include <map>
#include <iostream>

class Command {

public:

	// must be overloaded to return the command name
	virtual const char* name(void) = 0;

	// must be overloaded to return the command brief description
	virtual const char* brief( void ) = 0;

	// must be overloaded to execute the command
	virtual int main(std::string app, int argc, char* argv[])=0;

	// invoke to register a new command 
	static bool addCommand(Command* cmd);

	// invoke to execute a command
	static int execute(std::string app, std::string cmd, int argc, char* argv[]);

	// print the list of commands
	static void logCommands(void);

private:

	// command name -> command
	static std::map<std::string, Command*> _commands;
	
};

#endif
