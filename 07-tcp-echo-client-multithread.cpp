#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

#define DEFAULT_PORT "65456"
#define DEFAULT_BUFLEN 1024

void recvHandler(SOCKET clientSocket) {
    char recvData[DEFAULT_BUFLEN];
    int bytesReceived;

    while (true) {
        bytesReceived = recv(clientSocket, recvData, DEFAULT_BUFLEN, 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout << "> Connection closed by server" << std::endl;
            break;
        }

        recvData[bytesReceived] = '\0';
        std::cout << "> received: " << recvData << std::endl;

        if (strcmp(recvData, "quit") == 0) {
            break;
        }
    }
}

int main() {
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        std::cerr << "> WSAStartup failed" << std::endl;
        return -1;
    }

    addrinfo hints, * result = nullptr, * ptr = nullptr;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result) != 0) {
        std::cerr << "> getaddrinfo failed" << std::endl;
        WSACleanup();
        return -1;
    }

    SOCKET clientSocket = INVALID_SOCKET;

    for (ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        clientSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "> Error creating socket" << std::endl;
            freeaddrinfo(result);
            WSACleanup();
            return -1;
        }

        if (connect(clientSocket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "> Unable to connect to server!" << std::endl;
        WSACleanup();
        return -1;
    }

    std::cout << "> echo-client is activated" << std::endl;

    std::thread clientThread(recvHandler, clientSocket);
    clientThread.detach();

    while (true) {
        std::string sendMsg;
        std::getline(std::cin, sendMsg);

        send(clientSocket, sendMsg.c_str(), static_cast<int>(sendMsg.length()), 0);

        if (sendMsg == "quit") {
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    std::cout << "> echo-client is de-activated" << std::endl;

    return 0;
}