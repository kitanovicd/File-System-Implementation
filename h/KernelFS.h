#pragma once

#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <windows.h>
#include "Part.h"
#include "FS.h"
#include "Def.h"
#include "KernelFile.h"

using namespace std;

class FS;

class KernelFS
{

public:
	KernelFS();

	FileCnt readRootDir();

	char mount(Partition *partition);
	char unmount();
	char format();

	char doesExist(char *fname);
	char deleteFile(char *fname);
	void closeFile();
	File *open(char *fname, char mode);

	bool getValid(int clusterNo);
	int firstFreeCluster();
	int *filePosition(char *fname);
	int *findFreeSpace();

	void readCluster(int ClusterNo, char *buffer);
	void writeCluster(int ClusterNo, char *buffer);
	void freeClusters(int index, int entry1, int entry2, int size);
	void freeCluster(int ClusterNo);

	void setV(int i);

private:
	friend class KernelFile;

	HANDLE unmountSem;
	HANDLE closedSem;
	HANDLE mutex1;
	HANDLE fileClosed;
	Partition *part;

	bool canOpen;
	int mountW;
	int numberOfFiiles;
	int numberOfOpenFiles;

	static const char allOnes;
	static char validInit[2048];
	static char rootInit[2048];
};