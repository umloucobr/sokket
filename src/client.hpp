#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "sokket.hpp"

namespace sokket {
	namespace client {
		int setupSocket (SOCKET& sokket, bool autoMode) noexcept;
	}
}
#endif //CLIENT_HPP

