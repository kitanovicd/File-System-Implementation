#pragma once

#include "Part.h"
#include "FS.h"
#include "KernelFS.h"

class KernelFile;

class File
{
public:
	~File();

	BytesCnt filePos();
	BytesCnt getFileSize();
	BytesCnt read(BytesCnt bytes, char *buffer);

	int f();
	char eof();
	char truncate();
	char seek(BytesCnt bytes);
	char write(BytesCnt bytes, char *buffer);

private:
	friend class FS;
	friend class KernelFS;

	KernelFile *myImpl;

	File(int *info, int levelOne, int lastCluster, KernelFS *kernel, char rw, int c);
};