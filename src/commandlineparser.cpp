#include "commandlineparser.hpp"

extern std::string sokket::clparser::config::quitCombination {":q"};

//May not be thread safe for some time, sorry.
void sokket::clparser::readConsole(SOCKET& _sokket, std::atomic<bool>& stopProgram, bool& errorCode) {
	std::wstring wideInput{};

	while (!stopProgram.load()) {
		std::cout << "> ";
		std::getline(std::wcin, wideInput);

		std::string input{input.begin(), input.end()};

		if (input == sokket::clparser::config::quitCombination) {
			errorCode = false;
			stopProgram.store(true);

			int result{ sokket::sendSocket(_sokket, sokket::clparser::config::quitCombination, 2) };

			if (result != 0) {
				errorCode = true;
			}

			sokket::shutdownSocket(_sokket);
		}
		else {
			int result{};
			
			if (!stopProgram.load()) {
				result = { sokket::sendSocket(_sokket, input, input.size()) };
			}

			if (result != 0) {
				errorCode = true;
				stopProgram.store(true);
			}
		}
	}
}

int sokket::clparser::init(SOCKET& _sokket, std::string& receivedInformation) {
	bool disconnect {false}; //If the other user disconnected, disconnect will be true.
	int result {0}; //Result from sokket::receiveSocket.

	bool errorCode {false}; //Error code for the sokket::clparser::readConsole thread. May need to be a std::atomic<bool> in the future.
	std::atomic<bool> stopProgram {false};
	std::thread readConsoleThread(sokket::clparser::readConsole, std::ref(_sokket), std::ref(stopProgram), std::ref(errorCode));

	while (!stopProgram.load()) {
		result = sokket::receiveSocket(_sokket, receivedInformation, disconnect);
		if (result != 0) {
			errorCode = true;
			stopProgram.store(true);
		}
		if (disconnect) {
			errorCode = false;
			stopProgram.store(true);
			sokket::shutdownSocket(_sokket);
		}
	}

	std::cout << "Disconnected.";
	stopProgram.store(true);
	readConsoleThread.join();
	
	if (!errorCode) {
		return 0;
	}
	else {
		return 1;
	}
}