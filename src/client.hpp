#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "sokket.hpp"
namespace sokket {
	namespace client {
		SOCKET setupSocket (std::string& string, bool isText);
	}
}
#endif //CLIENT_HPP

