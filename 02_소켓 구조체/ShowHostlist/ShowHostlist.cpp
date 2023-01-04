#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <winsock2.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	char* host_name = argv[1];
	printf("Domain = %s\n\n", host_name);

	HOSTENT* ptr = gethostbyname(host_name);

	for (int i = 0; ptr->h_aliases[i] != NULL; i++)
	{
		printf("Alias = %s\n", ptr->h_aliases[i]);
	}

	printf("\n");

	for (int i = 0; ptr->h_addr_list[i] != NULL; i++)
	{
		printf("IPv4 = %s\n", inet_ntoa(*(IN_ADDR*)ptr->h_addr_list[i]));
	}

	printf("\n");

	WSACleanup();

	return 0;
}