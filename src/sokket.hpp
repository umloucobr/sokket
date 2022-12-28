#pragma once
#ifndef SOKKET_HPP
#define SOKKET_HPP

#include <iostream>
#include <string>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <thread>
#include <atomic>

#ifdef _WIN32
//windows.h messes with std::min and std::max.
#define NOMINMAX

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <locale> //For setlocale();

#endif //_WIN32

namespace sokket {
	namespace config {
		extern std::string port;
		extern std::string address;
		extern const int bufferSize;
	}

	int shutdownSocket (SOCKET& _sokket);
	int sendSocket (SOCKET& _sokket, std::string& sendBuffer, std::uint64_t sendBufferSize);
	int receiveSocket (SOCKET& _sokket, std::string& receivedInformation, bool& disconnect);
}

#endif //SOKKET_HPP
