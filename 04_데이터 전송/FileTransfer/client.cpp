#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERIP   "127.0.0.1"
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

int main()
{
	int retval;
	int len;

	// ���� �ʱ�ȭ
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

	// ������ ��ſ� ����� ����
	char fileName[256];
	ZeroMemory(fileName, 256);

	printf("���� ���� �̸� : ");
	scanf("%s", &fileName);

	// ���� ����
	FILE* fp = fopen(fileName, "rb");
	if (fp == NULL)
	{
		return -3;
	}

	// ���� ���� �̸� ũ�� ���� (��������)
	len = strlen(fileName);
	retval = send(sock, (char*)&len, sizeof(int), 0);
	if (retval == SOCKET_ERROR) return -4;

	// ���� ���� �̸� ���� (��������)
	retval = send(sock, fileName, len, 0);
	if (retval == SOCKET_ERROR) return -5;

	// ������ ���� ũ�� Ȯ��
	fseek(fp, 0, SEEK_END);
	int totalData = ftell(fp);

	// ���������� ���۹޾Ҵ��� üũ�ϱ� ���� �̸� ���� �������� ũ�⸦ ����
	retval = send(sock, (char*)&totalData, sizeof(totalData), 0);
	printf("���� ������ ũ��: %d\n", totalData);
	if (retval == SOCKET_ERROR) return -6;

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE];
	int sendData;
	int chkData = 0;

	// ���� ������ ������
	fseek(fp, 0, SEEK_SET);
	while (1)
	{
		sendData = fread(buf, 1, BUFSIZE, fp);
		if (sendData > 0)
		{
			// ���� ������ ũ�� ���� (��������)
			retval = send(sock, (char*)&sendData, sizeof(int), 0);
			if (retval == SOCKET_ERROR)
			{
				return -7;
				break;
			}

			// ���� ������ ���� (��������)
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
			printf("-> ���� ���� ����\n");
			break;
		}
		else
		{
			printf("-> ���� ���� ����\n");
			return -9;
			break;
		}
	}
	fclose(fp);

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();

	return 0;
}