//用于定义与迭代器相关的萃取功能
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
	};//为了避免在设计迭代器时，遗漏相应的类型，所有自己设计的迭代器都必须继承此类

	template <class iterator>
	struct IteratorTraits
	{
		typedef typename iterator::IteratorCategory		IteratorCategory;
		typedef typename iterator::ValueType			ValueType;
		typedef typename iterator::DifferentType		DifferentType;
		typedef typename iterator::pointer				pointer;
		typedef typename iterator::reference			reference;
	};//萃取类型的通用形式

	template<class T>
	struct IteratorTraits < T* >
	{
		typedef RandomIterator	IteratorCategory;//内置指针都是随机指针
		typedef T				ValueType;
		typedef ptrdiff_t		DifferentType;
		typedef T*				pointer;
		typedef T&				reference;
	};

	template<class T>
	struct IteratorTraits < const T* >
	{
		typedef RandomIterator	IteratorCategory;//内置指针都是随机指针
		typedef T				ValueType;
		typedef ptrdiff_t		DifferentType;
		typedef const T*		pointer;
		typedef const T&		reference;
		//如果传入的是常量指针则对应的指针类型和引用类型也应该是常量引用
	};

	//用于萃取迭代器的类型
	template<class Iterator>
	typename IteratorTraits<Iterator>::IteratorCategory
		iterator_category(const Iterator& iterator)//传入相应的迭代器
	{
		typedef IteratorTraits<Iterator>::IteratorCategory Category;
		return Category();
	}

	//distance函数
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
		_distance(RandomIterator begin, RandomIterator end, RandomIteratorTag)//Random继承自Input所以写这两种情况即可,其他均调用Input
	{
		IteratorTraits<RandomIterator>::DifferentType n = end - begin;
		return n;
	}

	template<class Iterator>//用于屏蔽掉重载函数的差异
	typename IteratorTraits<Iterator>::DifferentType
		distance(Iterator begin, Iterator end)
	{
		typedef IteratorTraits<Iterator>::IteratorCategory Category;
		return _distance(begin, end, Category());
	}
	//一组advance函数用于对迭代器进行运算
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
	//萃取距离类型和参数类型本身
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
