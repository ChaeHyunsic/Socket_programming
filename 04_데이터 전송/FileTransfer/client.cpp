#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

int main()
{
	int retval;
	int len;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) return -1;

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) return -2;

	// 데이터 통신에 사용할 변수
	char fileName[256];
	ZeroMemory(fileName, 256);

	printf("보낼 파일 이름 : ");
	scanf("%s", &fileName);

	// 파일 열기
	FILE* fp = fopen(fileName, "rb");
	if (fp == NULL)
	{
		return -3;
	}

	// 보낼 파일 이름 크기 전송 (고정길이)
	len = strlen(fileName);
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) return -4;

	// 보낼 파일 이름 전송 (가변길이)
	retval = send(sock, fileName, len, 0);
	if (retval == SOCKET_ERROR) return -5;

	// 전송할 파일 크기 확인
	fseek(fp, 0, SEEK_END);
	int totalData = ftell(fp);

	// 정상적으로 전송받았는지 체크하기 위해 미리 보낼 데이터의 크기를 전송
	retval = send(sock, (char*)&totalData, sizeof(totalData), 0);
	printf("보낼 데이터 크기: %d\n", totalData);
	if (retval == SOCKET_ERROR) return -6;

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	int sendData;
	int chkData = 0;

	// 파일 데이터 보내기
	fseek(fp, 0, SEEK_SET);
	while (1)
	{
		sendData = fread(buf, 1, BUFSIZE, fp);
		if (sendData > 0)
		{
			// 보낼 데이터 크기 전송 (고정길이)
			retval = send(sock, (char*)&sendData, sizeof(int), 0);
			if (retval == SOCKET_ERROR)
			{
				return -7;
				break;
			}

			// 보낼 데이터 전송 (가변길이)
			retval = send(sock, buf, sendData, 0);
			if (retval == SOCKET_ERROR)
			{
				return -8;
				break;
			}
			chkData += sendData;
		}
		else if (sendData == 0 && chkData == totalData)
		{
			printf("-> 파일 전송 성공\n");
			break;
		}
		else
		{
			printf("-> 파일 전송 실패\n");
			return -9;
			break;
		}
	}
	fclose(fp);

	// closesocket()
	closesocket(sock);

	// 윈속 종료
	WSACleanup();

	return 0;
}