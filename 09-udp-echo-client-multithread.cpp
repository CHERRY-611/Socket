#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

void recvHandler(SOCKET clientSocket) {
    char recvData[1024];
    int recvDataSize;

    while (true) {
        recvDataSize = recv(clientSocket, recvData, sizeof(recvData) - 1, 0);
        if (recvDataSize == SOCKET_ERROR ) {
            break;
        }
        if (recvDataSize > 0) {
            recvData[recvDataSize] = '\0';
            std::cout << "> Received: " << recvData << std::endl;
        }
        if (std::strcmp(recvData, "quit") == 0) {
            break;
        }
        if (recvDataSize == 0) {
            continue;
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "> Failed to initialize Winsock" << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "> Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; 
    serverAddress.sin_port = htons(0);

    if (bind(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "> Error binding socket: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(65456);

    std::cout << "> Echo client is activated" << std::endl;

    std::thread recvThread(recvHandler, clientSocket);
    recvThread.detach();

    char buf[1024];

    while (true) {
        std::string sendMsg;
        std::cout << "> ";
        std::getline(std::cin, sendMsg);

        sendto(clientSocket, sendMsg.c_str(), sendMsg.size(), 0, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

        if (sendMsg == "quit") {
            break;
        }   
    }

    std::cout << "> Echo client is deactivated" << std::endl;

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}