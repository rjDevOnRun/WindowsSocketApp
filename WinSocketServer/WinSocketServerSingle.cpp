#include <iostream>
#include <Winsock2.h> // always before windows.h
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

/* NOTES:
* https://www.youtube.com/watch?v=0Zr_0Jy8mWE
* https://www.youtube.com/watch?v=TP5Q0cs6uNo
* install PUTTY and use it as terminal on the port 54000
* use RAW if you get Invalid Padding
*/

#pragma region BARE-BONES SERVER

int mainforSingleServer()
{
	// Init WinSocket
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2); // need ver 2.0 of WSDATA
	int wsResult = WSAStartup(ver, &wsData);
	if (wsResult != 0)
	{
		cerr << "Can't Initialize WinSock! Err#: " << wsResult << endl;
		return 0;
	}

	// Create a TCP socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Err#: " << WSAGetLastError() << endl;
		return 0;
	}

	// Bind socket to IP address and port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000); // port #
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Can also use "inet_pton"

	bind(sock, (sockaddr*)&hint, sizeof(hint));

	// Tell winsock, this socket is for listening
	listen(sock, SOMAXCONN);

	// Wait for a connection
	sockaddr_in client;
	int clientsize = sizeof(client);
	SOCKET clientSocket = accept(sock, (sockaddr*)&client, &clientsize);
	if (clientSocket == INVALID_SOCKET)
	{
		cerr << "Connected Socket is INVALID! Quitting" << endl;
		return 0;
	}

	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXHOST];	// Service (i.e. port) the client is connected on..

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXHOST, 0) == 0)
	{
		cout << "Connected on port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << "Connected on port " <<
			ntohs(client.sin_port) << endl;
	}

	// Close listening socket
	closesocket(sock);

	// While loop: accept and echo message to client
	char buf[4096];

	while (true)
	{
		ZeroMemory(buf, 4096);

		// Wait for client to send data
		int bytesRecieved = recv(clientSocket, buf, 4096, 0);
		if (bytesRecieved == SOCKET_ERROR)
		{
			cerr << "Error in recv()! Quitting" << endl;
			break;
		}

		if (bytesRecieved == 0)
		{
			cout << "Client disconneted " << endl;
			break;
		}

		// Echo message back
		cout << string(buf, 0, bytesRecieved) << endl;

		// Echo message back to client
		send(clientSocket, buf, bytesRecieved + 1, 0);

	}

	// Close Socket
	closesocket(clientSocket);

	// Cleanup Winsock
	WSACleanup();

	return 0;
}

#pragma endregion