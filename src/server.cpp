#include "server.hpp"

int sokket::server::setupSocket(SOCKET& sokket) noexcept {
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
        WSACleanupWrapper();
        return 1;
    }

    //Create a socket for the server to listen to the client.
    SOCKET listenSocket = INVALID_SOCKET;
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << ".\n";
        freeaddrinfo(result);
        WSACleanupWrapper();
        return 1;
    }

    //Bind socket.
    iResult = bind(listenSocket, result->ai_addr, static_cast<int>(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "bind failed with error: " << WSAGetLastError() << ".\n";
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanupWrapper();
        return 1;
    }

    freeaddrinfo(result);

    //Listen for connections.
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << ".\n";
        closesocket(listenSocket);
        WSACleanupWrapper();
        return 1;
    }

    SOCKET clientSocket {INVALID_SOCKET};

    // Accept a client socket.
    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "accept failed: " << WSAGetLastError() << ".\n";
        closesocket(listenSocket);
        WSACleanupWrapper();
        return 1;
    }

    sokket = clientSocket;
    return 0;
}