#include <iostream>
#include <string>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{

	string SVR_IP_ADDRESS = "127.0.0.1";
	int constexpr SVR_PORT = 54000;

	// Init WinSocket
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can't start winsock, Err#: " << wsResult << endl;
		return -1;
	}

	// Create Socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err#: " << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	// Fill hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(SVR_PORT);
	inet_pton(AF_INET, SVR_IP_ADDRESS.c_str(), &hint.sin_addr);

	// Connect to Server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to Server, Err#: " << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	// Do-While Loop to send recieve data
	char buf[4096];
	string userInput;

	do
	{
		// Prompt the user for some text
		cout << "> ";
		getline(cin, userInput);

		if (userInput.size() > 0)
		{
			// Send the text
			int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
			if (sendResult != SOCKET_ERROR)
			{
				// Wait for client to send data
				ZeroMemory(buf, 4096);
				int bytesRecieved = recv(sock, buf, 4096, 0);
				if (bytesRecieved > 0)
				{
					// Echo message back to console
					cout << "SERVER> " << string(buf, 0, bytesRecieved) << endl;
				}
			}
		}
	} while (userInput.size() > 0);

	// Close socket
	closesocket(sock);

	// Cleanup
	WSACleanup();

	return 0;
}