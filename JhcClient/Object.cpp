#include "ExternClass.h"
#include "Object.h"

volatile __declspec( thread ) Object* Object::s_pCurrentThreadObject = NULL;

Object::Object(void) : m_isDelete( false ), m_nDeleteTime( 0 )
{
}

Object::~Object(void)
{
}

void Object::Delete()
{
	m_isDelete = true;
	m_nDeleteTime = g_GlobalTime.GetTime();
}