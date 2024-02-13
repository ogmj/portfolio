#pragma once
#include "../Common/Interface/Interface.h"

class JhcClientInterface : public Interface
{
public:
	JhcClientInterface() {};
	~JhcClientInterface() {};

	virtual bool onInit();
	virtual bool onDeInit();
	virtual void onCommand(std::vector<std::string>& _vec_command);

};