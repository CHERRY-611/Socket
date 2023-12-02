#include <iostream>
#include <string>
#include <winsock2.h>

#define PORT 65456
#define HOST "127.0.0.1"

#pragma comment (lib, "ws2_32.lib")

void HandleClient(SOCKET clientSocket) {
    char buf[1024];
    int bytesReceived;

    do {
        memset(buf, 0, 1024);
        bytesReceived = recv(clientSocket, buf, 1024, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error in recv" << std::endl;
            break;
        }

        std::cout << "> received from client: " << buf << std::endl;

        send(clientSocket, buf, bytesReceived, 0);

    } while (bytesReceived > 0 && std::string(buf) != "quit");

    std::cout << "> client disconnected" << std::endl;

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(HOST);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "> echo-server is activated" << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        getpeername(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        std::cout << "> client connected by IP address " << inet_ntoa(clientAddr.sin_addr)
            << " with Port number " << ntohs(clientAddr.sin_port) << std::endl;

        HandleClient(clientSocket);

        std::cout << "> waiting for the next client..." << std::endl;
    }

    std::cout << "> echo-server is de-activated" << std::endl;

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}