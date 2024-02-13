#pragma once
#pragma warning (disable : 4800 )


#pragma pack(push, 1) // 디버그용
#include <string>
#include <windows.h>

class Sync
{
public:
	Sync(const char* name = "NoName", int nLockOrder = -1);
	virtual ~Sync();
	virtual void Lock( const char* szFileName = "", int nLineNumber = 0 )		= 0;
	virtual void UnLock()	= 0;
	virtual bool IsLocked()	= 0;
	virtual bool TryLock( const char* szFileName = "", int nLineNumber = 0 )	{ return false; }

	void	onLock(const char* szFileName, int nLineNumber);
	void	onAcquire();
	void	onUnLock();
	static std::string getLockState();
	static void	dumpLockState();
	static void	dumpLockStateLog();

	std::string m_strName;

	const char*	m_pszFileName;
	int			m_nLineNumber;
	int			m_nAcquireCount;
	int			m_nLockOrder;
	char		m_DummyByte[3];	// for debug , Testors
};

#pragma pack( pop )

class DummySync : public Sync
{
public:
	DummySync(const char* name = "NoName");
	~DummySync();

	virtual void Lock(const char* szFileName = "", int nLineNumber = 0);
	virtual void UnLock();
	virtual bool IsLocked();
	virtual bool TryLock(const char* szFileName = "", int nLineNumber = 0);

	bool m_bIsLocked;
};

#ifdef _WIN32

#if (_WIN32_WINNT < 0x0400 )
#define _NO_TRY_ENTER
#endif
// }
class Mutex : public Sync
{
public:
	Mutex(const char* szName = "NoNameMutex", int nLockOrder = -1, bool make_not_initialize_mutex = false);
	~Mutex();
	bool Open(const char* szName);
	bool IsOpen();
	void Close();
	virtual void Lock(const char* szFileName = "", int nLineNumber = 0);
	virtual void UnLock();
	virtual bool IsLocked();
	virtual bool TryLock(const char* szFileName = "", int nLineNumber = 0);

private:
	bool	m_bIsLocked;
	HANDLE	m_hMutex;
};

class CriticalSection : public Sync
{
public:
	CriticalSection(const char* name = "NoNameCriticalSection", int nLockOrder = -1);
	CriticalSection(DWORD dwSpinCount);
	~CriticalSection();
	virtual void Lock(const char* szFileName = "", int nLineNumber = 0);
	virtual void UnLock();
	virtual bool IsLocked();
	HANDLE	GetOwningThread();
private:

	CRITICAL_SECTION m_cs;
	volatile int	 m_nLockCount;
};

class SpinLock : public Sync
{
public:

	SpinLock(const char* name = "NoNameSpinLock", int nLockOrder = -1);
	~SpinLock();
	virtual void Lock(const char* szFileName = "", int nLineNumber = 0);
	virtual void UnLock();
	virtual bool TryLock(const char* szFileName = "", int nLineNumber = 0);
	virtual bool IsLocked();
private:

	volatile int m_nLockCount;
	volatile bool	 m_bIsLocked;
};

#endif // _WIN32

class ScopedLock
{
public:

	ScopedLock(Sync* lock, const char* szFileName, int nLineNumber);
	ScopedLock(Sync* lock, bool bLock = true);
	ScopedLock(Sync& lock, const char* szFileName, int nLineNumber);
	ScopedLock(Sync& lock);
	~ScopedLock();

private:

	Sync* m_lock;
};

class LockSweeper
{
public:
	LockSweeper(Sync* lock);
	LockSweeper(Sync& lock);
	~LockSweeper();
private:
	Sync* m_lock;
};

class ScopedSpinLock
{
public:
	ScopedSpinLock(volatile int* lock);
	ScopedSpinLock(volatile int& lock);
	~ScopedSpinLock();
private:
	volatile int * m_lock;
};

#include <stack>
extern thread_local std::stack<std::string> lock_stack;

#define __LOCK( SyncObject ) SyncObject.Lock( __FUNCTION__" : "__FILE__, __LINE__ );
#define __UNLOCK( SyncObject ) SyncObject.UnLock();

#define THREAD_SYNCHRONIZE( SyncObject ) ScopedLock __scoped_lock( SyncObject, __FUNCTION__" : "__FILE__, __LINE__ );
#define THREAD_SYNCHRONIZE1( SyncObject ) ScopedLock __scoped_lock_1( SyncObject, __FUNCTION__" : "__FILE__, __LINE__ );
#define THREAD_SYNCHRONIZE2( SyncObject ) ScopedLock __scoped_lock_2( SyncObject, __FUNCTION__" : "__FILE__, __LINE__ );
#define THREAD_SYNCHRONIZE3( SyncObject ) ScopedLock __scoped_lock_3( SyncObject, __FUNCTION__" : "__FILE__, __LINE__ );
#define THREAD_SYNCHRONIZE_END( SyncObject ) ScopedLock __scoped_lock_end( SyncObject, __FUNCTION__" : "__FILE__, __LINE__ );
#define SPINLOCK_SYNCHRONIZE( SyncObject ) ScopedSpinLock __scoped_spinlock( SyncObject );

