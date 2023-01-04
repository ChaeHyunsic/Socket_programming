#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

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

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) return -1;

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) return -2;

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) return -3;

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	// accept()
	addrlen = sizeof(clientaddr);
	client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET)
	{
		return -4;
	}
	// 접속한 클라이언트 정보 출력
	printf("\nFileSender 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	char fileName[256];
	ZeroMemory(fileName, 256);

	// 받을 파일 이름 크기 받기 (고정길이)
	retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR)
	{
		return -5;
	}

	// 받을 파일 이름 받기 (가변길이)
	retval = recvn(client_sock, fileName, len, 0);
	if (retval == SOCKET_ERROR)
	{
		return -6;
	}

	printf("받을 파일 이름: %s\n", fileName);

	// 정상적으로 전송받았는지 체크하기 위해 미리 전송 받을 데이터의 크기를 저장
	int totalData;
	retval = recvn(client_sock, (char*)&totalData, sizeof(totalData), 0);
	if (retval == SOCKET_ERROR)
	{
		return -7;
		closesocket(client_sock);
	}
	printf("받을 데이터 크기: %d\n", totalData);

	// 파일 열기
	FILE* fp = fopen(fileName, "wb"); 
	if (fp == NULL)
	{
		return -8;
		closesocket(client_sock);
	}

	// 파일 데이터 받기
	int recvData = 0;
	while (1)
	{
		// 받을 데이터 크기 받기 (고정길이)
		retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			return -9;
			break;
		}

		// 받을 데이터 받기 (가변길이)
		retval = recvn(client_sock, buf, len, 0);
		if (retval == SOCKET_ERROR)
		{
			return -10;
			break;
		}
		else if (retval == 0)
			break;
		else
		{
			fwrite(buf, 1, retval, fp);
			if (ferror(fp))
			{
				return -11;
				break;
			}
			recvData += retval;
		}
	}
	fclose(fp);

	// 전송 결과 출력
	if (recvData == totalData)
		printf("-> 파일 전송 성공\n");
	else
		printf("-> 파일 전송 실패\n");

	// closesocket()
	closesocket(client_sock);

	printf("FileSender 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();

	return 0;
}