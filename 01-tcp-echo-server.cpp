#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <cstring>
#include <string>

#define PORT 65456
#pragma comment (lib,"ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "> echo-server is activated" << std::endl;

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
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