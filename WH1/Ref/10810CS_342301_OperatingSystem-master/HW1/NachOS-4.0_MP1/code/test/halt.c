/* halt.c
 *	Simple program to test whether running a user program works.
 *	
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures 
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as 
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"
// include <stdio.h> // 1910010[J]: �L�kinclude <stdio.h> ...
// 1910010[J]: Halt()�busrProg/syscall.h�̩I�s�A�i��N�Oreturn 0 ���N��A�Բөw�q����m�i��O�b
int
main()
{
    PrintInt(666666); 
    Halt();
    /* not reached */
    PrintInt(777777);
}
