#include <stdio.h>
#include <windows.h>
DWORD WINAPI print1(LPVOID lpParam)
{
	for (int i = 0;i < 1000;i++)
	{
		printf("1");
	}
	return 0;
}
DWORD WINAPI print2(LPVOID lpParam)
{
	for (int i = 0;i < 1000;i++)
	{
		printf("2");
	}
	return 0;
}
DWORD WINAPI print3(LPVOID lpParam)
{
	for (int i = 0;i < 1000;i++)
	{
		printf("3");
	}
	return 0;
}
void main(void)
{
	DWORD print1ID, print2ID, print3ID;
	HANDLE hPrint1, hPrint2, hPrint3;

	hPrint1 = CreateThread(NULL, 0, &print1, NULL, 0, &print1ID);
	hPrint2 = CreateThread(NULL, 0, &print2, NULL, 0, &print2ID);
	hPrint3 = CreateThread(NULL, 0, &print3, NULL, 0, &print3ID);
	int liI = getchar();
	CloseHandle(hPrint1);
	CloseHandle(hPrint2);
	CloseHandle(hPrint3);
}