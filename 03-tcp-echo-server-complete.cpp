#include <iostream>
#include <string>
#include <winsock2.h>

#define PORT 65456

#pragma comment (lib, "ws2_32.lib")

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
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "> echo-server is activated" << std::endl;

    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "accept() failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "> client connected" << std::endl;

    wchar_t recvData[1024];

    while (true) {
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(recvData), sizeof(recvData), 0);
        if (bytesReceived <= 0) {
            break;
        }

        std::wcout << L"> echoed: " << reinterpret_cast<char*>(recvData) << std::endl;
        send(clientSocket, reinterpret_cast<char*>(recvData), bytesReceived, 0);

        if (wcscmp(recvData, L"quit") == 0) {
            break;
        }
    }

    std::cout << "> echo-server is de-activated" << std::endl;

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}