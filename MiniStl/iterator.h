//���ڶ������������ص���ȡ����
#ifndef ITERATOR_H
#define ITERATOR_H
namespace MiniStl
{
	struct InputIteratorTag{};
	struct OutputIteratorTag{};
	struct ForwardIteratorTag{};
	struct BidrectionalIteratorTag{};
	struct RandomIteratorTag{};

	template <class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
	struct Iterator
	{
		typedef Category		IteratorCategory;
		typedef T				ValueType;
		typedef Distance		DifferentType;
		typedef Pointer			pointer;
		typedef Reference		reference;
	};//Ϊ�˱�������Ƶ�����ʱ����©��Ӧ�����ͣ������Լ���Ƶĵ�����������̳д���

	template <class iterator>
	struct IteratorTraits
	{
		typedef typename iterator::IteratorCategory		IteratorCategory;
		typedef typename iterator::ValueType			ValueType;
		typedef typename iterator::DifferentType		DifferentType;
		typedef typename iterator::pointer				pointer;
		typedef typename iterator::reference			reference;
	};//��ȡ���͵�ͨ����ʽ

	template<class T>
	struct IteratorTraits < T* >
	{
		typedef RandomIterator	IteratorCategory;//����ָ�붼�����ָ��
		typedef T				ValueType;
		typedef ptrdiff_t		DifferentType;
		typedef T*				pointer;
		typedef T&				reference;
	};

	template<class T>
	struct IteratorTraits < const T* >
	{
		typedef RandomIterator	IteratorCategory;//����ָ�붼�����ָ��
		typedef T				ValueType;
		typedef ptrdiff_t		DifferentType;
		typedef const T*		pointer;
		typedef const T&		reference;
		//���������ǳ���ָ�����Ӧ��ָ�����ͺ���������ҲӦ���ǳ�������
	};

	//������ȡ������������
	template<class Iterator>
	typename IteratorTraits<Iterator>::IteratorCategory
		iterator_category(const Iterator& iterator)//������Ӧ�ĵ�����
	{
		typedef IteratorTraits<Iterator>::IteratorCategory Category;
		return Category();
	}

	//distance����
	template<class InputIterator>
	typename IteratorTraits<InputIterator>::DifferentType
		_distance(InputIterator begin, InputIterator end, InputIteratorTag)
	{
		IteratorTraits<InputIterator>::DifferentType n = 0;
		while (begin != end)
		{
			n++;
			begin++;
		}
		return n;
	}


	template<class RandomIterator>
	typename IteratorTraits<RandomIterator>::DifferentType
		_distance(RandomIterator begin, RandomIterator end, RandomIteratorTag)//Random�̳���Input����д�������������,����������Input
	{
		IteratorTraits<RandomIterator>::DifferentType n = end - begin;
		return n;
	}

	template<class Iterator>//�������ε����غ����Ĳ���
	typename IteratorTraits<Iterator>::DifferentType
		distance(Iterator begin, Iterator end)
	{
		typedef IteratorTraits<Iterator>::IteratorCategory Category;
		return _distance(begin, end, Category());
	}
	//һ��advance�������ڶԵ�������������
	template<class InputIterator, class Distance>
	void _advance(InputIterator &iter, Distance n, InputIteratorTag)
	{
		while (n--)
			iter++;
	}

	template<class BidrectionalIterator, class Distance>
	void _advance(BidrectionalIterator &iter, Distance n, BidrectionalIteratorTag)
	{
		if (n > 0)
		{
			while (n--)
				iter++;
		}
		else
		{
			while (n++)
				iter--;
		}
	}


	template<class RandomIterator, class Distance>
	void _advance(RandomIterator &iter, Distance n, RandomIteratorTag)
	{
		iter += n;
	}

	template<class Iterator, class Distance>
	void advance(Iterator &iter, Distance n)
	{
		typedef IteratorTraits<Iterator>::IteratorCategory Category;
		_advance(iter, n, Category());
	}
	//��ȡ�������ͺͲ������ͱ���
	template<class Iterator>
	typename IteratorTraits<Iterator>::DifferentType *
		different_type(const Iterator&)
	{
		return static_cast<typename IteratorTraits<Iterator>::DifferentType*>(0);
	}

	template<class Iterator>
	typename IteratorTraits<Iterator>::ValueType *
		value_type(const Iterator&)
	{
		return static_cast<typename IteratorTraits<Iterator>::ValueType*>(0);
	}
}
#endif
