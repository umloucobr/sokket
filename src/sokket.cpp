#include "sokket.hpp"

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cout << "Usage: sokket [ip address] [port number] [c for client, s for server]\n";
		return -1;
	}
	else
	{
		sokket::config::port = argv[2];
		sokket::config::address = argv[3];
	}

#ifdef _WIN32
	WSADATA wsaData;
	int iResult{ WSAStartup(MAKEWORD(2, 2), &wsaData) };

	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << "\n";
		return 1;
	}
#endif //_WIN32

	return 0;
}
