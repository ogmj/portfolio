#pragma once
#include <stdio.h>

enum SEEK { BEGIN = SEEK_SET, CURRENT = SEEK_CUR, END = SEEK_END };
enum MODE { READ = 0, CREATE = 2, APPEND = 3 };

class StreamIntf
{
public:
	StreamIntf();
	virtual ~StreamIntf();

	virtual int			Read( void * szBuf, size_t nLen )			= 0;
	virtual int			Write( const void * szBuf, size_t nLen )	= 0;
	virtual int			Peek( void * szBuf, size_t nLen );
	virtual const char* GetBuf();

	virtual int			Tell();
	virtual bool		Seek(SEEK seek, int offset);

	virtual bool		IsValid()		= 0;
	virtual int			Size();
};

class FileStream : public StreamIntf
{
public:
	FileStream();
	FileStream(const char* szFileName, MODE mode);
	~FileStream();

	virtual int			Read(void* szBuf, size_t nLen);
	virtual int			Write(const void* szBuf, size_t nLen);
	virtual int			WriteEx(const void* szBuf, size_t nLen);
	virtual int			Peek(void* szBuf, size_t nLen);

	virtual const char* GetBuf();
	virtual int			Tell();
	virtual bool		Seek(SEEK seek, int offset);
	virtual bool		IsValid();
	virtual int			Size();

private:
	FILE *m_fp;
};

class MemoryStream : public StreamIntf
{
public:
	MemoryStream();
	MemoryStream(const void* pBuffer, size_t len, bool bDeepCopy = true);
	~MemoryStream();

	virtual int	Read(void* szBuf, size_t nLen);
	virtual int Write(const void* szBuf, size_t nLen);
	virtual int	Peek(void* szBuf, size_t nLen);
	virtual int Tell();
	virtual bool Seek(SEEK seek, int offset);
	virtual bool IsValid();
	virtual int Size();

	virtual const char* GetBuf();

private:

	void alloc(size_t len);
	void remove_link();

	bool	m_bLink;
	char	*m_pBuffer;
	size_t	m_nPos;
	size_t	m_nBufLen;
};
