//逐步采用C++11来写,慢慢全部改用C++11
//先写出大体框架来
/*
整个MiniVector最复杂的地方莫过于内存的重新分配,当内存申请插入元素时需要考虑内存是否不足的问题,
如果内存不足，需要在其他地方重新申请新的的内存,并将原始内存中的元素拷贝到该新分配的内存中，
然后释放分配后的内存，为了应用C++11同时需要考虑移动操作的使用
*/
#ifndef MINIVECTOR_H
#define MINIVECTOR_H
#define NOMINMAX
#include<initializer_list>
#include<exception>
#include<cstddef>
#include<algorithm>
#include"MiniAllocator.h"
//在移动元素时.需要考虑到fill和copy函数是对已经进行过构造的内存进行赋值替换,
//而unitialize_copy是对未进行初始化的内存进行初始化,分清楚两者的使用方法很重要

//fill函数将begin到end范围内的所有元素赋值为value
//fill_n函数将begin开始的n个元素范围赋值为value
//copy函数将begin到end范围内的元素拷贝到result开始的区间中

//copy在使用过程中实际上是调用的类的赋值构造函数


//两类函数最大区别在于当前拷贝或者赋值的内存是否已经初始化过

using namespace std;
using namespace MiniStl;
namespace MiniStl
{

	template<typename T, typename alloc = MiniStl::MiniAllocator<T> >
	class MiniVector
	{

	public:
		typedef T			value_type;
		typedef T*			pointer;
		typedef T*			iterator;
		typedef T&			reference;
		typedef const T&	const_reference;
		typedef ptrdiff_t	different_type;
		typedef size_t		size_type;
		//以上为嵌套类型定义

	private:
		//内部使用的内存分配函数
		iterator start;//起始内存位置
		iterator finish;//当前内存的使用末尾(最后一个之外)
		iterator EndStorage;//内存结束位置

		void fill_init(size_type n, const value_type &value)//起始构造的内存空间大小,由构造函数调用
		{
			auto ptr = alloc::allocate(n);//创建大小为n的空间
			start = ptr;
			finish = ptr + n;
			EndStorage = finish;
			//将创建的空间初始化为T
			while (n--)
			{
				alloc::construct(ptr, value);//构造元素
				ptr++;
			}
		}

		void fill_init(iterator iter1, iterator iter2)
		{
			auto ptr = alloc::allocate(iter2 - iter1);
			uninitialized_copy(iter1, iter2, ptr);
			start = ptr;
			finish = EndStorage = ptr + iter2 - iter1;
		}
		//重新为容器分配内存,并将原来容器中的元素转移到新的内存之中,由于采用C++11故这里我们采用
		//元素移动的方式进行相应的操作
		//本函数首先要判断当前内存是否已经满,然后再考虑是否需要进行分配内存和元素的移动

		void free()//此操作用于释放当前容器的内存空间
		{
			if (start != nullptr)//容器为空无法释放
			{
				for (auto iter = start; iter != finish; iter++)
				{
					alloc::destroy(iter);
				}
				alloc::deallocate(start, EndStorage - start);
			}
			start = finish = EndStorage = nullptr;
		}
		void reallocate(size_type n = 1)
		{
			//分两种情况来考虑,如果插入或者新增元素为一个
			//对于重新扩充内存后统一采用元素移动的方式来构造新的对象
			if (finish + n>EndStorage)
			{
				if (n == 1)
				{
				
					auto count = size();//记录当前容器的容量
					auto AddStorage = count ? 2 * size() : 1;//如果是初始化的容器则新增容量为1，否则为size的两倍
					
					auto NewPtr = alloc::allocate(AddStorage);
					
					auto tmp = NewPtr;

					for (auto iter = start; iter != finish; iter++)
					{
						
						alloc::construct(tmp, std::move(*iter));
						tmp++;
					}
					free();
					start = NewPtr;
					finish = tmp;
					EndStorage = NewPtr + AddStorage;
				}
				else//新增内存大于1的情况
				{
					auto AddStorage = size()>n ? 2 * size() : size() + n;
					auto NewPtr = alloc::allocate(AddStorage);
					auto tmp = NewPtr;

					for (auto iter = start; iter != finish; iter++)
					{
						alloc::construct(tmp, std::move(*iter));
						tmp++;
					}
					free();
					start = NewPtr;
					finish = tmp;
					EndStorage = NewPtr + AddStorage;
				}
			}
		}
		//类似于MiniVector提供和通用STL的相同接口

	public:
		//容器的所有构造函数
		MiniVector() :start(nullptr), finish(nullptr), EndStorage(nullptr){}//默认构造函数
		//默认构造不分配内存

		MiniVector(initializer_list<value_type> list)//列表初始化
		{
			fill_init(list.begin(), list.end());
		}

		MiniVector(size_type n)//对n个元素进行默认值初始化
		{
			//传入默认值即可
			fill_init(n, value_type());//对n个元素采用默认初始化
		}

		MiniVector(size_type n, const value_type& value)//将n个元素全部初始化为给定的类型
		{
			fill_init(n, value);
		}

		MiniVector(iterator lhs, iterator rhs)//将容器初始化为两个迭代器之间的元素
		{
			//先直接利用空间适配器来进行构造，后续再修改为全局的内存分配函数
			auto n = rhs - lhs;
			auto ptr = alloc::allocate(n);
			start = ptr;
			finish = ptr + n;
			EndStorage = finish;
			while (n--)
			{
				alloc::construct(ptr, *lhs);
				ptr++;
				lhs++;
			}

		}

		//有关移动构造函数和拷贝构造函数以及赋值构造函数后续补充
		//拷贝构造函数
		MiniVector(const MiniVector &vec)
		{
			fill_init(vec.begin(), vec.end());//创建新的内存空间,并用vec的元素去初始化该容器
		}

		//赋值构造函数
		//应该释放原来的内存,然后分配新的内存
		MiniVector& operator=(const MiniVector &vec)
		{
			if (this != &vec)
			{
				free();
				fill_init(vec.begin(), vec.end());
			}
			return *this;
		}

		//移动构造函数
		//移动构造函数只是接管了原容器的内存,但是不会析构掉原容器
		MiniVector(MiniVector&& vec) :start(vec.start), finish(vec.finish), EndStorage(vec.EndStorage)
		{
			vec.start = nullptr;
			vec.finish = nullptr;
			vec.EndStorage = nullptr;
		}

		MiniVector& operator=(MiniVector &&vec)//接管内存,释放空间
		{

			//只考虑非自赋值的情况下采用先释放自己的空间再接收其他空间
			if (this != &vec)
			{
				free();
				start = vec.start;
				finish = vec.finish;
				EndStorage = vec.EndStorage;
				vec.start = nullptr;
				vec.finish = nullptr;
				vec.EndStorage = nullptr;

			}
			return *this;
		}

		~MiniVector()//析构函数
		{
			clear();
		}

	public:
		//插入元素
		//在插入元素时，要考虑到内存不足的情况,当每次申请内存时要考虑内存增量的问题

		/*void push_back(const value_type& value)
		{
			//首先利用reallocate函数保证容器中有充足的内存
			reallocate();
			
			alloc::construct(finish, value);
			finish++;
		}

		//移动元素,并不是不用构造新的容器,而是针对该元素的移动,只是将该元素移动到了相应的内存
		//该元素还是保存在相应的内存当中的
		void push_back(value_type&& value)
		{
			reallocate();
			alloc::construct(finish, std::forward<value_type>(value));//保持右值不变
			finish++;
		}*/
		//插入元素
		//在插入元素时，要考虑到内存不足的情况,当每次申请内存时要考虑内存增量的问题
		//利用模板编程技术,将移动插入和普通插入合二为一,通过模板参数U去推断左值和右值
		template<typename U>
		void push_back(U&& value)
		{
			reallocate();

			alloc::construct(finish, std::forward<U>(value));
			finish++;
		}

		template<typename... Args>
		void emplace_back(Args&&... args)//在尾部构造一个元素,采用可变参数模板的形式去构造
		{
			reallocate();
			alloc::construct(finish, std::forward<Args>(args)...);
			finish++;
		}

		/*iterator insert(iterator iter, const value_type& value)
		{
			if (iter == finish)
			{
				push_back(value);
			}
			else
			{
				reallocate();
				auto OldFinish = finish;
				alloc::construct(OldFinish, *(OldFinish - 1));
				finish++;
				//cout << "testing" << endl;
				copy(iter, OldFinish - 1, iter + 1);//代码错误,iter所指向的内存已经被释放
				cout << "testing" << endl;
				fill_n(iter, 1, value);
			}
			return iter;

		}

		iterator insert(iterator iter, value_type&& value)//移动插入而不是直接插入
		{
			if (iter == finish)
			{
				push_back(std::forward<value_type>(value));
			}
			else
			{
				reallocate();
				auto OldFinish = finish;
				alloc::construct(OldFinish, *(OldFinish - 1));
				finish++;
				copy(iter, OldFinish - 1, iter + 1);
				fill_n(iter, 1, std::forward<value_type>(value));//移动操作
			}
			return iter;
		}*/
	public:
		//都是一样的写法,应当先判断容器可用容量是否充足,而不是分类讨论
		template <typename U>//利用模板来实现移动和赋值操作的合二为一
		iterator insert(iterator iter, U&& value)
		{
			size_type n = 1;
			size_type Velement = finish - iter;
			size_type Vcapacity = EndStorage - finish;
			auto OldFinish = finish;
			auto result = iter;
			if (Vcapacity >= n)
			{
				if (Velement > n)
				{
					uninitialized_copy(OldFinish - n, OldFinish, finish);
					finish += n;
					copy(iter, OldFinish - n, OldFinish + 1);
			//		fill_n(iter, n, std::forward<U>(value));
					*iter = std::forward<U>(value);
				}
				else
				{
					//uninitialized_fill_n(OldFinish, n - Velement, std::forward<U>(value));
					alloc::construct(OldFinish, std::forward<U>(value));
					finish = finish + n;
				}
			}
			else//内存不足又当如何
			{
				size_type AddStorage = size() > n ? 2 * size() : (size() + n);
				auto NewPtr = alloc::allocate(AddStorage);
				size_type count = 0;
				for (auto p = start; p != iter; p++)
				{
					alloc::construct(NewPtr + count, std::move(*p));
					count++;
				}
				auto result = NewPtr + count;
				//uninitialized_fill_n(NewPtr + count, n,std::forward<U>(value));
				alloc::construct(NewPtr + count, std::forward<U>(value));
				for (auto p = iter; p != finish; p++)
				{
					alloc::construct(NewPtr + count + n, std::move(*p));
					count++;
				}
				free();
				start = NewPtr;
				finish = NewPtr + count + n;
				EndStorage = start + AddStorage;
			}
			return result;
		}

		template<typename... Args>
		iterator emplace(iterator iter, Args&&... args)
		{
			if (iter == finish)
			{
				emplace_back(std::forward<Args>(args)...);//直接在原始内存上进行构造
			}
			else
			{
				reallocate();//首先确保内存足够
				//	uninitialized_copy(iter,finish,iter+1);
				//	alloc::construct(iter,std::forward<Args>(args)...);
				//	finish++;
				auto OldFinish = finish;
				alloc::construct(finish, *(finish - 1));
				finish++;
				copy(iter, OldFinish - 1, iter + 1);
				alloc::destroy(iter);
				alloc::construct(iter, std::forward<Args>(args)...);//这里由于是利用模板,所以可以直接考虑左值和右值两种情况
			}
			return iter;
		}

		iterator insert(iterator iter, size_type n,  value_type value)//将n个插入 
		{

			size_type Velement = finish - iter;
			size_type Vcapacity = EndStorage - finish;
			auto OldFinish = finish;
			auto result = iter;
			if (n > 0)
			{
				if (Vcapacity >= n)
				{
					if (Velement > n)
					{
						uninitialized_copy(OldFinish - n, OldFinish, finish);
						finish += n;
						copy(iter, OldFinish - n, OldFinish + 1);
						fill_n(iter, n, value);
					}
					else
					{
						uninitialized_fill_n(OldFinish, n - Velement, value);
						finish = finish + n - Velement;
						uninitialized_copy(OldFinish - Velement, OldFinish, finish);
						finish += Velement;
						fill_n(OldFinish - Velement, Velement, value);
					}
				}
				else//内存不足又当如何
				{
					size_type AddStorage = size() > n ? 2 * size() : size() + n;
					iterator NewPtr = alloc::allocate(AddStorage);
					size_type count = 0;
					for (auto p = start; p != iter; p++)
					{
						alloc::construct(NewPtr+count, std::move(*p));
						count++:
					}
					auto result = NewPtr + count;
					uninitialized_fill_n((NewPtr+count),n,value);
					for (auto p = iter; p != finish; p++)
					{
						alloc::construct(NewPtr + count+n, std::move(*p));
						count++;
					}
					free();
					start = NewPtr;
					finish = NewPtr + count + n;
					EndStorage = start + AddStorage;
				}
			}
			return result;
		}

		iterator insert(iterator iter1, iterator iter2, iterator iter3)
		{
			//将iter2到iter3之间的元素拷贝到iter1开始的区域
			size_type n = iter3 - iter2;
			size_type Velement = finish - iter1;
			size_type Vcapacity = EndStorage - finish;
			auto OldFinish = finish;
			auto result=iter1;
			if (n>0)
			{
				if (Vcapacity >= n)
				{
					if (n > Velement)
					{
						uninitialized_copy(iter2 + Velement, iter3, OldFinish);
						finish = finish + n - Velement;
						uninitialized_copy(iter1, OldFinish, iter1 + n);
						finish += Velement;
						copy(iter2, iter2 + Velement, iter1);
					}
					else
					{
						uninitialized_copy(OldFinish - n, OldFinish, OldFinish);
						finish += n;
						copy(iter1, OldFinish - n, iter1 + n);
						copy(iter2, iter3, iter1);
					}
				}
				else
				{
					size_type AddStorage = size() > n ? 2 * size() : size() + n;
					auto NewPtr = alloc::allocate(AddStorage);
					size_type count = 0;
					for (auto p = start; p != iter1; p++)
					{
						alloc::construct(NewPtr + count, std::move(*p));
						count++;
					}
					auto result = NewPtr + count;
					uninitialized_copy(iter2,iter3,result);
					for (auto p = iter1; p != finish; p++)
					{
						alloc::construct(NewPtr + count + n, std::move(*p));
						count++;
					}
					free();
					start = NewPtr;
					finish = NewPtr + count + n;
					EndStorage = start + AddStorage;
				}
				
			}
			return result;
		}

		iterator insert(iterator iter,initializer_list<value_type> list)//初始化列表类型，插入整个初始化列表
		{
			insert(iter, list.begin(), list.end());//重载函数解决代码冗余问题
		}

	public:
		//访问元素,均返回对元素的引用
		reference back()const
		{
			return *start;
		}

		reference front()const
		{
			return *(finish - 1);
		}

		reference operator[](size_type n)const
		{//下标运算符不考虑越界异常的问题
	
			return *(start + n);
		}

		reference at(size_type n)const
		{
			//此处应该考虑越界的异常问题
			size_type size = finish - start;
			if (n >= size || n < 0)
				throw out_of_range("MiniVector out_of_range");
			return *(start + n);
		}

	public:
		//删除元素操作

		void pop_back()
		{
			//析构尾部元素
			alloc::destroy(finish - 1);
			finish--;
		}

		iterator erase(iterator iter)
		{
			//删除某一元素需要将之后的元素进行一个整体上的移动
			//alloc::destroy(iter);
			copy(iter + 1, finish, iter);
			finish--;
			alloc::destroy(finish);

		}

		iterator erase(iterator lhs, iterator rhs)
		{
			auto n = rhs - lhs;
			copy(rhs, finish, lhs);
			finish -= n;
			for (auto i = 0; i<n; i++)
			{
				alloc::destroy(finish + i);
			}
		}

		void clear()//清空整个容器
		{
			free();
		}

	public:
		//容器大小相关的操作
		//resize用于改变容器中元素的个数,如果容器容量大于n则容器的size会缩小为n,否则会扩充容器容量到n
		//当有多个重载函数时，应该考虑默认参数

		void reserve(size_type n)//将capacity变为n(如果n比capacity大)如果小则不采取任何操作
		{
			size_type Vcapacity = EndStorage - start;
			if (n>Vcapacity)
			{
				auto NewPtr = alloc::allocate(n);
				auto tmp = NewPtr;
				for (auto iter = start; iter != finish; iter++)
				{
					alloc::construct(tmp, std::move(*iter));
					tmp++;
				}
				free();
				start = NewPtr;
				finish = tmp;
				EndStorage = NewPtr + n;
			}
		}

		void resize(size_type n, const value_type& value = value_type())
		{//将value默认值设定为value_type()	
			size_type Vsize = finish - start;
			size_type Vcapacity = EndStorage - start;
			if (n <= Vcapacity)
			{
				if (n <= Vsize)
				{
					auto OldFinish = finish;
					finish = finish - (Vsize - n);
					for (auto iter = finish; iter != OldFinish; iter++)
					{
						alloc::destroy(iter);
					}
				}
				else
				{
					uninitialized_fill_n(finish, n - Vsize, value);
					finish += (n - Vsize);
				}
			}
			else
			{
				reserve(n);
				uninitialized_fill(finish, EndStorage, value);
				finish = EndStorage;
			}
		}

		void shrink_to_fit()//将capacity减少到和size一样大
		{
			//回收finish后的所有内存
			if (finish != EndStorage)
			{
				alloc::deallocate(finish, EndStorage - finish);
				EndStorage = finish;
			}

		}

		size_type size()const //返回的是当前元素个数
		{
			return (size_type)(finish - start);
		}

		size_type capacity()const//返回容器可容纳元素个数
		{
			return (size_type)(EndStorage - start);
		}



	public:
		//访问迭代器的操作

		iterator begin()const
		{
			return start;
		}

		iterator end()const
		{
			return finish;
		}


	};
}


#endif
