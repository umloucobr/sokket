#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"
#include "commandlineparser.hpp"

extern std::string sokket::config::port {"27015"};
extern std::string sokket::config::address {"localhost"};
extern const int sokket::config::bufferSize {1400};

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
		std::cout << "Error! Sent " << bytesSent << " but total number of bytes is " << sendBufferSize << ".\n";
	}

	return 0;
}

//Input will come as a std::string with "Name of the file" "Path to the file".
int sokket::sendSocketFile(SOCKET& _sokket, std::string& input) {
	int iResult {0};
	std::string tempString{};
	std::string fileName{};
	std::string filePath{};

	for (auto c: input)
	{
		if (c == ' ')
		{
			fileName = tempString;
			tempString = "";
		}
		else
		{
			tempString += c;
		}
	}

	filePath = tempString;

	return 0;
};

int sokket::receiveSocket(SOCKET& _sokket, std::string& receivedInformation, bool& disconnect, std::atomic<bool>& fileModeBool) {
	char receiveBuffer[sokket::config::bufferSize + 1]; //1 more space for a null terminator if necessary.	
	char quit {};
	int iResult {0};
	std::uint64_t contentsSize {};
	std::uint64_t bytesReceived {};
	bool packetEnd {false};
	bool receiveSize {true}; //The first packet is the size of the contents.

	do {
		iResult = recv(_sokket, receiveBuffer, sokket::config::bufferSize + 1, 0);
		if (iResult > 0) {
			//The first packet is always the total size in bytes of the contents, so this receive the size.
			if (receiveSize) {
				receiveBuffer[iResult] = '\0';

				receiveSize = false;
				contentsSize = atoi(receiveBuffer); //Transform contents size to a 64 bits unsigned integer because you can only send and receive a const char* buffer.
			}
			else {
				packetEnd = false;

				if (receiveBuffer[iResult - 1] == '\n') {
					receiveBuffer[iResult - 1] = '\0';
				}

				receiveBuffer[iResult] = '\0';

				bytesReceived += iResult;
				receivedInformation.append(receiveBuffer);
			}

			if (bytesReceived == contentsSize) {   
				bytesReceived = 0;
				iResult = 0;
				receiveSize = true;
				std::cout << '\b' << '\b';
				std::cout << receivedInformation << "    S: " << contentsSize << '\n' << ">";

				//If other user send the quit command, disconnect = true so sokket::clparser can disconnect too.
				if (receivedInformation == sokket::clparser::config::quitCombination) { 
					disconnect = true;
				}
				else if (receivedInformation == sokket::clparser::config::fileMode) {
					fileModeBool.store(true);
				}
				else if (receivedInformation == sokket::clparser::config::messageMode) {
					fileModeBool.store(false);
				}
				receivedInformation = "";
			}
		}

		else if (iResult == 0 || !packetEnd) {
			packetEnd = true;
			receiveSize = true;
		}

		else if (iResult < 0) {
			std::cerr << "recv failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);
	
	return 0;
}

int sokket::shutdownSocket(SOCKET& _sokket) {
	int iResult {0};

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
	_setmode(_fileno(stdin), _O_WTEXT); //Correctly get UTF-8 characters.
#endif // _WIN32

	bool isClient {false};
	std::string receiveString{};	
	SOCKET sokket {INVALID_SOCKET};

	if (isClient) {
		sokket::client::setupSocket(sokket); //sokket::shutdownSocket is inside clparser. Sorry.

		int result {sokket::clparser::init(sokket, receiveString)}; //sokket::clparser::init returns 0 if there were no errors, so you can shutdown correctly.
		
		if (result == 0) {
			return 0;
		}
		else {
			return 1;
		}
	}
	else {
		sokket::server::setupSocket(sokket); //sokket::shutdownSocket is inside clparser. Sorry.

		int result {sokket::clparser::init(sokket, receiveString)}; //sokket::clparser::init returns 0 if there were no errors, so you can shutdown correctly.

		if (result == 0) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}
