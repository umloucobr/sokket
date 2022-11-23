#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"

extern std::string sokket::config::port {"27015"};
extern std::string sokket::config::address {"localhost"};
extern const int sokket::config::bufferSize {512};

int sokket::sendSocket(SOCKET& _sokket, std::string& sendBuffer, int sendBufferSize) {
	int iResult {};

	if (sendBuffer.size() > sokket::config::bufferSize)
	{
		return 1;
	}

	iResult = send(_sokket, sendBuffer.c_str(), static_cast<int>(sendBuffer.size())+ 1, 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "send failed with error: " << WSAGetLastError() << ".\n";
		closesocket(_sokket);
		WSACleanup();
		return 1;
	}
	return 0;
}

int sokket::receiveSocket(SOCKET& _sokket, std::string& receiveBufferString) {
	char exit{};
	bool connectionEnded {false};
	char receiveBuffer[sokket::config::bufferSize];
	int iResult;

	do {
		iResult = recv(_sokket, receiveBuffer, sokket::config::bufferSize, 0);
		if (iResult > 0) {
			connectionEnded = false;

			if (receiveBuffer[iResult - 1] == '\n') {
				receiveBuffer[iResult - 1] = '\0';
			}
			receiveBuffer[iResult - 1] = '\0';

			std::cout << receiveBuffer << '\n';

			receiveBufferString = receiveBuffer;
		}

		//Temporary because client closes automatically.
		else if (iResult == 0 && !connectionEnded) {;
			std::cout << std::endl;
			connectionEnded = true;
		}

		else if (iResult < 0) {
			std::cerr << "recv failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0 || iResult == 0);

	return 0;
}

int sokket::shutdownSocket(SOCKET& _sokket) {
	int iResult;

	iResult = shutdown(_sokket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "shutdown failed with error: " << WSAGetLastError() << ".\n";
		closesocket(_sokket);
		WSACleanup();
		return 1;
	}

	closesocket(_sokket);
	WSACleanup();

	return 0;
}

int main(int argc, char* argv[]) {
	//512 a.
	std::string test {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
	std::string test2 {"sup mate"};
	std::string receiveBufferString{};
	receiveBufferString.reserve(sokket::config::bufferSize);
	bool isClient {false};
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
		sokket = sokket::client::setupSocket();

		sokket::sendSocket(sokket, test, test.size());
		sokket::sendSocket(sokket, test2, test.size());

		sokket::shutdownSocket(sokket);
	}
	else
	{
		sokket = sokket::server::setupSocket();
		sokket::receiveSocket(sokket, receiveBufferString);
		sokket::shutdownSocket(sokket);
	}

	return 0;
}
