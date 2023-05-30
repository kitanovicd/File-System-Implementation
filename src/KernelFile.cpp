#include <iostream>
#include <cstring>
#include "FS.h"
#include "Part.h"
#include "KernelFile.h"

using namespace std;

HANDLE KernelFile::take = CreateSemaphore(NULL, 1, 32, NULL);

KernelFile::KernelFile(int *info, int levelOne, int size, KernelFS *kernel, char rw, int c)
{
	this->rw = rw;
	this->curr = c;
	this->info = info;
	this->size = size;
	this->levelOne = levelOne;
	this->myKernel = kernel;
	this->open = true;
	this->endOfLastCluster = size % 2048;

	if (size == 0)
		this->clustersUsed = size / 2048;
	else
		this->clustersUsed = size / 2048 + 1;

	if (levelOne != 0)
	{
		char *buffer = new char[2048];
		myKernel->readCluster(this->levelOne, buffer);

		char tempStorage[4];
		memcpy(tempStorage, buffer, 4);
		int *num = (int *)tempStorage;

		myKernel->readCluster(*num, buffer);

		memcpy(tempStorage, buffer, 4);
		int *num = (int *)tempStorage;

		this->currentCluster = *num;

		int second = size / 2048;
		int first = second / 512;
		second = second % 512;

		myKernel->readCluster(this->levelOne, buffer);

		memcpy(tempStorage, buffer + first * 4, 4);
		num = (int *)tempStorage;

		myKernel->readCluster(*num, buffer);

		memcpy(tempStorage, buffer + second * 4, 4);
		num = (int *)tempStorage;

		this->lastCluster = *num;
	}

	this->content = new char[2048];
}

KernelFile::~KernelFile()
{
	rw = 0;
	curr = 0;

	char buffer[2048];
	myKernel->readCluster(info[0], buffer);

	char *size = new char[4];
	int *sizeInt = new int();
	*sizeInt = this->size;
	size = (char *)sizeInt;

	memcpy(buffer + info[1] * 32 + 16, size, 4);
	buffer[info[1] * 32 + 20] = 1 << 5;

	myKernel->writeCluster(info[0], buffer);
	myKernel->closeFile();
}

char KernelFile::write(BytesCnt bytes, char *buffer)
{
	if (!(rw & 2))
		return '0';

	BytesCnt count = 0;

	if (levelOne == 0)
	{
		levelOne = myKernel->firstFreeCluster();

		char buffer[2048];
		myKernel->readCluster(info[0], buffer);

		char *index = new char[4];
		index = (char *)(&levelOne);

		memcpy(buffer + info[1] * 32 + 12, index, 4);

		myKernel->writeCluster(info[0], buffer);
		myKernel->setV(levelOne);
		myKernel->writeCluster(levelOne, KernelFS::rootInit);

		newCluster();
	}

	while (count < bytes)
	{
		myKernel->readCluster(lastCluster, content);

		while (endOfLastCluster != 2048 && count < bytes)
		{
			content[endOfLastCluster++] = buffer[count++];
			size++;

			if (size == 512 * 512 * 2048)
				return count;
		}

		myKernel->writeCluster(lastCluster, content);

		if (count < bytes)
		{
			newCluster();
		}
	}

	curr = size;

	char buffer[2048];
	myKernel->readCluster(info[0], buffer);

	char *newSize = new char[4];
	newSize = (char *)(&size);

	memcpy(buffer + info[1] * 32 + 16, newSize, 4);

	myKernel->writeCluster(info[0], buffer);

	return '1';
}

BytesCnt KernelFile::read(BytesCnt bytes, char *buffer)
{

	if (!(rw & 1))
		return 0;

	if (curr == size)
		return 0;

	int count = 0;
	int position = curr % 2048;

	if ((curr != 0) && (curr / 2048 > 0) && (curr % 2048 == 0))
	{
		nextCluster();
	}

	while (count < bytes)
	{
		myKernel->readCluster(currentCluster, content);

		while (position != 2048 && count < bytes)
		{
			buffer[count++] = content[position++];
			curr++;

			if (curr == size)
				return count;
		}

		if (count < bytes)
		{
			nextCluster();
		}

		position = 0;
	}

	return count;
}

char KernelFile::seek(BytesCnt bytes)
{
	if (!(rw & 3))
		return '0';

	if (bytes > size)
		return '0';

	curr = bytes;

	int clusterNo = curr / 2048;
	int entry = clusterNo / 512;

	myKernel->readCluster(levelOne, content);

	char *tempStorage = new char[4];

	memcpy(tempStorage, content + entry * 4, 4);

	int *num = (int *)tempStorage;
	entry = clusterNo % 512;

	myKernel->readCluster(*num, content);

	memcpy(tempStorage, content + entry * 4, 4);

	num = (int *)tempStorage;
	currentCluster = *num;

	return '1';
}

BytesCnt KernelFile::filePos()
{
	if (!(rw & 3))
		return 0;

	return curr;
}

char KernelFile::eof()
{
	if (!(rw & 3))
		return 1;

	if (curr == this->size)
	{
		return 2;
	}

	return 0;
}

BytesCnt KernelFile::getFileSize()
{
	if (!(rw & 3))
		return 0;

	return this->size;
}

char KernelFile::truncate()
{
	if (!(rw & 2))
		return '0';

	if (size == curr)
		return '0';

	if (size == 0)
		return '1';

	int clusterNo;
	if (curr % 2048 == 0)
		clusterNo = curr / 2048;
	else
		clusterNo = curr / 2048 + 1;

	int entry2 = clusterNo % 512;
	int entry1 = clusterNo / 512;

	myKernel->freeClusters(levelOne, entry1, entry2, size - curr);
	size = curr;

	return '1';
}

void KernelFile::newCluster()
{
	wait(take);

	clustersUsed++;
	int entry = (clustersUsed - 1) / 512;

	char index[2048];
	myKernel->readCluster(levelOne, index);

	int *value;
	char *tempStorage = new char[4];

	memcpy(tempStorage, index + entry * 4, 4);

	value = (int *)tempStorage;

	if (*value == -1)
	{
		int *free = new int();
		*free = myKernel->firstFreeCluster();
		myKernel->setV(*free);

		tempStorage = (char *)free;

		memcpy(index + entry * 4, tempStorage, 4);

		myKernel->writeCluster(levelOne, index);
		*value = *free;
	}

	if (clustersUsed % 512 == 0)
		entry = (clustersUsed - 1) % 512;
	else
		entry = (clustersUsed % 512 - 1);

	myKernel->readCluster(*value, index);

	int *free = new int();
	*free = myKernel->firstFreeCluster();

	myKernel->setV(*free);

	tempStorage = (char *)free;
	memcpy(index + entry * 4, tempStorage, 4);

	myKernel->writeCluster(*value, index);
	endOfLastCluster = 0;

	lastCluster = *free;

	if (currentCluster == 0)
		currentCluster = lastCluster;

	signal(take);
}

void KernelFile::nextCluster()
{

	int clusterNo = curr / 2048;
	int index = clusterNo % 512;
	int index = clusterNo / 512;

	char buffer[2048];
	myKernel->readCluster(levelOne, buffer);

	int *num;
	char tempStorage[4];

	memcpy(tempStorage, buffer + index * 4, 4);
	num = (int *)tempStorage;

	myKernel->readCluster(*num, buffer);

	memcpy(tempStorage, buffer + index * 4, 4);
	num = (int *)tempStorage;
	currentCluster = *num;
}
