#include <iostream>
#include <cstring>
#include <vector>
#include <bitset>
#include "Def.h"
#include "File.h"
#include "KernelFS.h"

using namespace std;

char KernelFS::validInit[2048];
char KernelFS::rootInit[2048];
const char KernelFS::allOnes = 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128;

KernelFS::KernelFS()
{
	this->part = nullptr;
	this->mountW = 0;
	this->canOpen = true;
	this->numberOfOpenFiles = 0;

	this->unmountSem = CreateSemaphore(NULL, 0, 32, NULL);
	this->closedSem = CreateSemaphore(NULL, 0, 32, NULL);
	this->mutex1 = CreateSemaphore(NULL, 1, 32, NULL);

	memset(validInit, allOnes, 2048);
	validInit[0] = 1 | 2 | 4 | 8 | 16 | 32;
	memset(rootInit, -1, 2048);
}

char KernelFS::mount(Partition *partition)
{
	if (part)
	{
		mountW++;
		wait(unmountSem);
		mountW--;
	}

	part = partition;

	return '1';
}

char KernelFS::unmount()
{
	if (part == nullptr)
		return '0';

	if (numberOfOpenFiles)
		wait(closedSem);

	part = nullptr;

	if (mountW > 0)
		signal(unmountSem);

	return '1';
}

char KernelFS::format()
{
	if (part == nullptr)
		return '0';

	if (numberOfOpenFiles)
		wait(closedSem);

	part->writeCluster(0, validInit);

	for (int i = 1; i < 10000; i++)
		part->writeCluster(i, rootInit);

	return '1';
}

FileCnt KernelFS::readRootDir()
{
	if (part == nullptr)
		return -1;

	return numberOfFiiles;
}

char KernelFS::doesExist(char *fname)
{
	if (part == nullptr)
		return '0';

	int *positions = filePosition(fname);

	if (positions == nullptr)
		return '0';

	return '1';
}

File *KernelFS::open(char *fname, char mode)
{

	if (part == nullptr)
		return 0;

	if (mode == 'r' || mode == 'a')
	{
		if (numberOfFiiles == 0)
			return nullptr;

		int *position = filePosition(fname);

		if (position == nullptr)
			return nullptr;

		char buffer[2048];
		part->readCluster(position[0], buffer);

		char *index = new char[4];
		memcpy(index, buffer + position[1] * 32 + 12, 4);

		char *size = new char[4];
		memcpy(size, buffer + position[1] * 32 + 16, 4);

		char open = buffer[position[1] * 32 + 20];

		if (mode == 'r')
		{
			while (open & (1 << 1))
			{
				wait(mutex1);

				part->readCluster(position[0], buffer);
				open = buffer[position[1] * 32 + 20];

				if (open & (1 << 1))
					signal(mutex1);
			}

			memcpy(index, buffer + position[1] * 32 + 12, 4);
			memcpy(size, buffer + position[1] * 32 + 16, 4);

			buffer[position[1] * 32 + 20] |= 1;
			part->writeCluster(position[0], buffer);
			numberOfOpenFiles++;

			return new File(position, *(int *)index, *(int *)size, this, buffer[position[1] * 32 + 20], 0);
		}

		if (mode == 'a')
		{
			while (open & ((1 << 1) | 1))
			{
				wait(mutex1);

				part->readCluster(position[0], buffer);

				char open = buffer[position[1] * 32 + 20];

				if (open & ((1 << 1) | 1))
					signal(mutex1);
			}

			memcpy(index, buffer + position[1] * 32 + 12, 4);
			memcpy(size, buffer + position[1] * 32 + 16, 4);

			buffer[position[1] * 32 + 20] |= 3;
			part->writeCluster(position[0], buffer);
			numberOfOpenFiles++;

			return new File(position, *(int *)index, *(int *)size, this, buffer[position[1] * 32 + 20], *(int *)size); // Ovde se vraca dobra vrednost
		}
	}
	if (mode == 'w')
	{
		int *position = filePosition(fname);

		if (position != nullptr)
		{
			char buffer[2048];
			part->readCluster(position[0], buffer);
			char open = buffer[position[1] * 32 + 20];

			while (open & ((1 << 1) | 1))
			{
				wait(mutex1);

				part->readCluster(position[0], buffer);

				char open = buffer[position[1] * 32 + 20];

				if (open & ((1 << 1) | 1))
					signal(mutex1);
			}

			deleteFile(fname);

			this->open(fname, mode);
		}

		if (position == nullptr)
		{
			position = findFreeSpace();
			setV(position[0]);

			char buffer[2048];
			part->readCluster(position[0], buffer);

			int curr_position = position[1] * 32;
			int i = 0;

			while (fname[i] != '.')
			{
				buffer[curr_position] = fname[i];
				i++;
				curr_position++;
			}

			int j = i;

			while (j < 8)
			{
				buffer[curr_position++] = ' ';
				j++;
			}

			i++;
			j = 0;

			while (fname[i] && j < 3)
			{
				buffer[curr_position++] = fname[i++];
				j++;
			}

			while (j < 3)
			{
				buffer[curr_position++] = ' ';
				i++;
			}

			buffer[curr_position++] = '0';

			char *size = new char[4];
			int *tempStorage = new int();
			*tempStorage = 0;
			size = (char *)tempStorage;

			buffer[curr_position++] = 0;
			buffer[curr_position++] = 0;
			buffer[curr_position++] = 0;
			buffer[curr_position++] = 0;

			buffer[curr_position++] = 0;
			buffer[curr_position++] = 0;
			buffer[curr_position++] = 0;
			buffer[curr_position++] = 0;

			buffer[curr_position++] = 3;
			buffer[curr_position] = 'Y';

			numberOfFiiles++;
			numberOfOpenFiles++;

			part->writeCluster(position[0], buffer);

			return new File(position, 0, 0, this, buffer[position[1] * 32 + 20], 0);
		}
	}
}

char KernelFS::deleteFile(char *fname)
{
	int count = numberOfFiiles;

	if (part == nullptr)
		return '0';

	char *buffer = new char[2048];

	part->readCluster(1, buffer);

	char *tempStorage = new char[4];
	memcpy(tempStorage, buffer, 4);
	int number = *((int *)tempStorage);

	int pos = 0;
	int step = 4;
	int newPosition = 32;

	char size[4];
	char index[4];

	while (number != -1)
	{
		char *newBuffer = new char[2048];
		part->readCluster(number, newBuffer);

		for (int i = 0; i < 64; i++)
		{
			char *name = new char[12];
			int j = 0;

			if (newBuffer[pos + 21] != 'Y')
			{
				pos = newPosition;
				newPosition += 32;
				continue;
			}

			while ((newBuffer[pos] != ' ') && (j < 8))
			{
				name[j++] = newBuffer[pos++];
			}

			pos += (8 - j);
			name[j++] = '.';

			int more = 0;

			while (newBuffer[pos] != ' ' && more < 3)
			{
				name[j + more] = newBuffer[pos++];
				more++;
			}

			name[j + more] = '\0';

			if (strcmp(name, fname) == 0)
			{
				int oldPosition = pos - (pos % 32);
				int pos = oldPosition + 20;

				char open = newBuffer[pos];
				char mask = (1 | (1 << 1));
				if (open & mask)
					return '0';

				newBuffer[oldPosition + 21] = 'X';
				part->writeCluster(number, newBuffer);

				numberOfFiiles--;

				memcpy(index, newBuffer + oldPosition + 12, 4);
				memcpy(size, newBuffer + oldPosition + 16, 4);

				freeClusters(*(int *)index, 0, 0, *(int *)size);

				return '1';
			}

			if (--count == 0)
				return '0';

			pos = newPosition;
			newPosition += 32;
		}

		memcpy(num, buffer + step, 4);

		number = *((int *)num);
		step += 4;
	}

	return '0';
}

bool KernelFS::getValid(int clusterNo)
{
	char buffer[2048];
	part->readCluster(0, buffer);

	int byte = clusterNo / 8;
	int num = clusterNo % 8;
	char number = buffer[byte];
	char mask = (1 << (7 - num));

	return (number & mask);
}

int KernelFS::firstFreeCluster()
{
	int number_of_clusters = part->getNumOfClusters();

	for (int i = 0; i < number_of_clusters; i++)
	{
		if (getValid(i))
			return i;
	}
}

int *KernelFS::filePosition(char *fname)
{
	int cnt = numberOfFiiles;
	int *result = new int[2];

	char *buffer = new char[2048];
	part->readCluster(1, buffer);

	char *tempStorage = new char[4];
	memcpy(tempStorage, buffer, 4);
	int number = *((int *)tempStorage);

	int pos = 0;
	int step = 4;
	int newPosition = 32;
	char newBuffer[2048];

	while (number != -1)
	{
		part->readCluster(number, newBuffer);

		for (int i = 0; i < 64; i++)
		{
			char *name = new char[12];
			int j = 0;

			char c = newBuffer[pos + 21];

			if (c != 'Y')
			{
				pos = newPosition;
				newPosition += 32;

				continue;
			}

			while ((newBuffer[pos] != ' ') && (j < 8))
			{
				name[j++] = newBuffer[pos++];
			}

			pos += (8 - j);
			name[j++] = '.';
			int more = 0;

			while (newBuffer[pos] != ' ' && more < 3)
			{
				name[j + more] = newBuffer[pos++];
				more++;
			}

			name[j + more] = '\0';

			if (strcmp(name, fname) == 0)
			{
				result[0] = number;
				result[1] = pos / 32;
				return result;
			}

			if (--cnt == 0)
				return nullptr;

			pos = newPosition;
			newPosition += 32;
		}

		memcpy(num, buffer + step, 4);

		number = *((int *)num);
		step += 4;
	}

	return nullptr;
}

int *KernelFS::findFreeSpace()
{
	char buffer[2048];
	part->readCluster(1, buffer);

	int *position = new int[2];

	if (numberOfFiiles % 64 == 0)
	{
		position[0] = firstFreeCluster();

		int entry = numberOfFiiles / 64;

		int *tempStorage = new int();
		*tempStorage = position[0];

		char *value = new char[4];
		value = (char *)tempStorage;

		memcpy(buffer + entry * 4, value, 4);

		part->writeCluster(1, buffer);
	}
	else
	{

		int entry = numberOfFiiles / 64;

		int *value;
		char *tempStorage = new char[4];

		memcpy(tempStorage, buffer + entry * 4, 4);
		value = (int *)tempStorage;

		position[0] = *value;
	}

	part->readCluster(position[0], buffer);

	int i = 21;

	while (buffer[i] == 'Y')
		i += 32;

	position[1] = i / 32;

	return position;
}

void KernelFS::readCluster(int ClusterNo, char *buffer)
{
	part->readCluster(ClusterNo, buffer);
}

void KernelFS::writeCluster(int ClusterNo, char *buffer)
{
	part->writeCluster(ClusterNo, buffer);
}

void KernelFS::freeClusters(int index, int entry1, int entry2, int size)
{

	char buffer[2048];
	part->readCluster(index, buffer);

	if (index == 0)
		return;

	char tempStorage[4];
	memcpy(tempStorage, buffer + entry1, 4);
	int *num = (int *)tempStorage;

	int step = entry1 + 4;
	char buffer1[2048];

	while (size > 0)
	{
		int position = 0;

		if (step == entry1 + 4)
			position = entry2 * 4;

		int oldPositionition = position;

		while (position < 2048)
		{
			part->readCluster(*num, buffer1);

			char temp[4];
			memcpy(temp, buffer1 + position, 4);
			int *num = (int *)temp;

			freeCluster(*num);

			size -= 2048;

			if (size <= 0)
			{
				if (entry1 == 0 && entry2 == 0)
					freeCluster(index);

				return;
			}
		}

		if (oldPositionition == 0)
			freeCluster(*num);

		memcpy(tempStorage, buffer + step, 4);
		int *num = (int *)tempStorage;
	}

	exit(1);

	if (entry1 == 0 && entry2 == 0)
		freeCluster(index);
}

void KernelFS::setV(int i)
{
	char buffer[2048];
	part->readCluster(0, buffer);

	char mask = ~(1 << (7 - (i % 8)));

	int byte = i / 8;
	buffer[byte] = buffer[byte] & mask;

	part->writeCluster(0, buffer);
}

void KernelFS::closeFile()
{
	numberOfOpenFiles--;
	signal(mutex1);

	if (numberOfOpenFiles == 0)
	{
		signal(closedSem);
	}
}

void KernelFS::freeCluster(int ClusterNo)
{
	char buffer[2048];
	part->readCluster(0, buffer);

	int i = ClusterNo;
	int byte = i / 8;

	char mask = (1 << (7 - (i % 8)));
	buffer[byte] = buffer[byte] | mask;

	part->writeCluster(0, buffer);
}