#include "client.hpp"

SOCKET sokket::client::setupSocket(std::string& string, bool isText) {
#ifdef _WIN32
    WSADATA wsaData;
    int iResulta{ WSAStartup(MAKEWORD(2, 2), &wsaData) };

    if (iResulta != 0) {
        std::cout << "WSAStartup failed: " << iResulta << "\n";
        return 1;
    }
#endif   

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
        std::cerr << "getaddrinfo failed: " << iResult << ".\n";
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    SOCKET ConnectSocket = INVALID_SOCKET;

    for (ptr = result; ptr != NULL; ptr=ptr->ai_next)
    {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            std::cerr << "Error at socket(): " << WSAGetLastError() << ".\n";
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Unable to connect to server!\n";
        WSACleanup();
        return 1;
    }
    return ConnectSocket;
}