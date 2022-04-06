#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <map>
#include <set>
#include "MSGPacket.h"
#include "PlayerData.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;


int main()
{
	map<SOCKET, PlayerData> ConnectedPlayer;//Session

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

	Timeout.tv_sec = 1;
	Timeout.tv_usec = 0;

	FD_ZERO(&Reads);
	FD_ZERO(&CopyReads);

	FD_SET(ServerSocket, &Reads);

	char Buffer[2049] = { 0, };
	char Header[2] = { 0, };
	char SendData[1024] = { 0, };


	while (1)
	{
		CopyReads = Reads;

		int ChangeFDNumber = select(0, &CopyReads, 0, 0, &Timeout);

		cout << "fd_count : " << Reads.fd_count << endl;

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
					if (Reads.fd_array[i] != ServerSocket)
					{
						int RecvLength = recv(Reads.fd_array[i], Header, 2, 0);
						if (RecvLength <= 0)
						{
							//close
							ConnectedPlayer.erase(Reads.fd_array[i]);

							cout << "Disconnect : " << GetLastError() << endl;
							closesocket(Reads.fd_array[i]);
							FD_CLR(Reads.fd_array[i], &Reads);
						}
						else
						{
							if ((UINT8)Header[0] == (UINT8)MSGPacket::Login)
							{
								//packet parsing
								char Data[3] = { 0, };
								int RecvLength = recv(Reads.fd_array[i], Data, 3, 0);

								//save session 
								PlayerData NewPlayerData;

								NewPlayerData.ClientSocket = Reads.fd_array[i];
								NewPlayerData.R = Data[0];
								NewPlayerData.G = Data[1];
								NewPlayerData.B = Data[2];
								NewPlayerData.X = 0;
								NewPlayerData.Y = 0;
								ConnectedPlayer[NewPlayerData.ClientSocket] = NewPlayerData;

								//reponse
								SendData[0] = (UINT8)MSGPacket::LoginAck;
								SendData[1] = (UINT8)4;
								memcpy(&SendData[2], &Reads.fd_array[i], 4);
								send(Reads.fd_array[i], SendData, 6, 0);

								cout << "save Session : " << Reads.fd_array[i] << endl;

								//이미 접속된 유저 리스트
								for (auto AlreadyConnectedPlayer : ConnectedPlayer)
								{
									cout << "AlreadyConnectedPlayer 1" << endl;
									//새로 접속한 아이
									if (AlreadyConnectedPlayer.second.ClientSocket == Reads.fd_array[i])
									{
										continue;
									}

									//새로 접속한 클라이언트 정보를 이미 접속한 클라이언트한테 전송
									SendData[0] = (UINT8)MSGPacket::MakePlayer;
									SendData[1] = (UINT8)15;
									SendData[2] = (UINT8)NewPlayerData.R;
									SendData[3] = (UINT8)NewPlayerData.G;
									SendData[4] = (UINT8)NewPlayerData.B;
									memcpy(&SendData[5], &(NewPlayerData.X), 4);
									memcpy(&SendData[9], &(NewPlayerData.Y), 4);
									memcpy(&SendData[13], &(NewPlayerData.ClientSocket), 4);

									send(AlreadyConnectedPlayer.second.ClientSocket, SendData, 15, 0);
								}

								//새로 접속한 클라이언트한테 모든 클라이언트 정보 보내기
								for (auto AlreadyConnectedPlayer : ConnectedPlayer)
								{
									cout << "AlreadyConnectedPlayer 2" << endl;

									//새로 접속한 아이
									if (AlreadyConnectedPlayer.second.ClientSocket == Reads.fd_array[i])
									{
										continue;
									}

									//새로 접속한 클라이언트 이미 접속한 클라이언트 정보 보내기
									SendData[0] = (UINT8)MSGPacket::MakePlayer;
									SendData[1] = (UINT8)15;
									SendData[2] = (UINT8)AlreadyConnectedPlayer.second.R;
									SendData[3] = (UINT8)AlreadyConnectedPlayer.second.G;
									SendData[4] = (UINT8)AlreadyConnectedPlayer.second.B;
									memcpy(&SendData[5], &(AlreadyConnectedPlayer.second.X), 4);
									memcpy(&SendData[9], &(AlreadyConnectedPlayer.second.Y), 4);
									memcpy(&SendData[13], &(AlreadyConnectedPlayer.second.ClientSocket), 4);

									send(NewPlayerData.ClientSocket, SendData, 15, 0);
								}

								cout << "Complete MakePlayer" << endl;
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


	WSACleanup();

	return 0;
}