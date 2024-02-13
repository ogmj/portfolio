#pragma once

#include "Limit.h"

class Addr
{
public:
	Addr(const char* szAddr = 0, int nPort = 0);
	~Addr();
	const char* GetAddr() const;
	int	GetPort() const;
	void SetAddr(const char* szAddr);
	void SetPortNum(int nPort);
	const Addr& operator = (const Addr& addr);
	bool operator == (const Addr& addr) const;

private:
	void setAddr(const char* szAddr);
	void setPort(int nPort);

	char	m_szAddr[ADDRESS_MAX_LENGTH];
	int		m_nPort;
};
