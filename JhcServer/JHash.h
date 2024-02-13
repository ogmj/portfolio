#pragma once
#include <vector>
/*
//template< typename T>
class node
{
public:
	//node(const T& obj, int key) : m_pObj(obj), m_Key(key) {};
	node(int key) : m_Key(key) {};
	~node() {};
	//T* m_pObj;
	int m_Key;
};
*/

template< typename T>
class JHash
{
public:

	class slot
	{
	public:
		slot()
		{
			m_slot.reserve(100);
		};
		~slot() {};
		std::vector<T> m_slot;
		std::vector<int> m_int;
		//typedef std::vector<T>::iterator slot_it;
		//using slot_it = std::vector<T>::iterator;
	};

	JHash(int max_slot = 10) : m_nMaxSlot(max_slot)
	{
		m_pSlot = new slot[m_nMaxSlot];
	};
	~JHash()
	{
		Clear();
		delete [] m_pSlot;
	};

	bool Add(int nKey, T& obj)
	{
		unsigned int hash_code = nKey % GetMaxSlot();
		slot * p_curr_slot = m_pSlot + hash_code;

		auto it = std::lower_bound(p_curr_slot->m_slot.begin(), p_curr_slot->m_slot.end(), nKey, []( T p, int key) -> int
		{
			return p->m_nKey < key;
		});
		if (it != p_curr_slot->m_slot.end())
		{
			//같은 키는 패스
			if ((*it)->m_nKey == nKey)
			{
				return false;
			}
		}

		if (p_curr_slot->m_slot.empty())
		{
			p_curr_slot->m_slot.emplace_back(obj);
		}
		else
		{
			p_curr_slot->m_slot.insert(it, obj);
		}
		return true;
	};

	void Get(int key ,T& obj)
	{
		unsigned int hash_code = key % GetMaxSlot();
		slot* p_curr_slot = m_pSlot + hash_code;

		auto it = std::lower_bound(p_curr_slot->m_slot.begin(), p_curr_slot->m_slot.end(), key, [](T p, int key)
		{
			return p->m_nKey > key;
		});
		if (it != p_curr_slot->m_slot.end())
		{
			//같은 키는 패스
			if ((*it)->m_nKey == key)
			{
				obj = (*it);
			}
		}
		obj = nullptr;
	};

	void Delete(int key, T& obj)
	{
		unsigned int hash_code = key % GetMaxSlot();
		slot* p_curr_slot = m_pSlot + hash_code;

		auto it = std::lower_bound(p_curr_slot->m_slot.begin(), p_curr_slot->m_slot.end(), key, [](T p, int key)
		{
			return p->m_nKey > key;
		});
		if (it != p_curr_slot->m_slot.end())
		{
			//같은 키는 패스
			if ((*it)->m_nKey == key)
			{
				p_curr_slot->m_slot.erase(it);
				//it.reset();
			}
		}
	}

	void Clear()
	{
		for (int i = 0; i < GetMaxSlot(); ++i)
		{
			slot* p_curr_slot = m_pSlot + i;

			for (auto& i : p_curr_slot->m_slot)
			{
				i.reset();
			}

			p_curr_slot->m_slot.clear();
		}
	};

	void Enum(std::vector<T>& v_enum)
	{
		for (int i = 0; i < GetMaxSlot(); ++i)
		{
			slot* p_curr_slot = m_pSlot + i;

			for (auto& i : p_curr_slot->m_slot)
			{
				v_enum.emplace_back(i);
			}
		}
	};

	int GetCount()
	{
		return m_nCnt;
	}


private:
	int GetMaxSlot(){ return m_nMaxSlot; };
private:
	int m_nMaxSlot;
	int m_nCnt;
	slot* m_pSlot;

};