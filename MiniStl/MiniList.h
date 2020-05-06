//写完一个容器就总结一个容器，包括其区别，常见用法等等。
#ifndef MINILIST_H
#define MINILIST_H
#include"MiniAllocator.h"
#include"iterator.h"
using namespace std;
using namespace MiniStl;
template <class T>
struct ListNode_
{
	typedef ListNode_<T>* ListNodePtr;
	ListNodePtr next;
	ListNodePtr prev;
	T data;
};
template < class T, class Ref=T&, class Ptr=T* >
struct iterator_
{
	typedef iterator_<T, T&, T*> iterator;
	typedef iterator_ < T, Ref, Ptr > self;

	typedef BidrectionalIteratorTag  IteratorCategory;
	typedef T ValueType;
	typedef Ref reference;
	typedef Ptr pointer;
	typedef ptrdiff_t DifferentType;//用于计算两个迭代器之间的距离，留疑
	typedef size_t SizeType;
	typedef ListNode_<T>* ListNodePtr;

	ListNodePtr node;//一个指向节点的指针，整个迭代器实际上是对该指针的一个封装
	 
	//常见的操作，包括构造和重载等等
	iterator_(){}
	iterator_(ListNodePtr x) :node(x){}//采用隐式构造函数的方式，在赋值此迭代器时直接以等号进行赋值
	iterator_(const iterator& x) :node(x.node){}

	bool operator==(const self& x)const
	{
		return node == x.node;
	}
	bool operator!=(const self& x)const
	{
		return node != x.node;
	}
	reference operator*()const
	{
		return (*node).data;
	}
	pointer operator->()const//重载箭头运算符时，是返回.运算符的
	{
		return &(operator*());
	}

	self& operator++()//前置运算符
	{
		node = (ListNodePtr)node->next;
		return *this;
	}
	self& operator--()
	{
		node = (ListNodePtr)node->prev;
		return *this;
	}
	self operator++(int)//后置运算符，返回的还是递增前的对象，但是不能返回引用，因为tmp为临时文件。
	{
		self tmp = *this;
		++*this;
		return tmp;
	}
	self operator--(int)
	{
		self tmp = *this;
		--*this;
		return tmp;
	}
	DifferentType operator-(operator& iter)
	{
		
	}
	void operator+(SizeType& n)
	{
		
	}
	void operator-(SizeType& n)
	{
		
	}
};
namespace MiniStl
{
	template <class T,class Alloc=MiniAllocator<ListNode_<T> > >
	class list
	{
	public:

		typedef T						value_type;//可能还设计迭代器的距离和迭代器之间的运算等等
		typedef T&						reference;
		typedef	iterator_<T, T&, T*>	iterator;
		typedef T*						pointer;
		typedef	const T&				const_reference;
		typedef size_t					size_type;
		typedef ListNode_<T>*			link_type;

	private:
		link_type node;//一个指针，指向尾端的一个空白节点，使得node指针即为end()
	protected:
		typedef ListNode_<T> ListNode;

		link_type get_node()//创建一个链接节点
		{
			return Alloc::allocate();
		}
		void put_node(link_type ptr)//释放掉一个节点
		{
			Alloc::deallocate(ptr);
		}
		link_type create_node(const T& x)//创建一个节点，并对节点的数据进行赋值
		{
			auto tmp = get_node();
			Alloc::construct(&tmp->data, x);//注意当创建某个节点后，节点的data会被自动赋予默认值，所以构造的指针应该为&tmp->data
			return tmp;
		}
		void destroy_node(link_type ptr)//销毁一个节点并析构掉节点的数据
		{
			Alloc::destroy(&ptr->data);
			put_node(ptr);
		}//上述函数为四大预备函数

		void empty_init()
		{
			node = get_node();
			node->next = node;
			node->prev = node;
		}

		void transfer(iterator position, iterator first, iterator last)//first到last是一个前闭后开的区间
		{//将first到last之间的节点转移到position之前，可以是不同的list之间，也可以是相同list之间
			link_type tmp = position.node->prev;
			last.node->prev->next = position;
			position.node->prev = last.node->prev;
			last.node->prev = first.node->prev;
			first.node->prev->next = last;
			tmp->next = first;
			first.node->prev = tmp;
		}
	public:
		//构造函数
		list()//构造一个空的node，即只有那个空node
		{
			empty_init();
		}
		list(const size_type& n, const value_type& value = value_type())//构造大小为n的链表,并给每个节点赋值为value
		{
			empty_init();
			auto ptr1 = node;
			for (size_t i = 1; i <= n; i++)
			{
				auto ptr2 = create_node(value);//注意list是循环链表
				ptr1->next = ptr2;
				ptr2->prev = ptr1;
				ptr2->next = node;
				node->prev = ptr2;
				ptr1 = ptr2;
			}
			
		}

		list(const operator &lhs, const operator& rhs)//以迭代器lhs-rhs范围的数据构造
		{
			empty_init();
			auto ptr1 = node;
			while (lhs != rhs)
			{
				auto ptr2 = create_node(*lhs);
				ptr1->next = ptr2;
				ptr2->prev = ptr1;
				ptr2->next = node;
				node->prev = ptr2;
				ptr1 = ptr2;
				lhs++;
			}
		}

		list(const list& list_)//善于将重复的操作写成一个函数，以供其调用，但是这样的函数最好是private类型的函数
		{
			/*empty_init();//首先创建一个类似于头节点的链表
			auto ptr1 = node;
			for (auto iter = list_.begin(); iter != list_.end(); iter++)
			{
				auto ptr2 = create_node(*iter);
				ptr1->next = ptr2;
				ptr2->prev = ptr1;
				ptr2->next = node;
				node->prev = ptr2;
				ptr1 = ptr2;
			}*/
			list(list_.begin(), list_.end());
		}

		list(list&&list_)//所谓移动操作，即为交换指针的操作，即将新建的链表的指针指向传入的参数，而把原来的指针置为空
		{
			node = list_.node;
			list_.node = nullptr;
		}

		list& operator=(const list& list_)//首先判断是否为自赋值的情况，如果不是则先释放掉原来的空间,然后再重新构造一个链表
		{
			if (list_ != *this)
			{
				clear();
				list(list_.begin(), list_.end());
			}
			return *this;
		}

		list& operator=(list&& list_)
		{
			if (list_ != *this)
			{
				clear();
				node = list_.node;
				list_.node = nullptr;
			}
			return *this;
		}

		//插入操作
		iterator insert(iterator iter, const T& value)//搞清楚什么时候用const 什么时候不用const
		{
			auto ptr = create_node(value);//对于list而言，在插入前和插入后迭代器都是有效的
			ptr->next = iter.node;
			ptr->prev = iter.node->prev;
			iter.node->prev->next = ptr;
			iter.node->prev = ptr;
			return ptr;
		}

		iterator insert(iterator iter, size_type n, const T& value)
		{

		}

		iterator insert(iterator iter, const iterator& lhs, const iterator& rhs)
		{

		}

		void push_back(const T& value)//尾插
		{
			insert(end(),value);
		}

		void push_front(const T& value)//头插
		{
			insert(begin(),value);
		}

		
		iterator begin()const
		{
			return node->next;
		}

		iterator end()const
		{
			return node;
		}

		size_type size()const
		{
			return ;
		}

		bool empty()const
		{
			return node->next == node;
		}

		reference front()const
		{
			return *begin();
		}

		reference back()const
		{
			return *(--end());
		}

		//删除操作
		iterator erase(iterator iter)//删除迭代器所指元素
		{
			iterator tmp(iter.node->next);
			iter.node->prev->next = iter.node->next;
			iter.node->next->prev = iter.node->prev;
			destroy_node(iter.node);//删除的只是迭代器中所指向的节点，并没有删除迭代器对象
			return tmp;
		}

		iterator erase(iterator lhs, iterator rhs)//迭代器的距离操作问题
		{

		}

		void pop_back()
		{
			erase(--end());
		}

		void pop_front()
		{
			erase(begin());
		}
		void clear()//清理掉列表中的所有元素
		{//执行清理操作时，尽量不要使用迭代器
			auto cur = node->next;
			while (cur!=node)
			{
				auto tmp = cur;
				cur = cur->next;
				destroy_node(tmp);
			}
			node->next = node;
			node->prev = node;
		}
		void remove(const T& value)//移除所有元素为value的节点
		{
			for (auto iter = begin(); iter != end();)
			{
				if (*iter == value)
				{
					auto tmp = iter;
					iter++:
					erase(tmp);
				}
			}
		}
		void unique()//移除数值相同且连续的元素然后只留一个
		{
			for (auto iter = begin(); iter != end();)
			{
				if (iter.node->next->data == *iter)
				{
					auto tmp = iter;
					iter++;
					erase(tmp);
					continue;
				}
				iter++;
			}
		}
		//交换操作,交换两个链表中的元素
		void swap(list& x)//实际上是对指针的交换
		{
			link_type tmp;
			tmp = x.node;
			x.node = node;
			node = x.node;
		}

		//迁移操作
		void splice(iterator position, list& x)//将list x的所有元素接续到position之前，但是x不能是this。
		{
			transfer(position, x.begin(), x.end());
		}
		void splice(iterator position, list&, iterator i)//将i所指元素接续到position之前,可以是同一个链表
		{
			auto j = i;
			j++;
			if (position != i&&position != j)//注意这里的i不能是end()，如果相等或者本来就在前面则不用移动.
			{
				transfer(position, i, j);
			}
		}
		void splice(iterator postion, list&, iterator first, iterator last)//将first到last之间的元素接续到position之前,可以是同一个链表
		{
			if (first != last)
			{
				transfer(postion, first, last);
			} 
		}
		void merge(list& x)//将本链表和x链表合并，但是调用此函数的前提条件必须是两个链表的元素都是从小递增的，且合并后的链表也必须是递增的
		{//在遍历的过程中，注意如果list1长于list2则跳出循环后不用采取任何操作，如果list1短于list2则需要将循环跳出后list2的剩余部分加入到list1后面。
			auto first1 = begin();
			auto last1 = end();
			auto first2 = x.begin();
			auto last2 = x.end();
			while (first1 != last1 && first2 != last2)
			{
				if (*first2 < *first1)
				{
					auto next = first2;
					transfer(first1, first2, ++next);
					first2 = next;
				}
				else
					first1++;//找到下一个比first2大的数
			}
			if (first2 != last2)
				transfer(last1, first2, last2);
		}
		void reverse()//将链表内的元素逆置
		{
			auto first = begin();
			auto last = end();
			while (first != last)
			{
				transfer(first, first.data->next, last);
				last--;
				first = begin();
			}
		}
		/*void sort()
		{
				
		}*/

	};

}
#endif

//整理迭代器的用法，写法，常见的成员函数等等
//在模板类内部使用类名表示类时，可以不加模板参数，但是该类的类型只能和this相同