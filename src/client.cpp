#include "client.hpp"

int sokket::client::createSocket() {
    struct addrinfo*
        result = NULL,
        * ptr = NULL,
        hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //Ipv4, Ipv6 or unspecified.
    hints.ai_socktype = SOCK_STREAM; //Socktype.
    hints.ai_protocol = IPPROTO_TCP; //Protocol.

    int iResult {getaddrinfo(sokket::config::address.c_str(), sokket::config::port.c_str(), &hints, &result)};
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << ".\n";
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    ptr = result; //getaddrinfo.

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << ".\n";
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
}