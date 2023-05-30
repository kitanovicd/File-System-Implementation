#pragma once

#include "File.h"
#include "FS.h"
#include "KernelFS.h"
#include "KernelFile.h"

class KernelFile
{
private:
	bool open;
	char rw;
	char *content;
	int *info;
	int levelOne;
	int clustersUsed;
	int endOfLastCluster;
	int lastCluster;
	int currentCluster;

	static HANDLE take;

	BytesCnt size;
	BytesCnt curr;
	KernelFS *myKernel;

public:
	KernelFile(int *info, int levelOne, int size, KernelFS *kernel, char rw, int c);
	~KernelFile();

	BytesCnt filePos();
	BytesCnt getFileSize();

	char eof();
	char truncate();
	void newCluster();
	void nextCluster();

	BytesCnt read(BytesCnt bytes, char *buffer);
	char seek(BytesCnt bytes);
	char write(BytesCnt bytes, char *buffer);
};