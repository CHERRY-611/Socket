#include <iostream>
#include <string>
#include <winsock2.h>

#define PORT 65456
#define HOST "127.0.0.1"

#pragma comment (lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(HOST);
    serverAddr.sin_port = htons(PORT);

    try {
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            throw WSAGetLastError();
        }
    }
    catch (int error) {
        std::cerr << "connect() failed: " << error << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "> echo-client is activated" << std::endl;

    char buf[1024];
    while (true) {
        std::string sendMsg;
        std::cout << "> ";
        std::getline(std::cin, sendMsg);

        send(clientSocket, sendMsg.c_str(), sendMsg.size() + 1, 0);

        memset(buf, 0, 1024);
        int bytesReceived = recv(clientSocket, buf, 1024, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error in recv" << std::endl;
            break;
        }

        std::cout << "> received: " << buf << std::endl;

        if (sendMsg == "quit") {
            break;
        }
    }

    std::cout << "> echo-client is de-activated" << std::endl;

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}