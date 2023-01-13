#include "commandlineparser.hpp"

std::string sokket::clparser::config::quitCombination {":q"};
std::string sokket::clparser::config::fileMode {":f"};
std::string sokket::clparser::config::messageMode {":m"};
std::atomic<bool> sokket::clparser::config::fileModeBool {false};

//May not be thread safe for some time, sorry. Some mutexes would help.
void sokket::clparser::readConsole(SOCKET& _sokket, std::atomic<bool>& stopProgram, bool& errorCode, std::atomic<bool>& fileModeBool) {
	std::wstring wideInput{};

	while (!stopProgram.load()) {
		std::cout << "> ";
		std::getline(std::wcin, wideInput);

		std::string input{wideInput.begin(), wideInput.end()};

		if (input == sokket::clparser::config::quitCombination) {
			errorCode = false;
			stopProgram.store(true);

			int result {sokket::sendSocket(_sokket, sokket::clparser::config::quitCombination, sokket::clparser::config::quitCombination.size())};
			if (result != 0) {
				errorCode = true;
			}

			sokket::shutdownSocket(_sokket);
		}
		else if (input == sokket::clparser::config::fileMode) {
			fileModeBool.store(true);
			std::cout << "Entering file mode, send a file with \"Name of the file\" \"Path of the file\"\n";

			int result {sokket::sendSocket(_sokket, sokket::clparser::config::fileMode, sokket::clparser::config::fileMode.size())};
			if (result != 0) {
				errorCode = true;
			}
		}
		else if (input == sokket::clparser::config::messageMode) {
			fileModeBool.store(false);

			int result{sokket::sendSocket(_sokket, sokket::clparser::config::messageMode, sokket::clparser::config::messageMode.size())};
			if (result != 0) {
				errorCode = true;
			}
		}
		else {
			int result{};
			
			if (!stopProgram.load() && !fileModeBool.load()) {
				result = sokket::sendSocket(_sokket, input, input.size());
			}
			else if (!stopProgram.load() && fileModeBool.load())
			{
				result = sokket::sendSocketFile(_sokket, input);
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
	std::thread readConsoleThread(sokket::clparser::readConsole, std::ref(_sokket), std::ref(stopProgram), std::ref(errorCode), std::ref(sokket::clparser::config::fileModeBool));

	while (!stopProgram.load()) {
		result = sokket::receiveSocket(_sokket, receivedInformation, disconnect, sokket::clparser::config::fileModeBool);
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