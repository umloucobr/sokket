#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"
#include "commandlineparser.hpp"

extern std::string sokket::config::port {"27015"};
extern std::string sokket::config::address {"localhost"};

int main(int argc, char* argv[]) {
#ifdef _WIN32
	_setmode(_fileno(stdin), _O_WTEXT); //Correctly get UTF-8 characters with this obscure function.
#endif // _WIN32

	bool isClient {true};
	bool autoMode {true}; //If this mode is set to true (default), if the client can't connect to a server he will become the server.
	SOCKET sokket {INVALID_SOCKET};

	if (argc == 4) {
		std::vector<std::string> arguments(argv + 1, argv + argc); //This does NOT store the name of the program as the first index.

		if (arguments[0] == "-a") {
			autoMode = true;
		}
		else if (arguments[0] == "-c") {
			isClient = true;
			autoMode = false;
		}
		else if (arguments[0] == "-s") {
			isClient = false;
			autoMode = false;
		}
		else {
			std::cout << "How to use: sokket [-a for automatic mode, -c for client and -s for server] [port] [address]\n";
			return 1;
		}
		sokket::config::port = arguments[1];
		sokket::config::address = arguments[2];

	}
	else if (argc == 2) {
		std::vector<std::string> arguments(argv + 1, argv + argc); //This does NOT store the name of the program as the first index.

		if (arguments[0] == "-a") {
			autoMode = true;
		}
		else if (arguments[0] == "-c") {
			isClient = true;
			autoMode = false;
		}
		else if (arguments[0] == "-s") {
			isClient = false;
			autoMode = false;
		}
		else {
			std::cout << "How to use: sokket [-a for automatic mode, -c for client and -s for server] [port] [address]\n";
			return 1;
		}
	}
	else if (argc == 1) {
		autoMode = true;
	}
	else {
		std::cout << "How to use: sokket [-a for automatic mode, -c for client and -s for server] [port] [address]\n";
		return 1;
	}

clientOrServer: //Yes, it is a goto. That is the best way to do this, for now.
	if (!isClient) {
		std::cout << "Entering server mode.\n";
		std::cout << "Port: " << sokket::config::port << '\n';
		std::cout << "Address: " << sokket::config::address << '\n';
		std::cout << "Connecting...\n";

		int result {sokket::server::setupSocket(sokket)}; //sokket::shutdownSocket is inside clparser. Sorry.
		if (result == 1){
			return 1;
		}

		std::cout << "Connected.\n";

		int result2 {sokket::clparser::init(sokket)}; //sokket::clparser::init returns 0 if there were no errors, so you can shutdown correctly.
		if (result2 == 1) {
			return 1;
		}
	}
	else {
		std::cout << "Entering client mode.\n";
		std::cout << "Port: " << sokket::config::port << '\n';
		std::cout << "Address: " << sokket::config::address << '\n';
		std::cout << "Connecting...\n";

		int result {sokket::client::setupSocket(sokket, autoMode)}; //sokket::shutdownSocket is inside clparser. Sorry.
		if (result == 1 && !autoMode) {
			return 1;
		}
		else if (result == 1 && autoMode) {
			std::cout << std::endl;
			isClient = false;
			goto clientOrServer;
		}

		std::cout << "Connected.\n";

		int result2 {sokket::clparser::init(sokket)}; //sokket::clparser::init returns 0 if there were no errors, so you can shutdown correctly.
		if (result2 == 1) {
			return 1;
		}
	}

	return 0;
}