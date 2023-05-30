#include "Test.h"

static char threadName[] = "thread1";

DWORD WINAPI thread1run()
{
	wait(mutex);
	partition = new Partition((char *)"p1.ini");
	signal(mutex);

	wait(mutex);
	cout << threadName << ":Partition created" << endl;
	signal(mutex);

	FS::mount(partition);

	wait(mutex);
	cout << threadName << ": Partition mounted" << endl;
	signal(mutex);

	FS::format();

	wait(mutex);
	cout << threadName << ": Partition formated" << endl;
	signal(mutex);

	signal(semaphore1);

	wait(mutex);
	cout << threadName << ": wait 1" << endl;
	signal(mutex);

	wait(semaphore2);

	{
		char filepath[] = "/fajl1.dat";
		File *f = FS::open(filepath, 'w');

		wait(mutex);
		cout << threadName << ": File created '" << filepath << "'" << endl;
		signal(mutex);

		f->write(entrySize, entryBuffer);

		wait(mutex);
		cout << threadName << ": Copied content from 'ulaz.dat' in '" << filepath << "'" << endl;
		signal(mutex);

		delete f;

		wait(mutex);
		cout << threadName << ": Closed file '" << filepath << "'" << endl;
		signal(mutex);
	}

	{
		File *src, *dst;

		char filepath[] = "/fajl1.dat";

		src = FS::open(filepath, 'r');
		src->seek(src->getFileSize() / 2); // pozicionira se na pola fajla

		wait(mutex);
		cout << threadName << ": Otvoren fajl '" << filepath << "' i pozicionirani smo na polovini" << endl;
		signal(mutex);

		char filepath1[] = "/fajll5.dat";
		dst = FS::open(filepath1, 'w');

		wait(mutex);
		cout << threadName << ": Otvoren fajl '" << filepath1 << "'" << endl;
		signal(mutex);

		char c;
		while (!src->eof())
		{
			src->read(1, &c);
			dst->write(1, &c);
		}

		wait(mutex);
		cout << threadName << ": Copied second part of '" << filepath << "' in '" << filepath1 << "'" << endl;
		signal(mutex);

		delete dst;

		wait(mutex);
		cout << threadName << ": Closed file '" << filepath1 << "'" << endl;
		signal(mutex);

		delete src;

		wait(mutex);
		cout << threadName << ": Closed file '" << filepath << "'" << endl;
		signal(mutex);
	}

	signal(semaphore1);

	wait(mutex);
	cout << threadName << ": wait 2" << endl;
	signal(mutex);

	wait(semaphore2); // ceka thread1

	{
		File *src, *dst;
		char filepath[] = "/fajl25.dat";
		dst = FS::open(filepath, 'a');

		wait(mutex);
		cout << threadName << ": File opened '" << filepath << "'" << endl;
		signal(mutex);

		char filepath1[] = "/fajll5.dat";
		src = FS::open(filepath1, 'r');

		wait(mutex);
		cout << threadName << ": File opened '" << filepath1 << "'" << endl;
		signal(mutex);

		char c;

		while (!src->eof())
		{
			src->read(1, &c);
			dst->write(1, &c);
		}

		wait(mutex);
		cout << threadName << ": Copied second part from '" << filepath << "' in '" << filepath1 << "'" << endl;
		signal(mutex);

		delete dst;

		wait(mutex);
		cout << threadName << ": Closed file '" << filepath1 << "'" << endl;
		signal(mutex);

		delete src;

		wait(mutex);
		cout << threadName << ": Closed file '" << filepath << "'" << endl;
		signal(mutex);
	}

	signal(semaphore1);

	wait(mutex);
	cout << threadName << ": Done!" << endl;
	signal(mutex);

	signal(mainSemaphore);

	return 0;
}