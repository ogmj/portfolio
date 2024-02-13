#pragma once
#include <iostream>

template<typename T, int ALLOC_BLOCK_SIZE = 50>
class MemoryPool
{
public:
	static void* operator new(std::size_t alloc_length)
	{

		//���� �� �̻� �Ҵ��� �� �ִ� ������ ���� ��� ���� �Ҵ��մϴ�.
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
		//delete�� ����� next�� ���� m_free_pointer�� �ּҸ� �־��ݴϴ�.
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
		//m_free_pointer�� ����� ũ���� �޸𸮸� �Ҵ��մϴ�.

		unsigned char** current = reinterpret_cast<unsigned char**>(m_free_pointer);
		//�Ҵ��� �޸��� ù ����� �����͸� current�� �ֽ��ϴ�.

		unsigned char* next = m_free_pointer; //�Ҵ�� �޸� ù ����Դϴ�.
		for (int i = 0; i < ALLOC_BLOCK_SIZE - 1; ++i)
		{
			next += alloc_length;	//���� ����� ����մϴ�.
			*current = next;
			current = reinterpret_cast<unsigned char**>(next);
		}
		*current = nullptr;	//�������� ��� �տ� �ּҴ� nullptr�Դϴ�.

	}

private:
	static unsigned char* m_free_pointer;

protected:
	~MemoryPool(){}
};

template<typename T, int ALLOC_BLOCK_SIZE>
unsigned char* MemoryPool<T, ALLOC_BLOCK_SIZE>::m_free_pointer = nullptr;
