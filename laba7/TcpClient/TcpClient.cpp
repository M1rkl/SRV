#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void main()
{
    string ipAddress = "127.0.0.1";
    int port = 54000;

    // Инициализация
    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if (wsResult != 0) {
        cerr << "Can't start Winsock" << endl;
        return;
    }

    // Создание сокета TCP
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Can't create socket" << endl;
        WSACleanup();
        return;
    }

    // Заполнение структуры
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Подключение к серверу
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        cerr << "Can't connect to server" << endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    char buf[4096];
    string userInput;

    // Цикл отправки
    do
    {
        cout << "> ";
        getline(cin, userInput);

        if (userInput.size() > 0)
        {
            // Отправка
            int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
            if (sendResult != SOCKET_ERROR)
            {
                // Ожидание ответа (Эхо)
                ZeroMemory(buf, 4096);
                int bytesReceived = recv(sock, buf, 4096, 0);
                if (bytesReceived > 0)
                {
                    cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
                }
            }
        }

        // Условие выхода
        if (userInput == "quit") {
            cout << "Quitting..." << endl;
            break;
        }

    } while (userInput.size() > 0);

    closesocket(sock);
    WSACleanup();
}