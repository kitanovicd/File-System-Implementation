#include <iostream>
#include "FS.h"
#include "File.h"
#include "KernelFile.h"

using namespace std;

File::File(int *info, int levelOne, int lastCluster, KernelFS *kernel, char rw, int c)
{
	myImpl = new KernelFile(info, levelOne, lastCluster, kernel, rw, c);
}

char File::write(BytesCnt bytes, char *buffer)
{
	return myImpl->write(bytes, buffer);
}

BytesCnt File::read(BytesCnt bytes, char *buffer)
{
	return myImpl->read(bytes, buffer);
}

char File::seek(BytesCnt bytes)
{
	return myImpl->seek(bytes);
}

BytesCnt File::filePos()
{
	return myImpl->filePos();
}

char File::eof()
{
	return myImpl->eof();
}

BytesCnt File::getFileSize()
{
	return myImpl->getFileSize();
}

char File::truncate()
{
	return myImpl->truncate();
}

int File::f()
{
	return myImpl->f();
}

File::~File()
{
	delete myImpl;
}