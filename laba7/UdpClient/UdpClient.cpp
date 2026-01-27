#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main()
{
    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    int wsOk = WSAStartup(version, &data);
    if (wsOk != 0)
    {
        cout << "Can't start Winsock! " << wsOk;
        return;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

    cout << "UDP Client ready. Type your message." << endl;

    string msg = "";
    while (true)
    {
        cout << "> ";
        getline(cin, msg);

        if (msg == "quit") break;

        int sendOk = sendto(out, msg.c_str(), msg.size() + 1, 0, (sockaddr*)&server, sizeof(server));

        if (sendOk == SOCKET_ERROR)
        {
            cout << "That didn't work! " << WSAGetLastError() << endl;
        }
    }

    closesocket(out);
    WSACleanup();
}