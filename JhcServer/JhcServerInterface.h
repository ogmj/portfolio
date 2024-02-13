#pragma once
#include "../Common/Interface/Interface.h"

class JhcServerInterface : public Interface
{
public:
	JhcServerInterface() {};
	~JhcServerInterface() {};

	virtual bool onInit();
	virtual bool onDeInit();
	virtual void onCommand(std::vector<std::string>& _vec_command);


};