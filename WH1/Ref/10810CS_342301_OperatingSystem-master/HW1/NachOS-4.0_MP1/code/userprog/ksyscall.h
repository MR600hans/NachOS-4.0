/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 **************************************************************/

// 1910010[J]: Hint: �o���ɮ׻ݭn�Q�ק�!
// 1910010[J]: syscall���G�O��user program�ݪ������A��ksyscall�h�O�bkernel mode�U�u���I�ssystem call���\��
// 191012[J]: �i��O�n��u�{���Ҷq�Aksyscall�S�|�A�שI�s�u�����h���\���@ 
// 191012[J]: �HFile Operation���ҡAksyscall���u�O�I�s�Ffilesys.h�̹�@���\��
#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
#include "synchconsole.h"


void SysHalt()
{
  kernel->interrupt->Halt();
}

void SysPrintInt(int val)
{ 
  DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, into synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
  kernel->synchConsoleOut->PutInt(val);
  DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, return from synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->fileSystem->Create(filename);
}


// 1910010[J]: �H�U����ӷs�w�q��4�Ӥ���
// 1910010[J]: ----------------------------------------------------------
// Open a file with the name, and returns its corresponding OpenFileId. 
// Return -1 if fail to open the file.
OpenFileId SysOpen(char *name) // 191012[J]: ���G�i�H�F?
{
  return kernel->fileSystem->OpenAFile(name);
}

// Write ��size�� characters from the buffer into the file, 
// and return the number of characters actually written to the file. 
// Return -1, if fail to write the file. 
int SysWrite(char *buffer, int size, OpenFileId id)
{
  return kernel->fileSystem->WriteFile0(buffer, size, id);
}

// Read ��size�� characters from the file to the buffer, 
// and return the number of characters actually read from the file. 
// Return -1, if fail to read the file.
int SysRead(char *buffer, int size, OpenFileId id)
{
  return kernel->fileSystem->ReadFile(buffer, size, id);
}

// Close the file with id. 
// Return 1 if successfully close the file. Otherwise, return -1.
int SysClose(OpenFileId id)
{
  return kernel->fileSystem->CloseFile(id);
}
// 1910010[J]: ----------------------------------------------------------

#endif /* ! __USERPROG_KSYSCALL_H__ */
