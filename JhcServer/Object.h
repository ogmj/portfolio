#pragma once

#include "../Common/Util/Type.h"
#include <set>
class Object
{
public:

	struct AutoThreadObject
	{
		AutoThreadObject( Object* pObj ) : prev_ptr( pObj ) {}
		~AutoThreadObject() { if ( prev_ptr ) Object::setCurrentThreadObject( prev_ptr ); }

		Object* prev_ptr;
	};

	static Object* GetCurrentThreadObject()					{ return (Object*)s_pCurrentThreadObject; }
	static Object* setCurrentThreadObject( Object* obj )	{ s_pCurrentThreadObject = obj; return GetCurrentThreadObject()	; }

	Object(void);
	virtual ~Object(void);

	virtual Object*					ToObject()				{ return this; }
	virtual class NetJhcServer*		ToJhcServer()			{ return nullptr; }
	virtual class Player*			ToPlayer()				{ return nullptr; }

	virtual bool Proc( SERVER_TIME tm )			{ return true; }

	bool			IsDeleted() const		{ return m_isDelete; }
	void			Delete();
	unsigned		GetDeleteTime() const	{ return m_nDeleteTime; }

	virtual void	onDelete()	{}
	virtual void	Send( struct Message* pMsg )	{}

private:
	bool			m_isDelete;
	SERVER_TIME		m_nDeleteTime;
protected:
	static volatile __declspec( thread ) Object*		s_pCurrentThreadObject;
};
