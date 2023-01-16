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
	std::uint64_t bytesSent {};
	std::uint64_t sendSocketSize {}; //Size of the packet.
	std::string buffer{}; //This buffer gets sendBuffer and divide it to match packet max size.
	std::string totalSize {std::to_string(sendBufferSize)}; //Transform int in a std::string to send size over packet. Yes, it's trash.

	//Basically, if it is a message the size number will begin with a 0, and a 1 for files.
	totalSize.insert(0, 1, '0');
	//Send size first.
	iResult = send(_sokket, totalSize.c_str(), totalSize.size(), 0);
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

		iResult = send(_sokket, buffer.c_str(), sendSocketSize, 0);
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

//Input will come as a std::string with ":f" "Name of the file" "Path to the file".
int sokket::sendSocketFile(SOCKET& _sokket, std::string& input) {
	using binary_data = std::vector<std::uint8_t>;

	bool fileCombination {true}; //This goes on the loop to remove the :f. 
	int iResult {};
	std::uint64_t bytesRemaining {};
	std::uint64_t sendSocketSize {}; //Size of the packet.
	std::uint64_t bytesSent {};
	std::string tempString{}; //Separate the file mode combination, name of the file and it's path.
	std::string fileName{};
	std::string filePath{};
	std::string totalSize{}; //Send total size.
	binary_data sendBuffer(sokket::config::bufferSize); //Lots of buffers, but this one is the one that is actually sent. Has a max size of sokket::config::bufferSize.

	for (auto c: input) {
		if (c == ' ' && fileCombination) {
			fileCombination = false;
			tempString = "";
		}
		else if (c == ' ' && !fileCombination)
		{
			fileName = tempString;
			tempString = "";
		}
		else {
			tempString += c;
		}
	}

	filePath = tempString;

	std::ifstream file {filePath, std::ifstream::binary};

	if (file.is_open()) {
		//Get file size.
		std::filesystem::path fileSystemPath{filePath};
		uint64_t fileSize {std::filesystem::file_size(fileSystemPath)};

		binary_data wholeFile(fileSize); //This is the buffer that contains the whole file.
		file.read(reinterpret_cast<char*>(wholeFile.data()), fileSize);

		bytesRemaining = fileSize;
		totalSize = std::to_string(fileSize);

		//Basically, if it is a message the size number will begin with a 0, and a 1 for files.
		totalSize.insert(0, 1, '1');

		//Send size first.
		iResult = send(_sokket, totalSize.c_str(), totalSize.size(), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send size failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanup();
			return 1;
		}

		//Send file name.
		iResult = send(_sokket, fileName.c_str(), fileName.size(), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send file name failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanup();
			return 1;
		}

		//Send contents.
		while (bytesSent < fileSize) {
			sendBuffer.clear();
			sendSocketSize = std::min(bytesRemaining, static_cast<std::uint64_t>(sokket::config::bufferSize));
			{
				int i {0};
				uint64_t j {bytesSent};
				for (i = 0, j = bytesSent; i <= sendSocketSize - 1; i++, j++) {
					sendBuffer[i] = wholeFile[j];
				}
			}

			iResult = send(_sokket, reinterpret_cast<char*>(sendBuffer.data()), sendSocketSize, 0);
			if (iResult == SOCKET_ERROR) {
				std::cerr << "send failed with error: " << WSAGetLastError() << ".\n";
				closesocket(_sokket);
				WSACleanup();
				return 1;
			}

			bytesSent += sendSocketSize;
			bytesRemaining -= sendSocketSize;
		}
	}
	else {
		std::cout << "Error. Could not open file.\n";
	}
	
	return 0;
}

int sokket::receiveSocket(SOCKET& _sokket, std::string& receivedInformation, bool& disconnect) {
	using binary_data = std::vector<std::uint8_t>;

	bool packetEnd {false};
	bool receiveSize {true}; //The first packet is the size of the contents.
	bool fileModeBool {false};
	bool receiveFileName {false}; //If it's a file, the second packet contains the file name.
	char receiveBuffer[sokket::config::bufferSize + 1]; //1 more space for a null terminator if necessary.	
	char quit {};
	int iResult {};
	std::uint64_t contentsSize {};
	std::uint64_t bytesReceived {};
	std::uint64_t receivedInformationFileIterator {};
	std::string fileName{"default.txt"};
	binary_data receivedInformationFile {};

	do {
		iResult = recv(_sokket, receiveBuffer, sokket::config::bufferSize + 1, 0);
		if (iResult > 0) {
			//The first packet is always the total size in bytes of the contents, so this receive the size. If it begins with a 0 it is a message, or a 1 for files.
			if (receiveSize) {
				receiveBuffer[iResult] = '\0';

				if (receiveBuffer[0] == '0') {
					fileModeBool = false;
				}
				else {
					fileModeBool = true;
					receiveFileName = true;
				}

				receiveBuffer[0] = '0';

				receiveSize = false;
				contentsSize = atoi(receiveBuffer); //Transform contents size to a 64 bits unsigned integer because you can only send and receive a const char* buffer.
				receivedInformationFile.resize(contentsSize);
			}
			else if(receiveFileName) {
				receiveBuffer[iResult] = '\0';

				fileName = receiveBuffer;
				receiveFileName = false;
			}
			else {
				if (fileModeBool) {
					packetEnd = false;

					bytesReceived += iResult;
					for (int i {0}; i < iResult; i++) {
						receivedInformationFile[receivedInformationFileIterator] = receiveBuffer[i];
						receivedInformationFileIterator++;
					}
				}
				else {
					packetEnd = false;

					receiveBuffer[iResult] = '\0';

					bytesReceived += iResult;
					receivedInformation += receiveBuffer;
				}
			}

			if (bytesReceived == contentsSize) {   
				bytesReceived = 0;
				iResult = 0;
				receiveSize = true;
				std::cout << '\b' << '\b';
				if (!fileModeBool) {
					std::cout << receivedInformation << "    S: " << contentsSize << '\n' << ">";
				}
				else {
					fileModeBool = false;

					std::ofstream file {fileName, std::ofstream::binary};
					file.write(reinterpret_cast<const char*>(receivedInformationFile.data()), receivedInformationFile.size());

					if (file.good()) {
						std::cout << "File written to path.\n";
					}
					else {
						std::cout << "Could not write file.\n";
					}
				}

				//If other user send the quit command, disconnect = true so sokket::clparser can disconnect too.
				if (receivedInformation == sokket::clparser::config::quitCombination) { 
					disconnect = true;
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
	_setmode(_fileno(stdin), _O_WTEXT); //Correctly get UTF-8 characters with this obscure function.
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
