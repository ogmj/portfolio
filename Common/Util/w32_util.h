// 2004/03/12 ��� �ý��� ���� ������ ������ �ȵ� �𽺾���� �ؼ� ���ϴ°͸� �̾Ƴ�.
// 2009/01/23 Win64 ������ ���� �ζ��� ������� w32_util_64.asm �� �и� �Ŀ� w64_util.lib �� �ٲ�.
//            Win32 ������ ���� ��Ĵ�� w32_util_64.h �� �����ϴ°����� ��� ����.
//            Win64 ������ w32_util.h ���� �� w32_util_64.lib �� ���̺귯�� ��ũ�� �߰��ؾ���.
//
//	by Testors

#pragma once

#ifdef _WIN64

extern "C" 
{

extern void break_point();
extern int interlocked_increment( volatile int * target, int count = 1 );
extern int interlocked_decrement( volatile int * target, int count = 1 );
extern int interlocked_exchange( volatile int * target, int value );
extern __int64 interlocked_increment64( volatile __int64 * target, int count = 1 );
extern __int64 interlocked_decrement64( volatile __int64 * target, int count = 1 );
extern __int64 interlocked_exchange64( volatile __int64 * target, __int64 value );
inline unsigned __int64 getElapsedPicoSecond()	{ return *(unsigned __int64 *)0x7FFE0008; }
extern unsigned __int64 getRDTSC();

}

#else

inline void break_point()
{
	__asm { int 3 }
}

inline int interlocked_increment( volatile int * target, int count = 1 ) 
{ 
   __asm { 
      mov         ebx, target 
      mov         eax, count 
      lock xadd   dword ptr [ebx],eax 
   } 
} 

inline int interlocked_decrement( volatile int * target, int count = 1 ) 
{ 
   __asm { 
      mov         ebx, target 
      mov         eax, count 
      neg         eax 
      lock xadd   dword ptr [ebx],eax 
   } 
} 

inline int interlocked_increment_one_return_new( volatile int * target ) 
{ 
	__asm { 
		mov			ebx, target 
		mov         eax,1 
		lock xadd   dword ptr [ebx],eax 
		inc         eax  
	} 
}

inline int interlocked_decrement_one_return_new( volatile int * target ) 
{ 
	__asm { 
		mov			ebx, target 
		mov         eax,0FFFFFFFFh 
		lock xadd   dword ptr [ebx],eax 
		dec         eax
	} 
}

inline int interlocked_exchange( volatile int * target, int value )
{
   __asm { 
      mov         ebx, target 
      mov         eax, value 
      lock xchg   dword ptr [ebx],eax 
   } 
}

// winmm.lib �� ������ �߰��� �ź���(?) Ÿ�̸� �ּ�. un-documented feature ��� �����ϰ�
// �����ս� ī���� ������ ����ϰ� �־����� ���� ���۷����� �߰�.
// NT Ŀ�� �̻󿡼��� �������� �ᵵ �ɵ�.
// http://research.microsoft.com/invisible/src/base/md/i386/sim/_glue.c.htm
inline unsigned __int64 getElapsedPicoSecond() 
{ 
   __asm 
   { 
      mov         edx,dword ptr ds:7FFE000Ch
      mov         eax,dword ptr ds:7FFE0008h
   } 
} 

inline unsigned __int64 getRDTSC() 
{ 
   __asm 
   { 
      rdtsc 
   } 
}


#endif

inline unsigned int getElapsedMilliSecond() 
{ 
   return static_cast< unsigned int >( getElapsedPicoSecond()/10000 ); 
} 

inline void spinlock_init( volatile int * lock )
{
	*lock = 0;
}

inline void spinlock_enter( volatile int * lock ) 
{ 
enter:
	if( interlocked_exchange( lock, 1 ) == 0 ) return;

	goto enter;
} 

inline void spinlock_leave( volatile int * lock ) 
{ 
	interlocked_exchange( lock, 0 );
} 

inline bool spinlock_tryenter( volatile int * lock ) 
{ 
	if( interlocked_exchange( lock, 1 ) == 0 ) return true;

	return false;
} 