//既然是在申请内存,就一定要考虑到内存不足的情况
#ifndef ALLOC_H
#define ALLOC_H
#include<cstddef>
#include<cstdlib>
#include<iostream>
using namespace std;
const size_t BASE = 8;//基准量
const size_t MAX_BYTES = 128;//最大量
const size_t FREELIST_NUM = MAX_BYTES / BASE;//空闲链表数量
//一级配置器
/*
一级配置器在解决内存不足的情况时,采用了模拟new函数的处理方式
具体的操作是首先由开发者手动设定出错的处理方式malloc_hanlder,
然后当malloc函数返回NULL后调用malloc_again函数和malloc_hanlder
来处理此问题
*/
template <int inst>
class MallocAlloc
{
private:
	//当内存不足时通过本函数不断尝试配置和释放的任务
	//并返回重新尝试后分配的内存,由本函数和malloc_hanlder共同构成处理函数
	static void *malloc_again(size_t size);

	static void *realloc_again(void *ptr, size_t size);

	//声明一个内存不足的处理函数,当内存不足时通过此函数指针所
	//指向函数来处理此问题
	static void(*malloc_hanlder)();
public:
	static void* allocate(size_t size)
	{//当所申请的内存大小大于128时，调用一级适配器进行内存的分配与释放
		//直接采用malloc或者operator new的方式进行内存分配
		void* ptr = malloc(size);
		//		cout<<"malloc"<<endl;
		if (ptr == NULL)
		{
			ptr = malloc_again(size);
		}
		return ptr;
	}

	static void* reallocate(void *OldPtr, size_t size)
	{
		void *ptr = realloc(OldPtr, size);
		if (ptr == NULL)
		{
			ptr = realloc_again(OldPtr, size);
		}
		return ptr;
	}

	static void deallocate(void* ptr, size_t size)
	{
		free(ptr);//注意这里的ptr必须是上面allocate所分配的内存(且n为大小)
	}

	static auto set_hander(void(*f)())->void(*)()
	{//本函数用于用户指定出错处理函数，并返回其他上一次的出错处理函数
		void(*OldHander)() = malloc_hanlder;
		malloc_hanlder = f;
		return OldHander;
	}

};

template<int inst>
void(*MallocAlloc<inst>::malloc_hanlder)() = nullptr;

//再次分配内存
template<int inst>
void* MallocAlloc<inst>::malloc_again(size_t size)
{
	void(*my_hander)();
	void *result;
	while (1)
	{
		my_hander = malloc_hanlder;
		if (malloc_hanlder == nullptr)
		{
			;
		}
		(*my_hander)();//调用处理函数,其尝试去释放内存,若一次执行后内存还是不够,则再次执行
		if ((result = malloc(size)) != NULL)
		{
			return result;
		}
	}
}
template<int inst>
void* MallocAlloc<inst>::realloc_again(void *OldPtr, size_t size)
{
	void(*my_hander)();
	void *result;
	while (1)
	{
		my_hander = malloc_hanlder;
		if (malloc_hanlder == nullptr)
		{
			;//未定义的内存不足处理函数
		}
		(*my_hander)();
		if ((result = realloc(OldPtr, size)) != NULL)
		{
			return result;
		}
	}
}


//二级配置器
template<bool threads, int inst>
class DefaultAlloc
{
private:
	//此为构造空闲链表的节点,为了避免内存的浪费,这里采用共用体
	union NODE
	{
		union NODE *next;
		char data[1];
	};
private:

	//空闲链表的数量
	static NODE *FreeList[FREELIST_NUM];
	//指向内存池的起点
	static char *start;//将指针声明为char类型是为了保证在进行内存的加减操作时每次为一个字节
	//指向内存池的终点
	static char *end;

	static size_t heap;
	//当申请的内存不足8的整数倍时,返回8的整数倍
	static size_t round_up(size_t size)
	{
		return (size + BASE - 1)& ~(BASE - 1);//先对7取反,然后进行与运算
	}
	//返回申请字节对应数组的索引
	static size_t FreeList_index(size_t size)
	{
		return round_up(size) / BASE - 1;
	}

public:

	static void *allocate(size_t size);

	static void *reallocate(size_t size);
	//回收p所指向的size大小的内存
	static void deallocate(void *ptr, size_t size);
	//当空闲链表无空闲空间时，重新填充空闲链表,重新装填后任然返回的是,分配给用户的内存
	//实际填充过程由chunk函数来完成,只有chunk函数才会操作内存池
	static void* refill(size_t size);
	//从内存池获取内存,当内存池内存不足时调用一级空间配置器
	//返回从内存池中实际获取到的内存,并重新装填给对应的空闲链表
	static char *chunk(size_t size, size_t& AllocNum);


};

template<bool threads, int inst>
char* DefaultAlloc<threads, inst>::start = nullptr;

template<bool threads, int inst>
char* DefaultAlloc<threads, inst>::end = nullptr;

template<bool threads, int inst>
size_t DefaultAlloc<threads, inst>::heap = 0;

template<bool threads, int inst>
typename DefaultAlloc<threads, inst>::NODE* //对静态数组进行声明 ,访问模板类中的类型时，为了区分其和静态成员的区别
//需要在类型前面加上关键字typename
DefaultAlloc<threads, inst>::FreeList[FREELIST_NUM] = {
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr };

template<bool threads, int inst>
void* DefaultAlloc<threads, inst>::allocate(size_t size)
{
	if (size>MAX_BYTES)
	{
		return MallocAlloc<0>::allocate(size);//调用模板类记得加入参数
	}
	NODE** MyAlloc;
	NODE* result;
	MyAlloc = FreeList_index(size) + FreeList;//MyAlloc指向相应的空闲链表
	size = round_up(size);//将待分配内存扩充到8的最小整数倍
	if (*MyAlloc != nullptr)//若对应空闲链表不为空则取出并分配
	{
		result = *MyAlloc;
		*MyAlloc = (*MyAlloc)->next;
		return result;
	}
	else
	{
		//	cout<<"testing"<<endl;
		void *r = refill(round_up(size));//重新装填
		return r;
	}
}
//回收分配的内存块
template<bool threads, int inst>
void DefaultAlloc<threads, inst>::deallocate(void *ptr, size_t size)
{

	NODE** MyAlloc;
	NODE* FreeNode = (NODE*)ptr;
	MyAlloc = FreeList + FreeList_index(size);
	if (size>MAX_BYTES)//如果要求释放的字节数大于128则调用第一级配置器
	{
		MallocAlloc<0>::deallocate(ptr, size);
		return;
	}
	FreeNode->next = *MyAlloc;
	*MyAlloc = FreeNode;

}

template<bool threads, int inst>
void * DefaultAlloc<threads, inst>::reallocate(size_t size)
{
	return nullptr;
}

//当空闲链表空间不足,重新从内存池中填充空闲链表并将相应的内存分配给用户
template<bool threads, int inst>
void* DefaultAlloc<threads, inst>::refill(size_t size)
{//这里的size已经是8的整数倍
	size_t AllocNum = 20;//以引用的形式传入参数,因为其值可能会随内存池内存大小而变动
	NODE* result;
	NODE** MyAlloc;
	NODE* CurrentNode;
	NODE* NextNode;
	char* AllocResult = chunk(size, AllocNum);
	result = (NODE*)AllocResult;
	if (AllocNum == 1)//若最后只分配到了一个内存块则直接分配给用户,而不用再加入链表中
	{
		return result;
	}
	else
	{
		AllocResult += size;//当分配内存块多于一个时,需要将多于的内存块加入到空闲链表中
		MyAlloc = FreeList + FreeList_index(size);
		//	CurrentNode=*MyAlloc;//*MyAlloc实际是指向对应项的指针,
		*MyAlloc = CurrentNode = (NODE*)AllocResult;//数组指针的问题
		//注意数组的操作问题
		//	cout<<"refill"<<endl;
		while (AllocNum>2)
		{

			AllocResult += size;
			NextNode = (NODE*)AllocResult;
//--------------			CurrentNode->next = NextNode;
			CurrentNode = NextNode;
			AllocNum--;
		}
		CurrentNode->next = nullptr;
		return result;
	}

}


template<bool threads, int inst>
char* DefaultAlloc<threads, inst>::chunk(size_t size, size_t& AllocNum)
{
	char *result;
	size_t TotalByte = size*AllocNum;//需要申请的内存字节数
	size_t LeftByte = end - start;//内存池中剩余的内存字节数
	if (TotalByte <= LeftByte)//当内存池中的内存足够分配所需要的内存时
	{
		result = start;
		start += TotalByte;
		return result;
	}
	else if (size <= LeftByte)//当内存池中的内存足够分配一个及以上的内存时
	{
		AllocNum = LeftByte / size;
		TotalByte = AllocNum*size;
		result = start;
		start += TotalByte;
		return result;
	}
	else//当内存池中的内存不够一个时，即内存池中还有一些内存零头,先将其分配给适当的链表
	{
		if (LeftByte>0)//这里还剩下的内存一定是8的整数倍，申请8的整数倍,取走8的整数倍
		{

			NODE**MyAlloc;
			MyAlloc = FreeList + FreeList_index(LeftByte);
			((NODE*)start)->next = *MyAlloc;//先进行类型转换再访问成员
			*MyAlloc = (NODE*)start;
			start = end;
		}

		size_t GetBytes = 2 * size*AllocNum + round_up(heap >> 4);//每次需要从堆中重新申请内存时,申请需要的两倍加上附加值 
		//向堆内存申请新的内存，考虑到堆内存不足的情况
		//这一步也保证了无论怎么分配，最终在refill的时候都是整数倍的去填充内存链表
		start = (char *)malloc(GetBytes);
		//	cout<<"内存池内存不足，向堆申请内存"<<endl;
		if (start == NULL)
		{
			//堆内存不足,考虑遍历整个内存链表找到所有的剩余内存
			//为了效率,只遍历大于size的链表
			NODE** MyAlloc;
			NODE *p;
			for (int i = size; i <= MAX_BYTES; i += BASE)
			{
				MyAlloc = FreeList + FreeList_index(i);
				if (*MyAlloc != nullptr)//如果某个链表仍然还有空闲内存则取出,交给内存池,然后再次调用自己
				{//交给内存池 就是在start和end上进行相应的操作
					p = *MyAlloc;
					*MyAlloc = (*MyAlloc)->next;
					start = (char *)p;
					end = start + i;
					return chunk(size, AllocNum);
				}
			}
			//如果还是没有内存则启用一级内存分配器的内存不足处理机制
			end = 0;
			start = (char *)MallocAlloc<0>::allocate(GetBytes);
		}
		end = start + GetBytes;
		heap += GetBytes;//一个增加值
		return chunk(size, AllocNum);
	}
}

#endif
