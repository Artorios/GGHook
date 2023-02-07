#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>
#include "SendTCP.h"

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996) 

WSADATA wsa;
SOCKET s;
struct sockaddr_in server;
char *message, server_reply[2000];
int recv_size;

int SendPacket(char *ip, int port, char *data, int size) 
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return 2;
		
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
		return 3;

	if (send(s, data, size, 0) < 0)
		return 4;


	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
		return 5;

	closesocket(s);
	WSACleanup();
	return 0;
}

int SendHTTP()
{
	SendPacket("127.0.0.1", 9090, "AAAAAAAAAAAAAAAAAA", 6);
	return 0;
}
