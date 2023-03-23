// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//  // 191019[J]: �ثe���Pı, syscall�Ouser program���椤�ɥD�ʵo�Xtrap��kernel�ӽШD����@�ǯS����O
//  // 191019[J]: ��exception�O�w�����ɵo�X���ҥ~���p�A Interrupt�h���I���O�n������@�b��M�ݭn������ (�m�ܬY�귽������) �ӵo�X��
//  // 191019[J]: Interrupt.cc ��@�F�n��interrupt�A�Pexception.cc�������A����y�������v���ಾ (user -> kernel)
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.

// 191010[J]: Hint: �o���ɮ׻ݭn�Q�ק�!
// 191012[J]: �qsyscall.h��SC_�N�X�A�A�z�L�I�sksyscall�Ӱ���u����systemcall

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 191019[J]: �o�q���ѭ��n
// 	**Entry point into the Nachos kernel**.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//
//  191012[J]:�o�q���Ѻ����n��
//----------------------------------------------------------------------
// 191019[J]: �o�䪺ExceptionType�w�q��Machine.h���A�HMP1�ӻ��OSysCallExcept
void
ExceptionHandler(ExceptionType which)
{
    char ch;
    int val;
    int type = kernel->machine->ReadRegister(2);
    int status, exit, threadID, programID, fileID, numChar;
    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
    DEBUG(dbgTraCode, "In ExceptionHandler(), Received Exception " << which << " type: " << type << ", " << kernel->stats->totalTicks);
    
    switch (which) {
    case SyscallException:
	
    switch(type) {
	    case SC_Halt:
      {
    		DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
    		SysHalt();
    		cout<<"in exception\n";
    		ASSERTNOTREACHED();
        break;
      }
	    case SC_PrintInt:
      {
    		DEBUG(dbgSys, "Print Int\n");
    		val = kernel->machine->ReadRegister(4);
    		DEBUG(dbgTraCode, "In ExceptionHandler(), into SysPrintInt, " << kernel->stats->totalTicks);    
    		SysPrintInt(val); 	
    		DEBUG(dbgTraCode, "In ExceptionHandler(), return from SysPrintInt, " << kernel->stats->totalTicks);
    		// Set Program Counter
    		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
    		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
    		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
    		return;
    		ASSERTNOTREACHED();
 	      break;
      }
	    case SC_MSG:
      {
    		DEBUG(dbgSys, "Message received.\n");
    		val = kernel->machine->ReadRegister(4);
        char *msg = &(kernel->machine->mainMemory[val]);
    		cout << msg << endl;
        SysHalt(); // 191012[J]: �o���Halt�Ӥ���Return�A���M���ԲӪ��t�O����?
    		ASSERTNOTREACHED();
 	      break;
      }
	    case SC_Create: // 191012[J]: 
      {
    		val = kernel->machine->ReadRegister(4);
    		char *filename = &(kernel->machine->mainMemory[val]);
    		//cout << filename << endl;
    		status = SysCreate(filename); // 191012[J]: systemcall�Ӹ`���O�bksyscall�̹�@
    		kernel->machine->WriteRegister(2, (int) status);
    		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
    		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
    		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
    		return;
    		ASSERTNOTREACHED();
 	      break;
      }
 	    case SC_Add:
      {
    		DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
    		/* Process SysAdd Systemcall*/
    		int result;
    		result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
    		/* int op2 */(int)kernel->machine->ReadRegister(5));
    		DEBUG(dbgSys, "Add returning with " << result << "\n");
    		/* Prepare Result */
    		kernel->machine->WriteRegister(2, (int)result);	
    		/* Modify return point */
    		
    		/* set previous programm counter (debugging only)*/
    		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
    		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
    		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
    		/* set next programm counter for brach execution */
    		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
    		
    		cout << "result is " << result << "\n";	
    		return;	
    		ASSERTNOTREACHED();
 	      break;
      }
     
     //  191012[J]: -------------------------------------------------------------------------------
     //  191012[J]: �����@�~��4��function�g�b�o��A��Ӹ`����@�g�bksyscall
      case SC_Open:
      { 
    		DEBUG(dbgSys, "Open\n");
    		// �ɮ׾ާ@�i�ѦҤW����SC_CREATE case
        val = kernel->machine->ReadRegister(4); 
    		cout << "val = " << val << endl;
    		char *filename = &(kernel->machine->mainMemory[val]);
    		cout << "filename = " << filename << endl;
    		status = SysOpen(filename); // 191012[J]: systemcall�Ӹ`���O�bksyscall�̹�@
    		kernel->machine->WriteRegister(2, (int) status);
    		
    		// 191012[J]: �C�@�ӥ\�൲���᳣�n Set Program Counter�C����n�̧�return, assertnotreached, break...
    		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg)); // set previous programm counter (debugging only)
    		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
    		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
    		return;
    		ASSERTNOTREACHED();
        break;
      }
    
         
      case SC_Write: 
      {
        DEBUG(dbgSys, "Write\n");
        val = kernel->machine->ReadRegister(4); 
        cout << "val = " << val << endl;
        char *buffer = &(kernel->machine->mainMemory[val]);
    		cout << "buffer = " << buffer << endl;
        numChar = kernel->machine->ReadRegister(5); 
        fileID = kernel->machine->ReadRegister(6); 
        cout << "fileID = " << fileID << endl;
    		status = SysWrite(buffer, numChar, fileID);
       
        kernel->machine->WriteRegister(2, (int) status);
    		
        // 191012[J]: �C�@�ӥ\�൲���᳣�n Set Program Counter�C����n�̧�return, assertnotreached, break...
    		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg)); // set previous programm counter (debugging only)
    		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
    		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
    		return;
    		ASSERTNOTREACHED();
        break;
      }     
    
      case SC_Read: 
      {
        DEBUG(dbgSys, "Read\n");
        val = kernel->machine->ReadRegister(4); 
        cout << "val = " << val << endl;
        char *buffer = &(kernel->machine->mainMemory[val]);
    		cout << "buffer = " << buffer << endl;
        numChar = kernel->machine->ReadRegister(5); 
        fileID = kernel->machine->ReadRegister(6); 
        cout << "fileID = " << fileID << endl;
    		
    		status = SysRead(buffer, numChar, fileID);
       
    		kernel->machine->WriteRegister(2, (int) status);
    		
        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg)); // set previous programm counter (debugging only)
    		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
    		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
    		return;
    		ASSERTNOTREACHED();
        break;
      }
    
      case SC_Close:
      { 
        DEBUG(dbgSys, "Close\n");
        fileID = kernel->machine->ReadRegister(4);
        cout << "fileID = " << fileID << endl;
        status = SysClose(fileID); 
        
    		kernel->machine->WriteRegister(2, (int) status);
    		
        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg)); // set previous programm counter (debugging only)
    		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
    		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
    		return;
    		ASSERTNOTREACHED();
        break;
      }
     //  191012[J]: -------------------------------------------------------------------------------
          
	    case SC_Exit:
			{
        DEBUG(dbgAddr, "Program exit\n");
              		val=kernel->machine->ReadRegister(4);
              		cout << "return value:" << val << endl;
  			kernel->currentThread->Finish();
              break;
        	    default:
  		  cerr << "Unexpected system call " << type << "\n";
        break;
      }
	}
	break;
	default:
		cerr << "Unexpected user mode exception " << (int)which << "\n";
		break;
  }
    ASSERTNOTREACHED();
}

