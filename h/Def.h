#pragma once

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

#define signal(x) ReleaseSemaphore(x, 1, NULL)
#define wait(x) WaitForSingleObject(x, INFINITE)
