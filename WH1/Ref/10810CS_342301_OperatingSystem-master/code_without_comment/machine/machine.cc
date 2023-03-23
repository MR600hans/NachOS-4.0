#include "machine.h"
#include "main.h"

static char* exceptionNames[] = { "no exception", "syscall",
				"page fault/no TLB entry", "page read only",
				"bus error", "address error", "overflow",
				"illegal instruction" };

static void CheckEndian()
{
    union checkit {
        char charword[4];
        unsigned int intword;
    } check;

    check.charword[0] = 1;
    check.charword[1] = 2;
    check.charword[2] = 3;
    check.charword[3] = 4;

#ifdef HOST_IS_BIG_ENDIAN
    ASSERT (check.intword == 0x01020304);
#else
    ASSERT (check.intword == 0x04030201);
#endif
}

Machine::Machine(bool debug)
{
    int i;
    for (i = 0; i < NumTotalRegs; i++)
        registers[i] = 0;
    mainMemory = new char[MemorySize];
    for (i = 0; i < MemorySize; i++)
      	mainMemory[i] = 0;
#ifdef USE_TLB
    tlb = new TranslationEntry[TLBSize];
    for (i = 0; i < TLBSize; i++)
	tlb[i].valid = FALSE;
    pageTable = NULL;
#else	// use linear page table
    tlb = NULL;
    pageTable = NULL;
#endif

    singleStep = debug;
    CheckEndian();
}

Machine::~Machine()
{
    delete [] mainMemory;
    if (tlb != NULL)
        delete [] tlb;
}

//----------------------------------------------------------------------
// Machine::RaiseException
// 	Transfer control to the Nachos kernel from user mode, because
//	the user program either invoked a system call, or some exception
//	occured (such as the address translation failed).
//
//	"which" -- the cause of the kernel trap
//	"badVaddr" -- the virtual address causing the trap, if appropriate
//----------------------------------------------------------------------

// 191019[J]: �bmipssim.cc�o���@�FMachine::Run�A�̭��I�s�FMachine::OneInstruction�A�M��OPCODE��Ū��61 (OP_SYSCALL)
// 191019[J]: ���ۦbmipssim.cc����OneInstruction�̭��S�A�שI�s�FRaiseException
void
Machine::RaiseException(ExceptionType which, int badVAddr)
{
    DEBUG(dbgMach, "Exception: " << exceptionNames[which]);

    registers[BadVAddrReg] = badVAddr;

    DelayedLoad(0, 0);			// finish anything in progress

    // 191012[J]: ��mipssim.cc�I�s�A�N�O�b�o���ഫ��KERNEL MODE��!!!
    kernel->interrupt->setStatus(SystemMode);

    // 191019[J]: �o��exception handler�h�O�bexception.cc�w�q��
    ExceptionHandler(which);		// interrupts are enabled at this point // 191012[J]: ! �w�鰻������~

    kernel->interrupt->setStatus(UserMode); // 191012[J]: exception�B�z�����A��^user mode
}

//----------------------------------------------------------------------
// Machine::Debugger
// 	Primitive debugger for user programs.  Note that we can't use
//	gdb to debug user programs, since gdb doesn't run on top of Nachos.
//	It could, but you'd have to implement *a lot* more system calls
//	to get it to work!
//
//	So just allow single-stepping, and printing the contents of memory.
//----------------------------------------------------------------------

void Machine::Debugger()
{
    char *buf = new char[80];
    int num;
    bool done = FALSE;

    kernel->interrupt->DumpState();
    DumpState();
    while (!done) {
      // read commands until we should proceed with more execution
      // prompt for input, giving current simulation time in the prompt
      cout << kernel->stats->totalTicks << ">";
      // read one line of input (80 chars max)
      cin.get(buf, 80);
      if (sscanf(buf, "%d", &num) == 1) {
	runUntilTime = num;
	done = TRUE;
      }
      else {
	runUntilTime = 0;
	switch (*buf) {
	case '\0':
	  done = TRUE;
	  break;
	case 'c':
	  singleStep = FALSE;
	  done = TRUE;
	  break;
	case '?':
	  cout << "Machine commands:\n";
	  cout << "    <return>  execute one instruction\n";
	  cout << "    <number>  run until the given timer tick\n";
	  cout << "    c         run until completion\n";
	  cout << "    ?         print help message\n";
	  break;
	default:
	  cout << "Unknown command: " << buf << "\n";
	  cout << "Type ? for help.\n";
	}
      }
      // consume the newline delimiter, which does not get
      // eaten by cin.get(buf,80) above.
      buf[0] = cin.get();
    }
    delete [] buf;
}

//----------------------------------------------------------------------
// Machine::DumpState
// 	Print the user program's CPU state.  We might print the contents
//	of memory, but that seemed like overkill.
//----------------------------------------------------------------------

void
Machine::DumpState()
{
    int i;

    cout << "Machine registers:\n";
    for (i = 0; i < NumGPRegs; i++) {
	switch (i) {
	  case StackReg:
	    cout << "\tSP(" << i << "):\t" << registers[i];
	    break;

	  case RetAddrReg:
	    cout << "\tRA(" << i << "):\t" << registers[i];
	    break;

	  default:
	    cout << "\t" << i << ":\t" << registers[i];
	    break;
	}
	if ((i % 4) == 3) { cout << "\n"; }
    }

    cout << "\tHi:\t" << registers[HiReg];
    cout << "\tLo:\t" << registers[LoReg];
    cout << "\tPC:\t" << registers[PCReg];
    cout << "\tNextPC:\t" << registers[NextPCReg];
    cout << "\tPrevPC:\t" << registers[PrevPCReg];
    cout << "\tLoad:\t" << registers[LoadReg];
    cout << "\tLoadV:\t" << registers[LoadValueReg] << "\n";
}

//----------------------------------------------------------------------
// Machine::ReadRegister/WriteRegister
//   	Fetch or write the contents of a user program register.
//----------------------------------------------------------------------

// 191018[H]: Ū�Ȧs��
int
Machine::ReadRegister(int num)
{
    ASSERT((num >= 0) && (num < NumTotalRegs));
    return registers[num];
}

// 191018[H]: �g�Ȧs��
void
Machine::WriteRegister(int num, int value)
{
    ASSERT((num >= 0) && (num < NumTotalRegs));
    registers[num] = value;
}

