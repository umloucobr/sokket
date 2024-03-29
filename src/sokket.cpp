﻿#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"
#include "commandlineparser.hpp"

extern const int sokket::config::bufferSize {1400};

int sokket::WSACleanupWrapper() {
#ifdef _WIN32
	WSACleanup();
#endif // _WIN32
	return 0;
}

int sokket::sendSocket(SOCKET& _sokket, std::string& input, std::uint64_t inputSize) {
	int iResult {0}; 
	std::uint64_t bytesRemaining {inputSize}; //Used on std::min, making a copy because I need to decrease it.
	std::uint64_t bytesSent {};
	std::uint64_t bufferSize {};
	std::string totalSize {std::to_string(inputSize)}; //Size of the packet in std::string.
	std::string buffer{}; //This buffer gets sendBuffer and divide it to match packet max size. It's the buffer who is actually sent.

	//Basically, if it is a message the size number will begin with a 0, and a 1 for files.
	totalSize.insert(0, 1, '0');
	//Send size first.
	iResult = send(_sokket, totalSize.c_str(), static_cast<int>(totalSize.size()), 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "send size failed with error: " << WSAGetLastError() << ".\n";
		closesocket(_sokket);
		WSACleanupWrapper();
		return 1;
	}
	
	//Send contents.
	while (bytesSent < inputSize) {
		buffer = "";
		bufferSize = std::min(bytesRemaining, static_cast<std::uint64_t>(sokket::config::bufferSize));
		buffer.assign(input, bytesSent, inputSize);

		iResult = send(_sokket, buffer.c_str(), static_cast<int>(bufferSize), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanupWrapper();
			return 1;
		}

		bytesSent += bufferSize;
		bytesRemaining -= bufferSize;
	}

	if (bytesSent != inputSize) {
		std::cout << "Error! Sent " << bytesSent << " but total number of bytes is " << inputSize << ".\n";
	}

	return 0;
}

//Input will come as a std::string with ":f" "Name of the file" "Path to the file".
int sokket::sendSocketFile(SOCKET& _sokket, std::string& input) {
	using binary_data = std::vector<std::uint8_t>;

	bool fileCombination {true}; //This goes on the loop to remove the :f. 
	int iResult {};
	std::uint64_t bytesRemaining {};
	std::uint64_t bytesSent{};
	std::uint64_t bufferSize {}; //Size of the packet.
	std::string tempString{}; //Separate the file mode combination, name of the file and it's path.
	std::string fileName{};
	std::string filePath{};
	std::string totalSize{}; //Send total size.
	binary_data buffer(sokket::config::bufferSize); //Lots of buffers, but this one is the one that is actually sent. Has a max size of sokket::config::bufferSize.

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

		//The 'r' is used to separate the size and name on the receiving end.
		std::string fileNameAndSize {totalSize + '\r' + fileName};

		//Send size and file name.
		iResult = send(_sokket, fileNameAndSize.c_str(), static_cast<int>(fileNameAndSize.size()), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "send size and file name failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanupWrapper();
			return 1;
		}

		//Send contents.
		while (bytesSent < fileSize) {
			buffer.clear();
			buffer.resize(sokket::config::bufferSize);
			bufferSize = std::min(bytesRemaining, static_cast<std::uint64_t>(sokket::config::bufferSize));
			{
				int i {0};
				uint64_t j {bytesSent};
				for (i = 0, j = bytesSent; i <= bufferSize - 1; i++, j++) {
					buffer[i] = wholeFile[j];
				}
			}

			iResult = send(_sokket, reinterpret_cast<char*>(buffer.data()), static_cast<int>(bufferSize), 0);
			if (iResult == SOCKET_ERROR) {
				std::cerr << "send failed with error: " << WSAGetLastError() << ".\n";
				closesocket(_sokket);
				WSACleanupWrapper();
				return 1;
			}

			bytesSent += bufferSize;
			bytesRemaining -= bufferSize;
		}
	}
	else {
		std::cout << "Error. Could not open file.\n";
	}
	
	return 0;
}

int sokket::receiveSocket(SOCKET& _sokket, bool& disconnect) {
	using binary_data = std::vector<std::uint8_t>;

	bool packetEnd {false};
	bool receiveSize {true}; //The first packet is the size of the contents.
	bool fileModeBool {false};
	bool getFileName {false};
	char receiveBuffer[sokket::config::bufferSize + 1] {}; //1 more space for a null terminator if necessary.	
	int iResult {};
	std::uint64_t contentsSize {};
	std::uint64_t bytesReceived {};
	std::uint64_t receivedInformationFileIterator {};
	std::string receivedInformation {};
	std::string fileName {"default.txt"};
	binary_data receivedInformationFile {};
	binary_data receivedNameAndSize{};

	do {
		receiveBuffer[0] = '\0';
		iResult = 0;
		iResult = recv(_sokket, receiveBuffer, sokket::config::bufferSize, 0);
		if (iResult > 0) {
			//The first packet is always the total size in bytes of the contents and the file name, if it's a file. If it begins with a 0 it is a message, or a 1 for files.
			if (receiveSize) {
				receiveBuffer[iResult] = '\0';

				if (receiveBuffer[0] == '0') {
					fileModeBool = false;

					contentsSize = std::atoi(receiveBuffer);
				}
				else {
					fileModeBool = true;

					receiveBuffer[0] = '0';

					receivedNameAndSize.insert(receivedNameAndSize.end(), receiveBuffer , receiveBuffer + iResult);

					{
						bool size{};
						std::string temp{};
						std::string temp2{};
						int j{};
						for (auto i : receivedNameAndSize) {
							if (!getFileName && receivedNameAndSize[j] != '\r') {
								temp.push_back(receivedNameAndSize[j]);
							}
							else if (getFileName || receivedNameAndSize[j] == '\r') {
								getFileName = true;
								if (receivedNameAndSize[j] != '\r') {
									temp2.push_back(receivedNameAndSize[j]);
								}
							}
							j++;
						}

						contentsSize = std::stoll(temp);
						fileName = temp2;
					}
				}
				receiveSize = false;

				if (fileModeBool) {
					receivedInformationFile.resize(contentsSize);
				}
				else {
					receivedInformation.resize(contentsSize);
				}
			}
			else {
				if (fileModeBool) {
					packetEnd = false;

					bytesReceived += iResult;

					for (int i {0}; i <= iResult - 1; i++) {
						receivedInformationFile[receivedInformationFileIterator] = receiveBuffer[i];
						receivedInformationFileIterator++;
					}
				}
				else if (!fileModeBool){
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
					return 0;
				}
				receivedInformation.clear();
			}
		}

		else if (iResult == 0 || !packetEnd) {
			packetEnd = true;
			receiveSize = true;
		}

		else if (iResult < 0) {
			std::cerr << "recv failed with error: " << WSAGetLastError() << ".\n";
			closesocket(_sokket);
			WSACleanupWrapper();
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
		WSACleanupWrapper();
		return 1;
	}

	closesocket(_sokket);
	WSACleanupWrapper();

	return 0;
}
