#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

int main()
{
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        std::cout << "Error in WSAStartup " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // Changed to TCP socket
    if (tcpSocket == INVALID_SOCKET)
    {
        std::cout << "Error creating socket " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in destinationAddr;
    destinationAddr.sin_family = AF_INET;
    destinationAddr.sin_port = htons(8888);
    const char* ip = "34.213.232.150";
    int inetResult = inet_pton(AF_INET, ip, &destinationAddr.sin_addr);

    if (inetResult != 1)
    {
        std::cout << "Error in inet_pton " << WSAGetLastError() << std::endl;
        closesocket(tcpSocket);
        WSACleanup();
        return 1;
    }

    int connectResult = connect(tcpSocket, (struct sockaddr*)&destinationAddr, sizeof(destinationAddr));
    if (connectResult == SOCKET_ERROR)
    {
        std::cout << "Error connecting " << WSAGetLastError() << std::endl;
        closesocket(tcpSocket);
        WSACleanup();
        return 1;
    }

    u_long nonBlocking = 1;
    if (ioctlsocket(tcpSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR) {
        std::cout << "Error setting non-blocking I/O " << WSAGetLastError() << std::endl;
        closesocket(tcpSocket);
        WSACleanup();
        return 1;
    }

    const char* message = __argv[0];
    int sendResult = send(tcpSocket, message, (int)strlen(message), 0);
    if (sendResult == SOCKET_ERROR)
    {
        std::cout << "Error sending " << WSAGetLastError() << std::endl;
        closesocket(tcpSocket);
        WSACleanup();
        return 1;
    }

    int shutdownResult = shutdown(tcpSocket, SD_SEND);
    if (shutdownResult == SOCKET_ERROR)
    {
        std::cout << "Error shutting down " << WSAGetLastError() << std::endl;
        closesocket(tcpSocket);
        WSACleanup();
        return 1;
    }

    std::vector<char> dataBuffer;

    int recvBufferLength = 1500;
    char* recvBuffer = new char[recvBufferLength];
    memset(recvBuffer, 0, recvBufferLength);

    bool waiting = true;
    while (waiting)
    {
        int recvResult = recv(tcpSocket, recvBuffer, recvBufferLength - 1, 0);
        if (recvResult > 0)
        {
            dataBuffer.insert(dataBuffer.end(), recvBuffer, recvBuffer + recvResult);
        }
        else if (recvResult == 0)
        {
            waiting = false;
        }
        else
        {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                std::cout << "Receive error " << error << std::endl;
                closesocket(tcpSocket);
                WSACleanup();
                return 1;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << ". ";
    }

    std::cout.write(dataBuffer.data(), dataBuffer.size()) << std::endl;

    closesocket(tcpSocket);
    WSACleanup();
    delete[] recvBuffer;

    return 0;
}
