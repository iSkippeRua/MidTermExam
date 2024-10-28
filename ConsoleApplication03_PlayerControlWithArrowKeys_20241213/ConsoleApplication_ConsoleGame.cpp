#include <winsock2.h>
#include "MyUtil.h"
#include <cwchar>
#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <Windows.h>
#include <WinUser.h>
#include <strsafe.h>
#include "KInput.h"
#include <vector>
#include <iostream>

#pragma warning(disable:4996)
#pragma comment(lib, "ws2_32.lib")

double g_drawScale = 1.0;
bool isChatMode = false;

class KVector2
{
public:
    double x;
    double y;
};

SOCKET s;
KVector2 g_characterPos{10,10};

void InitializeSocketClient() {
    WSADATA wsaData;
    int Ret;

    if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        printf("WSAStartup failed with error %d\n", Ret);
        exit(1);
    }

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        printf("Socket creation failed with error %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    SOCKADDR_IN ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(5150); // Port number
    ServerAddr.sin_addr.s_addr = inet_addr("192.168.56.1"); // Server IP

    printf("Connecting to server...\n");
    if (connect(s, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)) == SOCKET_ERROR) {
        printf("Connect failed with error %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        exit(1);
    }
    printf("Connected to server!\n");
}

void DrawLine(double x, double y, double x2, double y2, char ch)
{
    KVector2 center{ g_width / 2.0, g_height / 2.0 };
    ScanLine(int(x * g_drawScale + center.x), int(-y * g_drawScale + center.y)
        , int(x2 * g_drawScale + center.x), int(-y2 * g_drawScale + center.y), ch);
}

void Update(double elapsedTime)
{
    g_drawScale = 1.0;
    DrawLine(-g_width / 2, 0, g_width / 2, 0, '.');
    DrawLine(0, -g_height / 2, 0, g_height / 2, '.');

    PutTextf(0, 0, "%g", elapsedTime);
    //
    // game object update logic here
    //
    if (Input.GetKeyDown(VK_LEFT))
        g_characterPos.x -= 1;
    if (Input.GetKeyDown(VK_RIGHT))
        g_characterPos.x += 1;
    if (Input.GetKeyDown(VK_UP))
        g_characterPos.y -= 1;
    if (Input.GetKeyDown(VK_DOWN))
        g_characterPos.y += 1;
}

void DrawGameWorld() {
    //
    // game object drawing routine here
    //
    float h = Input.GetAxis("Horizontal");
    float v = Input.GetAxis("Vertical");
    PutTextf(1, 1, "Simultaneous Key Processing:");
    PutTextf(1, 2, "h = %g", h);
    PutTextf(1, 3, "v = %g", v);
    PutText(g_characterPos.x, g_characterPos.y, "P");
    DrawBuffer();
}

void SendMessageToServer(const std::string& message) {
    send(s, message.c_str(), message.size(), 0);
}

std::string ReceiveMessageFromServer() {
    char DataBuffer[1024] = {};
    int Ret = recv(s, DataBuffer, sizeof(DataBuffer), 0);
    return std::string(DataBuffer);
}

int main() {
    InitializeSocketClient();

    g_hwndConsole = GetConsoleWindow();
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    ShowCursor(false);

    bool isGameLoop = true;

    clock_t prevClock = clock();
    clock_t currClock = prevClock;

    while (isGameLoop) {
        prevClock = currClock;
        currClock = clock();
        const double elapsedTime = ((double)currClock - (double)prevClock) / CLOCKS_PER_SEC;

        if (_kbhit()) {
            int key = _getch();
            if (key == VK_ESCAPE) {
                isGameLoop = false;
            }
            else if (key == VK_RETURN) {
                isChatMode = !isChatMode;
            }
        }

        if (isChatMode) {
            std::string message;
            printf("\n Client: ");
            std::getline(std::cin, message);
            SendMessageToServer(message);

            std::string serverResponse = ReceiveMessageFromServer();
            if (!serverResponse.empty()) {
                printf("Server: %s\n", serverResponse.c_str());
                isChatMode = false;
            }
        }
        else {
            ClearBuffer();
            Input.Update(elapsedTime);
            Update(elapsedTime);
            Sleep(10);
            DrawGameWorld();
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}

