#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"

extern std::string sokket::config::port {"27015"};
extern std::string sokket::config::address {"localhost"};
extern int sokket::config::bufferSize {512};

int main(int argc, char* argv[])
{
	std::string test {"sup mate"};
	bool isClient {true};
	SOCKET sokket {INVALID_SOCKET};
	/*if (argc != 4)
	{
		std::cout << "Usage: sokket [ip address] [port number] [c for client, s for server]\n";
		return -1;
	}
	else
	{
		sokket::config::port = argv[2];
		sokket::config::address = argv[3];
	} */

	if (isClient)
	{
		int iResult;
		sokket = sokket::client::setupSocket();

		iResult = send(sokket, test.c_str(), test.size(), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send failed with error: " << WSAGetLastError() << ".\n";
			closesocket(sokket);
			WSACleanup();
			return 1;
		}

		iResult = shutdown(sokket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "shutdown failed with error: " << WSAGetLastError() << ".\n";
			closesocket(sokket);
			WSACleanup();
			return 1;
		}

		closesocket(sokket);
		WSACleanup();
	}
	else
	{
		sokket = sokket::server::setupSocket();
	}
	return 0;
}
