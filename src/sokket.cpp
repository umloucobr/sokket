#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"

extern std::string sokket::config::port {"27015"};
extern std::string sokket::config::address {"localhost"};
extern const int sokket::config::bufferSize {512};

int sokket::sendSocket(SOCKET& _sokket, std::string& sendBuffer, std::uint64_t sendBufferSize) {
	//std:uint64_t sendBufferSizeHtonll {htonll(sendBufferSize)};
	int iResult {0}; 
	std::uint64_t bytesRemaining {sendBufferSize}; //Used on std::min, making a copy because I need to decrease it.
	std::uint64_t bytesSent {0};
	std::uint64_t sendSocketSize {std::min(bytesRemaining, static_cast<std::uint64_t>(sokket::config::bufferSize))};

	/*Send size first.
	iResult = send(_sokket, reinterpret_cast<char*>(&sendBufferSizeHtonll), sizeof(sendBufferSizeHtonll), 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "send size failed with error: " << WSAGetLastError() << ".\n";
		closesocket(_sokket);
		WSACleanup();
		return 1;
	}*/

	std::string buffer {};
	buffer.assign(sendBuffer, 0, sendSocketSize); //\0;

	while (bytesSent < sendBufferSize) {
		sendSocketSize = std::min(bytesRemaining, static_cast<std::uint64_t>(sokket::config::bufferSize - 1));
		buffer.assign(sendBuffer, bytesSent, sendSocketSize);

		//If it's the last packet, add space for a null terminator. 
		if (sendSocketSize < sokket::config::bufferSize - 1)
		{
			sendSocketSize += 1;
		}

		iResult = send(_sokket, buffer.c_str(), sendSocketSize, 0); //\0.
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanup();
			return 1;
		}

		bytesSent += sendSocketSize;
		bytesRemaining -= sendSocketSize;
	}

	if (bytesSent != sendBufferSize)
	{
		std::cout << "Erorr! Sent " << bytesSent << " but total number of bytes is " << sendBufferSize << ".\n";
	}

	return 0;
}

int sokket::receiveSocket(SOCKET& _sokket, std::string& receivedInformation) {
	bool connectionEnded {false};

	char receiveBuffer[sokket::config::bufferSize];

	std::uint64_t packageSize {};
	int iResult {};
	int iResultSize {};

	do {
		/*
		iResultSize = recv(_sokket, reinterpret_cast<char*>(ntohll(packageSize)), sokket::config::bufferSize, 0);
		if (iResult > 0) {
			std::cout << "sçdk";
			std::cout << packageSize;
		}

		else if (iResult < 0) {
			std::cerr << "size recv failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanup();
			return 1;
		} */

		iResult = recv(_sokket, receiveBuffer, sokket::config::bufferSize, 0);
		if (iResult > 0) {
			connectionEnded = false;

			if (receiveBuffer[iResult - 1] == '\n') {
				receiveBuffer[iResult - 1] = '\0';
			}

			receiveBuffer[iResult - 1] = '\0';

			std::cout << receiveBuffer << '\n';

			receivedInformation.append(receiveBuffer);
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
	std::string test {"asdfhjasdlçfhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhfsdfsdfsdfsdfsdfddddddhhhhhhhhhhhhhhhhhhhhhhhhhjsaçfhasldujgkldhujaslifuhnjasioljdufioasujfhoasfasuihbgfiasuyhfgiasfgasiufgyasuifyhgasufyhgasiufdyhgasfsdfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffdd"};
	std::string test2 {"hate it or love the underdogs are on top"};
	std::string receiveBufferString{};
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

	if (isClient) {
		sokket = sokket::client::setupSocket();

		std::uint64_t size {test.size()};
		sokket::sendSocket(sokket, test, size);
		sokket::shutdownSocket(sokket);
	}
	else {
		sokket = sokket::server::setupSocket();
		sokket::receiveSocket(sokket, receiveBufferString);
		sokket::shutdownSocket(sokket);
	}

	return 0;
}
