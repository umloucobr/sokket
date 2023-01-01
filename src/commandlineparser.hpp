#pragma once
#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP
#include "sokket.hpp"

namespace sokket {
	namespace clparser {
		namespace config {
			extern std::string quitCombination;
			extern std::string fileMode;
			extern std::string messageMode;
			extern std::atomic<bool> fileModeBool;
		}
		
		int init(SOCKET& _sokket, std::string& receivedInformation);
		void readConsole(SOCKET& _sokket, std::atomic<bool>& stopProgram, bool& errorCode, std::atomic<bool>& fileModeBool);
	}
}

#endif //COMMANDLINEPARSER_HPP
