#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define REMOTEIP   "255.255.255.255"
#define REMOTEPORT 9000
#define LOCALPORT 9000
#define BUFSIZE    512

// 소켓 함수 오류 출력 후 종료
void err_quit(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// 소켓 함수 오류 출력
void err_display(char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

DWORD WINAPI sendProc(LPVOID arg) {

    char sendBuf[BUFSIZE + 1];
    int len;
    int retval;
    int addrlen;
    SOCKET sendSock = (SOCKET)arg;

    SOCKADDR_IN remoteaddr;
    ZeroMemory(&remoteaddr, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_addr.s_addr = inet_addr(REMOTEIP);
    remoteaddr.sin_port = htons(REMOTEPORT);

    while (1) {
        printf("전송 : ");
        // 데이터 보내기
        if (fgets(sendBuf, BUFSIZE + 1, stdin) == NULL)
            break;

        // '\n' 문자 제거
        len = strlen(sendBuf);
        if (sendBuf[len - 1] == '\n')
            sendBuf[len - 1] = '\0';

        if (strlen(sendBuf) == 0)
            break;

        // 브로드캐스트 데이터 보내기
        retval = sendto(sendSock, sendBuf, strlen(sendBuf), 0, (SOCKADDR*)&remoteaddr, sizeof(remoteaddr));
        if (retval == SOCKET_ERROR) {
            err_display("sendto()");
            continue;
        }
        printf("[UDP] %d바이트를 보냈습니다.\n", retval);
    }
}

DWORD WINAPI recvProc(LPVOID arg) {

    char recvBuf[BUFSIZE + 1];
    int retval;
    SOCKET sock = (SOCKET)arg;
    SOCKADDR_IN peeraddr;
    int addrlen;

    addrlen = sizeof(peeraddr);
    retval = recvfrom(sock, recvBuf, BUFSIZE, 0, (SOCKADDR*)&peeraddr, &addrlen);
    if (retval == SOCKET_ERROR) {
        err_display("recvfrom()");
    }

    // 받은 데이터 출력
    recvBuf[retval] = '\0';
    printf("\n[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), recvBuf);
}

int main(int argc, char* argv[])
{
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // socket() sender
    SOCKET sendSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sendSock == INVALID_SOCKET) err_quit("socket()");

    // socket() receiver
    SOCKET recvSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (recvSock == INVALID_SOCKET) err_quit("socket()");

    // bind()
    SOCKADDR_IN localaddr;
    ZeroMemory(&localaddr, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localaddr.sin_port = htons(LOCALPORT);
    retval = bind(recvSock, (SOCKADDR*)&localaddr, sizeof(localaddr));
    if (retval == SOCKET_ERROR) err_quit("bind()");

    // 브로드캐스팅 활성화
    BOOL bEnable = TRUE;
    retval = setsockopt(sendSock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable));
    if (retval == SOCKET_ERROR) err_quit("setsockopt()");

    // 스레드 생성
    HANDLE hThread1;
    HANDLE hThread2;
    
    hThread1 = CreateThread(NULL, 0, sendProc, (LPVOID)sendSock, 0, NULL);

    while (1) {
        hThread2 = CreateThread(NULL, 0, recvProc, (LPVOID)recvSock, 0, NULL);
    }

    // closesocket()
    closesocket(sendSock);
    closesocket(recvSock);

    // 윈속 종료
    WSACleanup();

    return 0;
}
