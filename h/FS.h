#pragma once

typedef long FileCnt;
typedef unsigned long BytesCnt;

const unsigned int FNAMELEN = 8;
const unsigned int FEXTLEN = 3;

class KernelFS;
class Partition;
class File;

class FS
{
public:
   ~FS();

   static char format();
   static char doesExist(char *fname);
   static FileCnt readRootDir();

   static char mount(Partition *partition);
   static char unmount();

   static File *open(char *fname, char mode);
   static char deleteFile(char *fname);

   static void write();

protected:
   friend int main();

   static KernelFS *myImpl;

   FS();
};