#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 512

// ����� ���� ������ ���� �Լ�
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

	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
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
	// ������ Ŭ���̾�Ʈ ���� ���
	printf("\nFileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	char fileName[256];
	ZeroMemory(fileName, 256);

	// ���� ���� �̸� ũ�� �ޱ� (��������)
	retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR)
	{
		return -5;
	}

	// ���� ���� �̸� �ޱ� (��������)
	retval = recvn(client_sock, fileName, len, 0);
	if (retval == SOCKET_ERROR)
	{
		return -6;
	}

	printf("���� ���� �̸�: %s\n", fileName);

	// ���������� ���۹޾Ҵ��� üũ�ϱ� ���� �̸� ���� ���� �������� ũ�⸦ ����
	int totalData;
	retval = recvn(client_sock, (char*)&totalData, sizeof(totalData), 0);
	if (retval == SOCKET_ERROR)
	{
		return -7;
		closesocket(client_sock);
	}
	printf("���� ������ ũ��: %d\n", totalData);

	// ���� ����
	FILE* fp = fopen(fileName, "wb"); 
	if (fp == NULL)
	{
		return -8;
		closesocket(client_sock);
	}

	// ���� ������ �ޱ�
	int recvData = 0;
	while (1)
	{
		// ���� ������ ũ�� �ޱ� (��������)
		retval = recvn(client_sock, (char*)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			return -9;
			break;
		}

		// ���� ������ �ޱ� (��������)
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

	// ���� ��� ���
	if (recvData == totalData)
		printf("-> ���� ���� ����\n");
	else
		printf("-> ���� ���� ����\n");

	// closesocket()
	closesocket(client_sock);

	printf("FileSender ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();

	return 0;
}