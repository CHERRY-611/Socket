#include <iostream>
#include <vector>
#include <cstring>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")


std::vector<SOCKADDR_IN> group_queue;
int threadNumber = 1;


class MyUDPHandler {
public:
    MyUDPHandler() {}

    void handle(char* recvData, int dataSize, SOCKADDR_IN clientAddress, SOCKET recvSocket) {

        std::string recvCmd(recvData, dataSize);

        if (recvCmd[0] == '#' || recvCmd == "quit") {
            if (recvCmd == "#REG") {
                std::cout << "> client registered " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;
                group_queue.push_back(clientAddress);
            }
            else if (recvCmd == "#DEREG" || recvCmd == "quit") {
                auto it = std::find_if(group_queue.begin(), group_queue.end(),
                    [&clientAddress](const SOCKADDR_IN& addr) {
                        return memcmp(&addr, &clientAddress, sizeof(clientAddress)) == 0;
                    });

                if (it != group_queue.end()) {
                    std::cout << "> client de-registered " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << std::endl;
                    group_queue.erase(it);
                }
            }
            sendto(recvSocket, recvData, 0, 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
        }
        else {
            if (group_queue.empty()) {
                std::cout << "> no clients to echo" << std::endl;
                sendto(recvSocket, recvData, 0, 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
            }
            else {
                auto it = std::find_if(group_queue.begin(), group_queue.end(),
                    [&clientAddress](const SOCKADDR_IN& addr) {
                        return memcmp(&addr, &clientAddress, sizeof(clientAddress)) == 0;
                    });

                if (it == group_queue.end()) {
                    std::cout << "> ignores a message from un-registered client" << std::endl;
                    sendto(recvSocket, recvData, 0, 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));

                }
                else {
                    std::cout << "> received (" << recvCmd << ") and echoed to " << group_queue.size() << " clients" << std::endl;
                    for (const auto& clientConn : group_queue) {
                        sendto(recvSocket, recvCmd.c_str(), recvCmd.size(), 0, (SOCKADDR*)&clientConn, sizeof(clientConn));
                    }
                }
            }
        }
    }
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKADDR_IN serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(65456);

    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "> echo-server is activated" << std::endl;

    MyUDPHandler udpHandler;

    while (true) {
        char recvData[1024];
        SOCKADDR_IN clientAddress;
        int clientAddressSize = sizeof(clientAddress);
        int recvResult = recvfrom(serverSocket, recvData, sizeof(recvData), 0, (SOCKADDR*)&clientAddress, &clientAddressSize);

        if (recvResult == SOCKET_ERROR) {
            if (group_queue.size() == 0) {
                break;
            }
            else
                continue;
        }

        udpHandler.handle(recvData, recvResult, clientAddress, serverSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    std::cout << "> echo-server is de-activated" << std::endl;

    return 0;
}