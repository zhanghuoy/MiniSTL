//�𲽲���C++11��д,����ȫ������C++11
//��д����������
/*
����MiniVector��ӵĵط�Ī�����ڴ�����·���,���ڴ��������Ԫ��ʱ��Ҫ�����ڴ��Ƿ��������,
����ڴ治�㣬��Ҫ�������ط����������µĵ��ڴ�,����ԭʼ�ڴ��е�Ԫ�ؿ��������·�����ڴ��У�
Ȼ���ͷŷ������ڴ棬Ϊ��Ӧ��C++11ͬʱ��Ҫ�����ƶ�������ʹ��
*/
#ifndef MINIVECTOR_H
#define MINIVECTOR_H
#define NOMINMAX
#include<initializer_list>
#include<exception>
#include<cstddef>
#include<algorithm>
#include"MiniAllocator.h"
//���ƶ�Ԫ��ʱ.��Ҫ���ǵ�fill��copy�����Ƕ��Ѿ����й�������ڴ���и�ֵ�滻,
//��unitialize_copy�Ƕ�δ���г�ʼ�����ڴ���г�ʼ��,��������ߵ�ʹ�÷�������Ҫ

//fill������begin��end��Χ�ڵ�����Ԫ�ظ�ֵΪvalue
//fill_n������begin��ʼ��n��Ԫ�ط�Χ��ֵΪvalue
//copy������begin��end��Χ�ڵ�Ԫ�ؿ�����result��ʼ��������

//copy��ʹ�ù�����ʵ�����ǵ��õ���ĸ�ֵ���캯��


//���ຯ������������ڵ�ǰ�������߸�ֵ���ڴ��Ƿ��Ѿ���ʼ����

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
		//����ΪǶ�����Ͷ���

	private:
		//�ڲ�ʹ�õ��ڴ���亯��
		iterator start;//��ʼ�ڴ�λ��
		iterator finish;//��ǰ�ڴ��ʹ��ĩβ(���һ��֮��)
		iterator EndStorage;//�ڴ����λ��

		void fill_init(size_type n, const value_type &value)//��ʼ������ڴ�ռ��С,�ɹ��캯������
		{
			auto ptr = alloc::allocate(n);//������СΪn�Ŀռ�
			start = ptr;
			finish = ptr + n;
			EndStorage = finish;
			//�������Ŀռ��ʼ��ΪT
			while (n--)
			{
				alloc::construct(ptr, value);//����Ԫ��
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
		//����Ϊ���������ڴ�,����ԭ�������е�Ԫ��ת�Ƶ��µ��ڴ�֮��,���ڲ���C++11���������ǲ���
		//Ԫ���ƶ��ķ�ʽ������Ӧ�Ĳ���
		//����������Ҫ�жϵ�ǰ�ڴ��Ƿ��Ѿ���,Ȼ���ٿ����Ƿ���Ҫ���з����ڴ��Ԫ�ص��ƶ�

		void free()//�˲��������ͷŵ�ǰ�������ڴ�ռ�
		{
			if (start != nullptr)//����Ϊ���޷��ͷ�
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
			//���������������,��������������Ԫ��Ϊһ��
			//�������������ڴ��ͳһ����Ԫ���ƶ��ķ�ʽ�������µĶ���
			if (finish + n>EndStorage)
			{
				if (n == 1)
				{
				
					auto count = size();//��¼��ǰ����������
					auto AddStorage = count ? 2 * size() : 1;//����ǳ�ʼ������������������Ϊ1������Ϊsize������
					
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
				else//�����ڴ����1�����
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
		//������MiniVector�ṩ��ͨ��STL����ͬ�ӿ�

	public:
		//���������й��캯��
		MiniVector() :start(nullptr), finish(nullptr), EndStorage(nullptr){}//Ĭ�Ϲ��캯��
		//Ĭ�Ϲ��첻�����ڴ�

		MiniVector(initializer_list<value_type> list)//�б��ʼ��
		{
			fill_init(list.begin(), list.end());
		}

		MiniVector(size_type n)//��n��Ԫ�ؽ���Ĭ��ֵ��ʼ��
		{
			//����Ĭ��ֵ����
			fill_init(n, value_type());//��n��Ԫ�ز���Ĭ�ϳ�ʼ��
		}

		MiniVector(size_type n, const value_type& value)//��n��Ԫ��ȫ����ʼ��Ϊ����������
		{
			fill_init(n, value);
		}

		MiniVector(iterator lhs, iterator rhs)//��������ʼ��Ϊ����������֮���Ԫ��
		{
			//��ֱ�����ÿռ������������й��죬�������޸�Ϊȫ�ֵ��ڴ���亯��
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

		//�й��ƶ����캯���Ϳ������캯���Լ���ֵ���캯����������
		//�������캯��
		MiniVector(const MiniVector &vec)
		{
			fill_init(vec.begin(), vec.end());//�����µ��ڴ�ռ�,����vec��Ԫ��ȥ��ʼ��������
		}

		//��ֵ���캯��
		//Ӧ���ͷ�ԭ�����ڴ�,Ȼ������µ��ڴ�
		MiniVector& operator=(const MiniVector &vec)
		{
			if (this != &vec)
			{
				free();
				fill_init(vec.begin(), vec.end());
			}
			return *this;
		}

		//�ƶ����캯��
		//�ƶ����캯��ֻ�ǽӹ���ԭ�������ڴ�,���ǲ���������ԭ����
		MiniVector(MiniVector&& vec) :start(vec.start), finish(vec.finish), EndStorage(vec.EndStorage)
		{
			vec.start = nullptr;
			vec.finish = nullptr;
			vec.EndStorage = nullptr;
		}

		MiniVector& operator=(MiniVector &&vec)//�ӹ��ڴ�,�ͷſռ�
		{

			//ֻ���Ƿ��Ը�ֵ������²������ͷ��Լ��Ŀռ��ٽ��������ռ�
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

		~MiniVector()//��������
		{
			clear();
		}

	public:
		//����Ԫ��
		//�ڲ���Ԫ��ʱ��Ҫ���ǵ��ڴ治������,��ÿ�������ڴ�ʱҪ�����ڴ�����������

		/*void push_back(const value_type& value)
		{
			//��������reallocate������֤�������г�����ڴ�
			reallocate();
			
			alloc::construct(finish, value);
			finish++;
		}

		//�ƶ�Ԫ��,�����ǲ��ù����µ�����,������Ը�Ԫ�ص��ƶ�,ֻ�ǽ���Ԫ���ƶ�������Ӧ���ڴ�
		//��Ԫ�ػ��Ǳ�������Ӧ���ڴ浱�е�
		void push_back(value_type&& value)
		{
			reallocate();
			alloc::construct(finish, std::forward<value_type>(value));//������ֵ����
			finish++;
		}*/
		//����Ԫ��
		//�ڲ���Ԫ��ʱ��Ҫ���ǵ��ڴ治������,��ÿ�������ڴ�ʱҪ�����ڴ�����������
		//����ģ���̼���,���ƶ��������ͨ����϶�Ϊһ,ͨ��ģ�����Uȥ�ƶ���ֵ����ֵ
		template<typename U>
		void push_back(U&& value)
		{
			reallocate();

			alloc::construct(finish, std::forward<U>(value));
			finish++;
		}

		template<typename... Args>
		void emplace_back(Args&&... args)//��β������һ��Ԫ��,���ÿɱ����ģ�����ʽȥ����
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
				copy(iter, OldFinish - 1, iter + 1);//�������,iter��ָ����ڴ��Ѿ����ͷ�
				cout << "testing" << endl;
				fill_n(iter, 1, value);
			}
			return iter;

		}

		iterator insert(iterator iter, value_type&& value)//�ƶ����������ֱ�Ӳ���
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
				fill_n(iter, 1, std::forward<value_type>(value));//�ƶ�����
			}
			return iter;
		}*/
	public:
		//����һ����д��,Ӧ�����ж��������������Ƿ����,�����Ƿ�������
		template <typename U>//����ģ����ʵ���ƶ��͸�ֵ�����ĺ϶�Ϊһ
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
			else//�ڴ治���ֵ����
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
				emplace_back(std::forward<Args>(args)...);//ֱ����ԭʼ�ڴ��Ͻ��й���
			}
			else
			{
				reallocate();//����ȷ���ڴ��㹻
				//	uninitialized_copy(iter,finish,iter+1);
				//	alloc::construct(iter,std::forward<Args>(args)...);
				//	finish++;
				auto OldFinish = finish;
				alloc::construct(finish, *(finish - 1));
				finish++;
				copy(iter, OldFinish - 1, iter + 1);
				alloc::destroy(iter);
				alloc::construct(iter, std::forward<Args>(args)...);//��������������ģ��,���Կ���ֱ�ӿ�����ֵ����ֵ�������
			}
			return iter;
		}

		iterator insert(iterator iter, size_type n,  value_type value)//��n������ 
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
				else//�ڴ治���ֵ����
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
			//��iter2��iter3֮���Ԫ�ؿ�����iter1��ʼ������
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

		iterator insert(iterator iter,initializer_list<value_type> list)//��ʼ���б����ͣ�����������ʼ���б�
		{
			insert(iter, list.begin(), list.end());//���غ������������������
		}

	public:
		//����Ԫ��,�����ض�Ԫ�ص�����
		reference back()const
		{
			return *start;
		}

		reference front()const
		{
			return *(finish - 1);
		}

		reference operator[](size_type n)const
		{//�±������������Խ���쳣������
	
			return *(start + n);
		}

		reference at(size_type n)const
		{
			//�˴�Ӧ�ÿ���Խ����쳣����
			size_type size = finish - start;
			if (n >= size || n < 0)
				throw out_of_range("MiniVector out_of_range");
			return *(start + n);
		}

	public:
		//ɾ��Ԫ�ز���

		void pop_back()
		{
			//����β��Ԫ��
			alloc::destroy(finish - 1);
			finish--;
		}

		iterator erase(iterator iter)
		{
			//ɾ��ĳһԪ����Ҫ��֮���Ԫ�ؽ���һ�������ϵ��ƶ�
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

		void clear()//�����������
		{
			free();
		}

	public:
		//������С��صĲ���
		//resize���ڸı�������Ԫ�صĸ���,���������������n��������size����СΪn,�������������������n
		//���ж�����غ���ʱ��Ӧ�ÿ���Ĭ�ϲ���

		void reserve(size_type n)//��capacity��Ϊn(���n��capacity��)���С�򲻲�ȡ�κβ���
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
		{//��valueĬ��ֵ�趨Ϊvalue_type()	
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

		void shrink_to_fit()//��capacity���ٵ���sizeһ����
		{
			//����finish��������ڴ�
			if (finish != EndStorage)
			{
				alloc::deallocate(finish, EndStorage - finish);
				EndStorage = finish;
			}

		}

		size_type size()const //���ص��ǵ�ǰԪ�ظ���
		{
			return (size_type)(finish - start);
		}

		size_type capacity()const//��������������Ԫ�ظ���
		{
			return (size_type)(EndStorage - start);
		}



	public:
		//���ʵ������Ĳ���

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
