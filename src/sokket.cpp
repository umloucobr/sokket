#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"

extern std::string sokket::config::port {"27015"};
extern std::string sokket::config::address {"localhost"};
extern const int sokket::config::bufferSize {4};

int sokket::sendSocket(SOCKET& _sokket, std::string& sendBuffer, std::uint64_t sendBufferSize) {
	int iResult {0}; 
	std::uint64_t bytesRemaining {sendBufferSize}; //Used on std::min, making a copy because I need to decrease it.
	std::uint64_t bytesSent {0};
	std::uint64_t sendSocketSize {0}; //Size of the packet.
	std::string buffer{};
	std::string sendBufferSizeString {std::to_string(sendBufferSize)}; //Transform int in a std::string to send size over packet. Yes, it's trash but the send() function isn't accepting int.

	//Send size first.
	iResult = send(_sokket, sendBufferSizeString.c_str(), sendBufferSizeString.size(), 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "send size failed with error: " << WSAGetLastError() << ".\n";
		closesocket(_sokket);
		WSACleanup();
		return 1;
	}
	
	//Send contents.
	while (bytesSent < sendBufferSize) {
		sendSocketSize = std::min(bytesRemaining, static_cast<std::uint64_t>(sokket::config::bufferSize));
		buffer.assign(sendBuffer, bytesSent, sendSocketSize);
		std::cout << buffer;
		std::cout << buffer.size();

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
	char receiveBuffer[sokket::config::bufferSize + 1];	
	char quit {};
	int iResult {};
	int iResultSize {};
	std::uint64_t contentsSize {};
	std::uint64_t bytesReceived {};
	bool connectionEnded {false};
	bool receiveSize {true}; //The first packet is the size of the contents.

	do {
		iResult = recv(_sokket, receiveBuffer, sokket::config::bufferSize + 1, 0);
		if (iResult > 0) {
			if (receiveSize) {
				receiveBuffer[iResult] = '\0';

				receiveSize = false;
				contentsSize = atoi(receiveBuffer);
			}
			else {
				connectionEnded = false;

				if (receiveBuffer[iResult - 1] == '\n') {
					receiveBuffer[iResult - 1] = '\0';
				}

				receiveBuffer[iResult] = '\0';

				bytesReceived += iResult;
				receivedInformation.append(receiveBuffer);
			}
		}

		//Temporary because client closes automatically.
		else if (iResult == 0 || !connectionEnded) {
			if (bytesReceived != contentsSize) {
				std::cout << "Error: Received " << bytesReceived << " bytes but " << contentsSize << " bytes were sent.\n";
			}

			std::cout << bytesReceived << '\n' << contentsSize << '\n';
			connectionEnded = true;
			receiveSize = true;

			std::cout << receivedInformation;
			std::cin >> quit;
		}

		else if (iResult < 0) {
			std::cerr << "recv failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanup();
			return 1;
		}
	} while (quit != 'q');
	
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
#ifdef _WIN32
	setlocale(LC_ALL, ""); //For some reason, Windows terminal will not output UTF-8 characters without this function.
#endif // _WIN32

	bool isClient {false};
	std::string test {"oioioioi"};
	std::string receiveBufferString{};	
	SOCKET sokket {INVALID_SOCKET};

	if (isClient) {
		sokket::client::setupSocket(sokket);

		std::uint64_t size {test.size()};
		sokket::sendSocket(sokket, test, size);
		sokket::shutdownSocket(sokket);
		system("pause"); //Temporary for debugging.
	}
	else {
		sokket::server::setupSocket(sokket);

		sokket::receiveSocket(sokket, receiveBufferString);
		sokket::shutdownSocket(sokket);
	}

	return 0;
}
