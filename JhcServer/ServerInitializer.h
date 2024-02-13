#pragma once

class ServerInitializer
{
public:
	ServerInitializer(class IOCP* iocp, class IOCPAcceptor* acceptor);
	~ServerInitializer(void);

	bool onProcess();

protected:
	void onInitServer();
	void onInitNetwork();
	void onInitResource();
	void onInitDB();


private:
	class IOCP* m_pIOCP;
	class IOCPAcceptor* m_pAcceptor;
};
