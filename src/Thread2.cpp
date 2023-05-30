#include "Test.h"

static char threadName[] = "thread2";

DWORD WINAPI thread2run()
{
	wait(semaphore1);
	signal(semaphore2);

	{
		File *src, *dst;
		char filepath[] = "/fajl1.dat";

		while ((src = FS::open(filepath, 'r')) == 0)
		{
			wait(mutex);
			cout << threadName << ":File not open: '" << filepath << "'" << endl;
			signal(mutex);

			Sleep(1);
		}

		wait(mutex);
		cout << threadName << ": File opened '" << filepath << "'" << endl;
		signal(mutex);

		char filepath1[] = "/fajl2.dat";
		dst = FS::open(filepath1, 'w');

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
		cout << threadName << ": Copied file from '" << filepath << "' to '" << filepath1 << "'" << endl;
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

	wait(mutex);
	cout << threadName << ": wait 1" << endl;
	signal(mutex);

	wait(semaphore1); // ceka nit 1

	{
		wait(mutex);
		cout << threadName << ": Number of files on disk is " << FS::readRootDir() << endl;
		signal(mutex);
	}

	{
		char filepath[] = "/fajl2.dat";
		File *f = FS::open(filepath, 'r');

		wait(mutex);
		cout << threadName << ": File opened " << filepath << "" << endl;
		signal(mutex);

		delete f;

		wait(mutex);
		cout << threadName << ": File closed " << filepath << "" << endl;
		signal(mutex);
	}

	{
		char filepath[] = "/fajl2.dat";
		File *f = FS::open(filepath, 'r');

		wait(mutex);
		cout << threadName << ": File opened " << filepath << "" << endl;
		signal(mutex);

		ofstream fout("izlaz1.dat", ios::out | ios::binary);
		char *buff = new char[f->getFileSize()];

		f->read(f->getFileSize(), buff);
		fout.write(buff, f->getFileSize());

		wait(mutex);
		cout << threadName << ": Written '" << filepath << "' in file 'izlaz1.dat'" << endl;
		signal(mutex);

		delete[] buff;

		fout.close();

		delete f;

		wait(mutex);
		cout << threadName << ": Closed file " << filepath << "" << endl;
		signal(mutex);
	}

	{
		char copied_filepath[] = "/fajll5.dat";
		File *copy = FS::open(copied_filepath, 'r');

		BytesCnt size = copy->getFileSize();

		wait(mutex);
		cout << threadName << ": Opened file '" << copied_filepath << endl;
		signal(mutex);

		delete copy;

		wait(mutex);
		cout << threadName << ": File closed '" << copied_filepath << "'" << endl;
		signal(mutex);

		char filepath[] = "/fajl1.dat";

		File *src, *dst;
		src = FS::open(filepath, 'r');
		src->seek(0);

		wait(mutex);
		cout << threadName << ": Opened file '" << filepath << "' and we are on the middle of it" << endl;
		signal(mutex);

		char filepath1[] = "/fajl25.dat";
		dst = FS::open(filepath1, 'w');

		wait(mutex);
		cout << threadName << ": Opened file '" << filepath1 << "'" << endl;
		signal(mutex);

		char c;
		BytesCnt cnt = src->getFileSize() - size;

		while (!src->eof() && cnt-- > 0)
		{
			src->read(1, &c);
			dst->write(1, &c);
		}

		wait(mutex);
		cout << threadName << ": Copied second part '" << filepath << "' in '" << filepath1 << "'" << endl;
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

	signal(semaphore2);
	wait(mutex);
	cout << threadName << ": wait 1" << endl;
	signal(mutex);

	wait(semaphore1);

	{

		char filepath[] = "/fajl25.dat";
		File *f = FS::open(filepath, 'r');

		wait(mutex);
		cout << threadName << ": File opened " << filepath << "" << endl;
		signal(mutex);

		ofstream fout("izlaz2.dat", ios::out | ios::binary);

		char *buff = new char[f->getFileSize()];

		f->read(f->getFileSize(), buff);
		fout.write(buff, f->getFileSize());

		wait(mutex);
		cout << threadName << ": Written '" << filepath << "'in file 'izlaz1.dat'" << endl;
		signal(mutex);

		delete[] buff;

		fout.close();

		delete f;

		wait(mutex);
		cout << threadName << ": Closed file " << filepath << "" << endl;
		signal(mutex);
	}

	{
		FS::unmount();

		wait(mutex);
		cout << threadName << ": Partition p1 unmounted" << endl;
		signal(mutex);
	}

	wait(mutex);
	cout << threadName << ": Done!" << endl;

	signal(mutex);
	signal(mainSemaphore);

	return 0;
}