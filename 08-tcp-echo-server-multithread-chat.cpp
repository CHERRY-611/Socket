#include <iostream>
#include <string>
#include <winsock2.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

#define PORT 65456
#define HOST "127.0.0.1"

#pragma comment(lib, "ws2_32.lib")

std::atomic<bool> terminateServer(false);
int threadNumber = 1;
std::vector<SOCKET> connectedClients;

void HandleClient(int threadnum, SOCKET clientSocket) {
    char buf[1024];
    int bytesReceived;

    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);

    getpeername(clientSocket, (struct sockaddr*)&clientAddr, &addrLen);
    std::cout << "> client connected from IP Address " << inet_ntoa(clientAddr.sin_addr)
        << " with Port number " << ntohs(clientAddr.sin_port) << std::endl;

    connectedClients.push_back(clientSocket);

    do {
        memset(buf, 0, 1024);
        bytesReceived = recv(clientSocket, buf, 1024, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error in recv" << std::endl;
            break;
        }

        std::cout << "> received ( " << buf << " ) and echoed to " << threadNumber - 1 << " clients" << std::endl;

        for (SOCKET& conn : connectedClients) {
            if (conn != clientSocket) {
                send(conn, buf, bytesReceived, 0);
            }
        }

    } while (bytesReceived > 0 && std::string(buf) != "quit");

    auto it = std::find(connectedClients.begin(), connectedClients.end(), clientSocket);
    
    if (it != connectedClients.end()) {
        connectedClients.erase(it);
    }

    threadNumber--;
    std::cout << "> client disconnected" << std::endl;

    closesocket(clientSocket);
}

void ServerThread() {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(HOST);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::vector<std::thread> clientThreads;
    std::cout << "> [Thread " << threadNumber << "] echo-server is activated" << std::endl;

    while (!terminateServer) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int result = select(0, &readSet, nullptr, nullptr, &timeout);

        if (result == SOCKET_ERROR) {
            std::cerr << "Select failed with error: " << WSAGetLastError() << std::endl;
            break;
        }

        if (result == 0) {
            continue;
        }

        if (FD_ISSET(serverSocket, &readSet)) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
                closesocket(serverSocket);
                WSACleanup();
                return;
            }

            threadNumber++;

            clientThreads.emplace_back(HandleClient, threadNumber, clientSocket);
        }
    }

    for (auto& thread : clientThreads) {
        thread.join();
    }

    std::cout << "> [Thread " << threadNumber << "] echo-server is de-activated" << std::endl;

    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    std::thread serverThread(ServerThread);

    while (true) {
        std::string msg;
        std::cout << "> ";
        std::getline(std::cin, msg);
        if (msg == "quit") {
            if (threadNumber == 1) {
                std::cout << "> stop procedure started" << std::endl;
                terminateServer = true;
                break;
            }
            else {
                std::cout << "> active threads are remaining: " << threadNumber - 1 << " threads" << std::endl;
            }
        }
    }

    serverThread.join();
    return 0;
}