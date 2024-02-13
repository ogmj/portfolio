#pragma once

#include "MsgBase.h"
#include "MessageCommand.h"
//#include "MessageSub.h"

#pragma pack( push, 1 )

struct SC_Ping : public Message
{
	SC_Ping() { msg = SC_PING; size = sizeof(*this); }
	__int64 timestamp;
};

struct CS_Ping : public Message
{
	CS_Ping() { msg = CS_PING; size = sizeof(*this); }
	__int64 timestamp;
};

struct SC_Encryption : public Message
{
	SC_Encryption() { msg = SC_ENCRYPTION; size = sizeof( *this ); }

	int key_length;
	// 이하 key_length만큼 string 이 따라붙음
};

struct CS_Echo : public Message
{
	CS_Echo() { msg = CS_ECHO; size = sizeof(*this); }
	char echo_message[256];
};

struct SC_Echo : public Message
{
	SC_Echo() { msg = SC_ECHO; size = sizeof(*this); }
	char echo_message[256];
};

#pragma pack( pop )