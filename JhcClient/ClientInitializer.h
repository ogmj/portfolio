#pragma once

class ClientInitializer
{
public:
	ClientInitializer(class IOCP* iocp);
	~ClientInitializer(void);

	bool onProcess();

protected:
	void onInitClient();
	void onNetworkConnect();

private:
	class IOCP* m_pIOCP;
};
