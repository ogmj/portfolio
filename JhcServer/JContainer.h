#pragma once
#include "JHash.h"
#include "../network/include/thread/Sync.h"

template<typename T>
class JContainer
{
public:
	JContainer() {};
	~JContainer() {};

	void Add(std::shared_ptr<T>& p)
	{
		THREAD_SYNCHRONIZE(m_lock);
		m_hs_Obj.Add(p.get()->m_nKey, p);
	}

	void Get(int _key, std::shared_ptr<T>& p)
	{
		THREAD_SYNCHRONIZE(m_lock);
		m_hs_Obj.Get(_key, p);
	}
	void Delete(int _key, std::shared_ptr<T>& p)
	{
		THREAD_SYNCHRONIZE(m_lock);
		m_hs_Obj.Delete(_key, p);
	}

	void Proc(unsigned int tm, std::vector< std::shared_ptr<T>>& v_delete)
	{
		std::vector< std::shared_ptr<T>> v_data;
		Enum(v_data);

		for (auto& i : v_data)
		{
			THREAD_SYNCHRONIZE(i->m_player_lock);
			i->Proc(tm);
			//if (i->Proc(tm))
			//{
			//	v_delete.emplace_back(i);
			//}
		}
	};

	void Enum(std::vector< std::shared_ptr<T>>& v_enum)
	{
		THREAD_SYNCHRONIZE(m_lock);
		m_hs_Obj.Enum(v_enum);
	}

private:
	JHash<std::shared_ptr<T>> m_hs_Obj;
	CriticalSection m_lock;
};