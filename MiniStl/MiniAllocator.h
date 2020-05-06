#ifndef MINIALLOCATOR_H
#define MINIALLOCATOR_H
#include"Alloc.h"
#include<cstddef>
#include<climits>

namespace MiniStl
{
	template<typename T, typename Alloc = DefaultAlloc<true, 0> >
	class MiniAllocator
	{
	public:

		typedef T			value_type;//ģ���������
	private:
		typedef T*			pointer;//ģ�����ָ������

		typedef const T*	const_pointer;//ģ�������ָ������

		typedef T&			reference;//ģ�������������

		typedef const T&	const_reference;//ģ���������������

		typedef size_t		size_type;//ģ���еļ�������

		typedef ptrdiff_t	difference_type;

		//rebind���ڸı�ռ������������õ�����
		template<typename U>
		struct rebind
		{
			typedef MiniAllocator<U> other;
		};


	public:

		MiniAllocator() = default;
		MiniAllocator(const MiniAllocator&) = default;

		pointer address(T& x)const
		{
			return &x;
		}//���ش���Ԫ�صĵ�ַ

		const_pointer address(const T& x)const
		{
			return &x;
		}//���ش��볣��Ԫ�ص�����

		size_t max_size()const
		{
			return UINT_MAX / sizeof(T);//�����Ŀ
		}
		static pointer allocate(size_t n = 1, const void*ptr = 0)
		{
			return (pointer)Alloc::allocate(sizeof(T)*n); //ֱ�Ӵ�����Ҫ���ڴ��ֽڴ�С
		}

		static void deallocate(T* ptr, size_t n = 1)
		{
			Alloc::deallocate(ptr, n);
		}

		template<typename... Args>
		static void construct(T* ptr, Args&&... args)//C++11�Ŀɱ亯��ģ����� 
		{//ʵ��Ҳ������newȥ������Ӧ�Ĺ��캯��,forward�ܹ��������ֲ������͵Ĳ�����
			//��������argsΪ��ֵ��forward������ֵ,��ֵ�򷵻���ֵ
			//���ö�λnew���ʽ����Ԫ�صĹ���
			new (ptr)T(std::forward<Args>(args)...);
		}

		static void destroy(T* ptr)
		{
			ptr->~T();
		}


	};
}
#endif
