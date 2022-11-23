﻿#pragma once
#ifndef SOKKET_HPP
#define SOKKET_HPP

#include <iostream>
#include <string>
#include <cstdint>

#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#endif //_WIN32

namespace sokket {
	namespace config {
		extern std::string port;
		extern std::string address;
		extern const int bufferSize;
	}

	int shutdownSocket (SOCKET& _sokket);
	int sendSocket (SOCKET& _sokket, std::string& sendBuffer, int sendBufferSize);
	int receiveSocket (SOCKET& _sokket, std::string& receiveBufferString);
}

#endif //SOKKET_HPP
