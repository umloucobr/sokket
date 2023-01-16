#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP
#include "sokket.hpp"

namespace sokket {
	namespace server {
		int setupSocket(SOCKET& sokket) noexcept;
	}
}
#endif //SERVER_HPP

