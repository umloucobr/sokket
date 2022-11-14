#pragma once
#ifndef SOKKET_HPP
#define SOKKET_HPP

#include <iostream>
#include <string>

namespace sokket {
	namespace config {
		extern std::string port;
		extern std::string address;
	}
}
#ifdef _WIN32

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#endif //_WIN32

#endif //SOKKET_HPP
