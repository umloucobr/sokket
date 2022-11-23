#include "server.hpp"

SOCKET sokket::server::setupSocket() {
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

    int iResult {getaddrinfo(NULL, sokket::config::port.c_str(), &hints, &result)};
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed: " << iResult << ".\n";
        WSACleanup();
        return 1;
    }

    //Create a socket for the server to listen to the client.
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << ".\n";
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //Bind socket.
    iResult = bind(ListenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "bind failed with error: " << WSAGetLastError() << ".\n";
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    //Listen for connections.
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << ".\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    SOCKET ClientSocket {INVALID_SOCKET};

    // Accept a client socket.
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "accept failed: " << WSAGetLastError() << ".\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    return ClientSocket;
}