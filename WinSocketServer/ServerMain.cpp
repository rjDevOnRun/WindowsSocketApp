#include <iostream>
#include <Winsock2.h> // always before windows.h
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

/* NOTES:
* https://www.youtube.com/watch?v=0Zr_0Jy8mWE
* https://www.youtube.com/watch?v=TP5Q0cs6uNo
* https://www.youtube.com/watch?v=dquxuXeZXgo
* https://www.youtube.com/watch?v=wKxbjB6zqS8
* 
* make sure to run the Server app before the Clients
* install PUTTY and use it as terminal on the port 54000
* use RAW if you get Invalid Padding
*/

#pragma region BARE-BONES SERVER

int main()
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
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Err#: " << WSAGetLastError() << endl;
		return 0;
	}

	// Initialize hint
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000); // port #
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Can also use "inet_pton"

	// Bind socket to IP address and port
	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell winsock, this socket is for listening
	listen(listening, SOMAXCONN);

	// Init Multi-Client listening capabilities
	fd_set master;
	FD_ZERO(&master);
	FD_SET(listening, &master);

	while (true)
	{
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, 
									nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];
			if (sock == listening)
			{
				// Accept new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				// Add the new conn to connected clients
				FD_SET(client, &master);

				// Send welcome message to new client
				string welcomeMsg = "SERVER: Welcome to Chat Server!\r\n";
				
				// Broadcast to new connection
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
			}
			else
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Accept a new message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop Client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// send message to other clients
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];
						if (outSock == listening) continue;

						ostringstream ss;
						if (outSock != sock)
						{
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
						}
						else
						{
							ss << "ME:" << buf << "\r\n";
						}

						string strOut = ss.str();
						send(outSock, strOut.c_str(), strOut.size() + 1, 0);
					}
				}
			}
		}
	}

	// Remove listening socket from master
	// to prevent any1 else trying to connect
	FD_CLR(listening, &master);
	closesocket(listening);

	// Message to let users know we are shutting down
	string shutdownMessage = "SERVER: Server is shutting down. GoodBye\r\n";

	while (master.fd_count > 0)
	{
		// Parse all connections in set
		SOCKET sock = master.fd_array[0];
		// set message
		send(sock, shutdownMessage.c_str(), shutdownMessage.size() + 1, 0);
		// cleanup the sockets
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// Cleanup Winsock
	WSACleanup();

	return 0;
}

#pragma endregion

#pragma region DEPRECATED CODES

// Wait for a connection
	//sockaddr_in client;
	//int clientsize = sizeof(client);
	//SOCKET clientSocket = accept(sock, (sockaddr*)&client, &clientsize);
	//if (clientSocket == INVALID_SOCKET)
	//{
	//	cerr << "Connected Socket is INVALID! Quitting" << endl;
	//	return 0;
	//}

	//char host[NI_MAXHOST];		// Client's remote name
	//char service[NI_MAXHOST];	// Service (i.e. port) the client is connected on..

	//ZeroMemory(host, NI_MAXHOST);
	//ZeroMemory(service, NI_MAXHOST);

	//if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXHOST, 0) == 0)
	//{
	//	cout << "Connected on port " << service << endl;
	//}
	//else
	//{
	//	inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
	//	cout << host << "Connected on port " <<
	//		ntohs(client.sin_port) << endl;
	//}

	//// Close listening socket
	//closesocket(sock);

	//// While loop: accept and echo message to client
	//char buf[4096];

	//while (true)
	//{
	//	ZeroMemory(buf, 4096);

	//	// Wait for client to send data
	//	int bytesRecieved = recv(clientSocket, buf, 4096, 0);
	//	if (bytesRecieved == SOCKET_ERROR)
	//	{
	//		cerr << "Error in recv()! Quitting" << endl;
	//		break;
	//	}

	//	if (bytesRecieved == 0)
	//	{
	//		cout << "Client disconneted " << endl;
	//		break;
	//	}

	//	// Echo message back
	//	cout << string(buf, 0, bytesRecieved) << endl;

	//	// Echo message back to client
	//	send(clientSocket, buf, bytesRecieved + 1, 0);

	//}

#pragma endregion