// 2004/03/12 몇가지 시스템 콜의 성능이 마음에 안들어서 디스어셈블 해서 원하는것만 뽑아냄.
// 2009/01/23 Win64 포팅을 위해 인라인 어셈들을 w32_util_64.asm 로 분리 후에 w64_util.lib 로 바꿈.
//            Win32 에서는 기존 방식대로 w32_util_64.h 를 포함하는것으로 사용 가능.
//            Win64 에서는 w32_util.h 포함 후 w32_util_64.lib 를 라이브러리 링크에 추가해야함.
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

// winmm.lib 를 뒤져서 발견한 신비한(?) 타이머 주소. un-documented feature 라고 생각하고
// 퍼포먼스 카운터 등으로 사용하고 있었으나 드디어 레퍼런스를 발견.
// NT 커널 이상에서는 마음놓고 써도 될듯.
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