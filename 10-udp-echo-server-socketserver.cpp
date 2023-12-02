#include <iostream>
#include <cstring>
#include <thread>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

class MyUDPHandler {
public:
    MyUDPHandler(SOCKET socket, const sockaddr_in& clientAddress) : socket_(socket), clientAddress_(clientAddress) {}

    void handle() {
        char recvData[1024];
        int recvDataSize;
        while (true) {
            recvDataSize = recvfrom(socket_, recvData, sizeof(recvData), 0, (struct sockaddr*)&clientAddress_, &clientAddressSize_);
            if (recvDataSize == SOCKET_ERROR) {
                std::cerr << "> Error receiving data: " << WSAGetLastError() << std::endl;
                break;
            }
            std::cout << "> Echoed: " << recvData << std::endl;
            sendto(socket_, recvData, recvDataSize, 0, (struct sockaddr*)&clientAddress_, clientAddressSize_);
        }
    }

private:
    SOCKET socket_;
    sockaddr_in clientAddress_;
    int clientAddressSize_ = sizeof(clientAddress_);
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "> Failed to initialize Winsock" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "> Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(65456);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "> Error binding socket: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "> Echo server is activated" << std::endl;

    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);

    char buf[1024];

    while (true) {

        memset(buf, 0, 1024);
        int recvDataSize = recvfrom(serverSocket, buf, sizeof(buf), 0, (struct sockaddr*)&clientAddress, &clientAddressSize);
        if (recvDataSize == SOCKET_ERROR) {
            std::cerr << "Error in recv" << std::endl;
            break;
        }

        std::cout << "> echoed: " << buf << std::endl;

        sendto(serverSocket, buf, recvDataSize, 0, (struct sockaddr*)&clientAddress, clientAddressSize);
        
    }

    std::cout << "> Echo server is deactivated" << std::endl;

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}