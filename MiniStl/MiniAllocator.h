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

		typedef T			value_type;//模板参数类型
	private:
		typedef T*			pointer;//模板参数指针类型

		typedef const T*	const_pointer;//模板参数常指针类型

		typedef T&			reference;//模板参数引用类型

		typedef const T&	const_reference;//模板参数常引用类型

		typedef size_t		size_type;//模板中的计数类型

		typedef ptrdiff_t	difference_type;

		//rebind用于改变空间配置器想配置的类型
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
		}//返回传入元素的地址

		const_pointer address(const T& x)const
		{
			return &x;
		}//返回传入常量元素的引用

		size_t max_size()const
		{
			return UINT_MAX / sizeof(T);//最大数目
		}
		static pointer allocate(size_t n = 1, const void*ptr = 0)
		{
			return (pointer)Alloc::allocate(sizeof(T)*n); //直接传入需要的内存字节大小
		}

		static void deallocate(T* ptr, size_t n = 1)
		{
			Alloc::deallocate(ptr, n);
		}

		template<typename... Args>
		static void construct(T* ptr, Args&&... args)//C++11的可变函数模板参数 
		{//实际也是利用new去调用相应的构造函数,forward能够用来保持参数类型的不变形
			//如果传入的args为左值，forward返回左值,右值则返回右值
			//利用定位new表达式进行元素的构造
			new (ptr)T(std::forward<Args>(args)...);
		}

		static void destroy(T* ptr)
		{
			ptr->~T();
		}


	};
}
#endif
