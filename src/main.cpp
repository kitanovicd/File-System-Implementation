#include <iostream>
#include "Test.h"

using namespace std;

HANDLE thread1, thread2;
DWORD ThreadID;

HANDLE mainSemaphore = CreateSemaphore(NULL, 0, 32, NULL);
HANDLE semaphore1 = CreateSemaphore(NULL, 0, 32, NULL);
HANDLE semaphore2 = CreateSemaphore(NULL, 0, 32, NULL);
HANDLE mutex = CreateSemaphore(NULL, 1, 32, NULL);

Partition *partition;

int entrySize;
char *entryBuffer;

int main()
{
	clock_t startTime, endTime;
	startTime = clock();

	{
		FILE *f = fopen("ulaz.dat", "rb");

		if (f == 0)
		{
			system("PAUSE");
			return 0; // exit program
		}

		entryBuffer = new char[32 * 1024 * 1024]; // 32MB
		entrySize = fread(entryBuffer, 1, 32 * 1024 * 1024, f);

		fclose(f);
	}

	thread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread1run, NULL, 0, &ThreadID);
	thread2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread2run, NULL, 0, &ThreadID);

	for (int i = 0; i < 2; i++)
		wait(mainSemaphore);

	delete[] entryBuffer;

	endTime = clock();

	CloseHandle(mutex);
	CloseHandle(mainSemaphore);
	CloseHandle(semaphore1);
	CloseHandle(semaphore2);
	CloseHandle(thread1);
	CloseHandle(thread2);

	return 0;
}