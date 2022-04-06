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
};
