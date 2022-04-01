#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include "MSGPacket.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ServerSocket;
	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN ServerAddr;
	memset(&ServerAddr, 0, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = PF_INET;
	ServerAddr.sin_addr.s_addr = INADDR_ANY;
	ServerAddr.sin_port = htons(3000);

	bind(ServerSocket, (SOCKADDR*)(&ServerAddr), sizeof(ServerAddr));

	listen(ServerSocket, 0);

	fd_set Reads;
	fd_set CopyReads;

	TIMEVAL Timeout;

	Timeout.tv_sec = 0;
	Timeout.tv_usec = 500;

	FD_ZERO(&Reads);
	FD_ZERO(&CopyReads);

	FD_SET(ServerSocket, &Reads);

	char Buffer[2049] = { 0, };
	char Header[2] = { 0, };

	while (1)
	{
		CopyReads = Reads;

		int ChangeFDNumber = select(0, &CopyReads, 0, 0, &Timeout);

		if (ChangeFDNumber == 0)
		{
			//not connect
			//other process
			continue;
		}

		if (ChangeFDNumber < 0)
		{
			cout << "Error : " << GetLastError() << endl;
			exit(-1);
		}

		for (int i = 0; i < (int)Reads.fd_count; ++i)
		{
			if (FD_ISSET(Reads.fd_array[i], &CopyReads))
			{
				if (Reads.fd_array[i] == ServerSocket)
				{
					//client Connect;
					SOCKADDR_IN ClientAddr;
					memset(&ClientAddr, 0, sizeof(SOCKADDR_IN));
					int ClientAddrSize = sizeof(ClientAddr);
					SOCKET ClientSocket = accept(Reads.fd_array[i], (SOCKADDR*)(&ClientAddr), &ClientAddrSize);

					cout << "Connected Client : " << ClientSocket << endl;

					FD_SET(ClientSocket, &Reads);
				}
				else //client prcess
				{
					for (int ClientIndex = 0; ClientIndex < (int)Reads.fd_count; ++ClientIndex)
					{
						if (Reads.fd_array[ClientIndex] != ServerSocket)
						{
							int RecvLength = recv(Reads.fd_array[i], Header, 2, 0);
							if (RecvLength <= 0)
							{
								//close
								cout << GetLastError() << endl;
							}
							else
							{
								if ((UINT8)Header[0] == (UINT8)MSGPacket::Login)
								{
									char Data[3] = { 0, };
									int RecvLength = recv(Reads.fd_array[i], Data, 3, 0);
									cout << "RecvLength : " << RecvLength << endl;
									//save player rgb 
									cout << (UINT8)Data[0] << endl;
									cout << (UINT8)Data[1] << endl;
									cout << (UINT8)Data[2] << endl;
								}
								else if ((UINT8)Header[0] == (UINT8)MSGPacket::MakePlayer)
								{

								}
								else if ((UINT8)Header[0] == (UINT8)MSGPacket::MovePlayer)
								{

								}
							}

						}
					}
				}
			}
		}
	}


	WSACleanup();

	return 0;
}