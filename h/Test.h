#pragma once

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <windows.h>
#include <ctime>

#include "FS.h"
#include "File.h"
#include "Part.h"

#define signal(x) ReleaseSemaphore(x, 1, NULL)
#define wait(x) WaitForSingleObject(x, INFINITE)

using namespace std;

extern HANDLE thread1, thread2;
extern DWORD ThreadID;

extern HANDLE mutex;
extern HANDLE mainSemaphore;
extern HANDLE semaphore1;
extern HANDLE semaphore2;

extern Partition *partition;

extern char *entryBuffer;
extern int entrySize;

DWORD WINAPI thread1run();
DWORD WINAPI thread2run();