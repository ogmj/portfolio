#pragma once
#include <iostream>

template<typename T, int ALLOC_BLOCK_SIZE = 50>
class MemoryPool
{
public:
	static void* operator new(std::size_t alloc_length)
	{

		//만약 더 이상 할당할 수 있는 공간이 없을 경우 새로 할당합니다.
		if (!m_free_pointer)
		{
			alloc_block(get_size(alloc_length));
		}
		unsigned char* ReturnPointer = m_free_pointer;
		m_free_pointer = *reinterpret_cast<unsigned char**>(ReturnPointer);


 		return ReturnPointer;
	}

	static void operator delete(void* delete_pointer)
	{
		*reinterpret_cast<unsigned char**>(delete_pointer) = m_free_pointer;
		//delete된 블록의 next에 현재 m_free_pointer의 주소를 넣어줍니다.
		m_free_pointer = static_cast<unsigned char*>(delete_pointer);
	}

private:
	static std::size_t get_size(std::size_t alloc_length)
	{
		if (alloc_length > 8)
		{
			return alloc_length;
		}
		else
		{
			T* p = nullptr;
			return sizeof(p);
		}
	}
	static void alloc_block(std::size_t alloc_length)
	{
		m_free_pointer = new unsigned char[alloc_length * ALLOC_BLOCK_SIZE];
		//m_free_pointer에 사용할 크기의 메모리를 할당합니다.

		unsigned char** current = reinterpret_cast<unsigned char**>(m_free_pointer);
		//할당한 메모리의 첫 블록의 포인터를 current에 넣습니다.

		unsigned char* next = m_free_pointer; //할당된 메모리 첫 블록입니다.
		for (int i = 0; i < ALLOC_BLOCK_SIZE - 1; ++i)
		{
			next += alloc_length;	//다음 블록을 계산합니다.
			*current = next;
			current = reinterpret_cast<unsigned char**>(next);
		}
		*current = nullptr;	//마지막일 경우 앞에 주소는 nullptr입니다.

	}

private:
	static unsigned char* m_free_pointer;

protected:
	~MemoryPool(){}
};

template<typename T, int ALLOC_BLOCK_SIZE>
unsigned char* MemoryPool<T, ALLOC_BLOCK_SIZE>::m_free_pointer = nullptr;
