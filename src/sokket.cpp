﻿#include "sokket.hpp"
#include "client.hpp"
#include "server.hpp"

extern std::string sokket::config::port {"27015"};
extern std::string sokket::config::address {"localhost"};
extern int sokket::config::bufferSize {512};

int main(int argc, char* argv[])
{
	/*if (argc != 4)
	{
		std::cout << "Usage: sokket [ip address] [port number] [c for client, s for server]\n";
		return -1;
	}
	else
	{
		sokket::config::port = argv[2];
		sokket::config::address = argv[3];
	} */

	std::string temp;
	sokket::client::setupSocket(temp, true);
	return 0;
}
