//д��һ���������ܽ�һ�����������������𣬳����÷��ȵȡ�
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
	typedef ptrdiff_t DifferentType;//���ڼ�������������֮��ľ��룬����
	typedef size_t SizeType;
	typedef ListNode_<T>* ListNodePtr;

	ListNodePtr node;//һ��ָ��ڵ��ָ�룬����������ʵ�����ǶԸ�ָ���һ����װ
	 
	//�����Ĳ�����������������صȵ�
	iterator_(){}
	iterator_(ListNodePtr x) :node(x){}//������ʽ���캯���ķ�ʽ���ڸ�ֵ�˵�����ʱֱ���ԵȺŽ��и�ֵ
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
	pointer operator->()const//���ؼ�ͷ�����ʱ���Ƿ���.�������
	{
		return &(operator*());
	}

	self& operator++()//ǰ�������
	{
		node = (ListNodePtr)node->next;
		return *this;
	}
	self& operator--()
	{
		node = (ListNodePtr)node->prev;
		return *this;
	}
	self operator++(int)//��������������صĻ��ǵ���ǰ�Ķ��󣬵��ǲ��ܷ������ã���ΪtmpΪ��ʱ�ļ���
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

		typedef T						value_type;//���ܻ���Ƶ������ľ���͵�����֮�������ȵ�
		typedef T&						reference;
		typedef	iterator_<T, T&, T*>	iterator;
		typedef T*						pointer;
		typedef	const T&				const_reference;
		typedef size_t					size_type;
		typedef ListNode_<T>*			link_type;

	private:
		link_type node;//һ��ָ�룬ָ��β�˵�һ���հ׽ڵ㣬ʹ��nodeָ�뼴Ϊend()
	protected:
		typedef ListNode_<T> ListNode;

		link_type get_node()//����һ�����ӽڵ�
		{
			return Alloc::allocate();
		}
		void put_node(link_type ptr)//�ͷŵ�һ���ڵ�
		{
			Alloc::deallocate(ptr);
		}
		link_type create_node(const T& x)//����һ���ڵ㣬���Խڵ�����ݽ��и�ֵ
		{
			auto tmp = get_node();
			Alloc::construct(&tmp->data, x);//ע�⵱����ĳ���ڵ�󣬽ڵ��data�ᱻ�Զ�����Ĭ��ֵ�����Թ����ָ��Ӧ��Ϊ&tmp->data
			return tmp;
		}
		void destroy_node(link_type ptr)//����һ���ڵ㲢�������ڵ������
		{
			Alloc::destroy(&ptr->data);
			put_node(ptr);
		}//��������Ϊ�Ĵ�Ԥ������

		void empty_init()
		{
			node = get_node();
			node->next = node;
			node->prev = node;
		}

		void transfer(iterator position, iterator first, iterator last)//first��last��һ��ǰ�պ󿪵�����
		{//��first��last֮��Ľڵ�ת�Ƶ�position֮ǰ�������ǲ�ͬ��list֮�䣬Ҳ��������ͬlist֮��
			link_type tmp = position.node->prev;
			last.node->prev->next = position;
			position.node->prev = last.node->prev;
			last.node->prev = first.node->prev;
			first.node->prev->next = last;
			tmp->next = first;
			first.node->prev = tmp;
		}
	public:
		//���캯��
		list()//����һ���յ�node����ֻ���Ǹ���node
		{
			empty_init();
		}
		list(const size_type& n, const value_type& value = value_type())//�����СΪn������,����ÿ���ڵ㸳ֵΪvalue
		{
			empty_init();
			auto ptr1 = node;
			for (size_t i = 1; i <= n; i++)
			{
				auto ptr2 = create_node(value);//ע��list��ѭ������
				ptr1->next = ptr2;
				ptr2->prev = ptr1;
				ptr2->next = node;
				node->prev = ptr2;
				ptr1 = ptr2;
			}
			
		}

		list(const operator &lhs, const operator& rhs)//�Ե�����lhs-rhs��Χ�����ݹ���
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

		list(const list& list_)//���ڽ��ظ��Ĳ���д��һ���������Թ�����ã����������ĺ��������private���͵ĺ���
		{
			/*empty_init();//���ȴ���һ��������ͷ�ڵ������
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

		list(list&&list_)//��ν�ƶ���������Ϊ����ָ��Ĳ����������½��������ָ��ָ����Ĳ���������ԭ����ָ����Ϊ��
		{
			node = list_.node;
			list_.node = nullptr;
		}

		list& operator=(const list& list_)//�����ж��Ƿ�Ϊ�Ը�ֵ�������������������ͷŵ�ԭ���Ŀռ�,Ȼ�������¹���һ������
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

		//�������
		iterator insert(iterator iter, const T& value)//�����ʲôʱ����const ʲôʱ����const
		{
			auto ptr = create_node(value);//����list���ԣ��ڲ���ǰ�Ͳ���������������Ч��
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

		void push_back(const T& value)//β��
		{
			insert(end(),value);
		}

		void push_front(const T& value)//ͷ��
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

		//ɾ������
		iterator erase(iterator iter)//ɾ����������ָԪ��
		{
			iterator tmp(iter.node->next);
			iter.node->prev->next = iter.node->next;
			iter.node->next->prev = iter.node->prev;
			destroy_node(iter.node);//ɾ����ֻ�ǵ���������ָ��Ľڵ㣬��û��ɾ������������
			return tmp;
		}

		iterator erase(iterator lhs, iterator rhs)//�������ľ����������
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
		void clear()//������б��е�����Ԫ��
		{//ִ���������ʱ��������Ҫʹ�õ�����
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
		void remove(const T& value)//�Ƴ�����Ԫ��Ϊvalue�Ľڵ�
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
		void unique()//�Ƴ���ֵ��ͬ��������Ԫ��Ȼ��ֻ��һ��
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
		//��������,�������������е�Ԫ��
		void swap(list& x)//ʵ�����Ƕ�ָ��Ľ���
		{
			link_type tmp;
			tmp = x.node;
			x.node = node;
			node = x.node;
		}

		//Ǩ�Ʋ���
		void splice(iterator position, list& x)//��list x������Ԫ�ؽ�����position֮ǰ������x������this��
		{
			transfer(position, x.begin(), x.end());
		}
		void splice(iterator position, list&, iterator i)//��i��ָԪ�ؽ�����position֮ǰ,������ͬһ������
		{
			auto j = i;
			j++;
			if (position != i&&position != j)//ע�������i������end()�������Ȼ��߱�������ǰ�������ƶ�.
			{
				transfer(position, i, j);
			}
		}
		void splice(iterator postion, list&, iterator first, iterator last)//��first��last֮���Ԫ�ؽ�����position֮ǰ,������ͬһ������
		{
			if (first != last)
			{
				transfer(postion, first, last);
			} 
		}
		void merge(list& x)//���������x����ϲ������ǵ��ô˺�����ǰ���������������������Ԫ�ض��Ǵ�С�����ģ��Һϲ��������Ҳ�����ǵ�����
		{//�ڱ����Ĺ����У�ע�����list1����list2������ѭ�����ò�ȡ�κβ��������list1����list2����Ҫ��ѭ��������list2��ʣ�ಿ�ּ��뵽list1���档
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
					first1++;//�ҵ���һ����first2�����
			}
			if (first2 != last2)
				transfer(last1, first2, last2);
		}
		void reverse()//�������ڵ�Ԫ������
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

//������������÷���д���������ĳ�Ա�����ȵ�
//��ģ�����ڲ�ʹ��������ʾ��ʱ�����Բ���ģ����������Ǹ��������ֻ�ܺ�this��ͬ