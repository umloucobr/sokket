#pragma once
#ifndef COMMANDLINEPARSER_HPP
#define COMMANDLINEPARSER_HPP
#include "sokket.hpp"

namespace sokket {
	namespace clparser {
		namespace config {
			extern std::string quitCombination;
			extern std::string fileMode;
			extern std::atomic<bool> fileModeBool;
		}
		
		int init(SOCKET& _sokket);
		void readConsole(SOCKET& _sokket, std::atomic<bool>& stopProgram, bool& errorCode);
	}
}

#endif //COMMANDLINEPARSER_HPP
