#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    if (WSAStartup(ver, &wsData) != 0) {
        std::cerr << "Error initializing winsock" << std::endl;
        return -1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(65456);
    inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);

    if (connect(clientSocket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << "Can't connect to server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
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

    closesocket(clientSocket);
    WSACleanup();

    std::cout << "> echo-client is de-activated" << std::endl;

    return 0;
}