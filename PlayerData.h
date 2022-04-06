#pragma once
#include <Windows.h>

class PlayerData
{
public:
	UINT8 R;
	UINT8 G;
	UINT8 B;
	int X;
	int Y;
	SOCKET ClientSocket;

	//proto buffer, flat buffer
	void MakeData(char* Data) //Deserialze(역직렬화), json -> Data
	{
		R = (UINT8)Data[0];
		G = (UINT8)Data[1];
		B = (UINT8)Data[2];
		X = (UINT32)Data[3];
		Y = (UINT32)Data[7];
		ClientSocket = (UINT32)Data[11];
	}

	void MakePacket(char* Packet) //Serialize(직렬화), Data -> json
	{
		Packet[2] = (UINT8)R;
		Packet[3] = (UINT8)G;
		Packet[4] = (UINT8)B;
		memcpy(&Packet[5], &(X), 4);
		memcpy(&Packet[9], &(Y), 4);
		memcpy(&Packet[13], &(ClientSocket), 4);
	}
};
