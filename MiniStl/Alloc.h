//��Ȼ���������ڴ�,��һ��Ҫ���ǵ��ڴ治������
#ifndef ALLOC_H
#define ALLOC_H
#include<cstddef>
#include<cstdlib>
#include<iostream>
using namespace std;
const size_t BASE = 8;//��׼��
const size_t MAX_BYTES = 128;//�����
const size_t FREELIST_NUM = MAX_BYTES / BASE;//������������
//һ��������
/*
һ���������ڽ���ڴ治������ʱ,������ģ��new�����Ĵ���ʽ
����Ĳ����������ɿ������ֶ��趨����Ĵ���ʽmalloc_hanlder,
Ȼ��malloc��������NULL�����malloc_again������malloc_hanlder
�����������
*/
template <int inst>
class MallocAlloc
{
private:
	//���ڴ治��ʱͨ�����������ϳ������ú��ͷŵ�����
	//���������³��Ժ������ڴ�,�ɱ�������malloc_hanlder��ͬ���ɴ�����
	static void *malloc_again(size_t size);

	static void *realloc_again(void *ptr, size_t size);

	//����һ���ڴ治��Ĵ�����,���ڴ治��ʱͨ���˺���ָ����
	//ָ���������������
	static void(*malloc_hanlder)();
public:
	static void* allocate(size_t size)
	{//����������ڴ��С����128ʱ������һ�������������ڴ�ķ������ͷ�
		//ֱ�Ӳ���malloc����operator new�ķ�ʽ�����ڴ����
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
		free(ptr);//ע�������ptr����������allocate��������ڴ�(��nΪ��С)
	}

	static auto set_hander(void(*f)())->void(*)()
	{//�����������û�ָ������������������������һ�εĳ�������
		void(*OldHander)() = malloc_hanlder;
		malloc_hanlder = f;
		return OldHander;
	}

};

template<int inst>
void(*MallocAlloc<inst>::malloc_hanlder)() = nullptr;

//�ٴη����ڴ�
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
		(*my_hander)();//���ô�����,�䳢��ȥ�ͷ��ڴ�,��һ��ִ�к��ڴ滹�ǲ���,���ٴ�ִ��
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
			;//δ������ڴ治�㴦����
		}
		(*my_hander)();
		if ((result = realloc(OldPtr, size)) != NULL)
		{
			return result;
		}
	}
}


//����������
template<bool threads, int inst>
class DefaultAlloc
{
private:
	//��Ϊ�����������Ľڵ�,Ϊ�˱����ڴ���˷�,������ù�����
	union NODE
	{
		union NODE *next;
		char data[1];
	};
private:

	//�������������
	static NODE *FreeList[FREELIST_NUM];
	//ָ���ڴ�ص����
	static char *start;//��ָ������Ϊchar������Ϊ�˱�֤�ڽ����ڴ�ļӼ�����ʱÿ��Ϊһ���ֽ�
	//ָ���ڴ�ص��յ�
	static char *end;

	static size_t heap;
	//��������ڴ治��8��������ʱ,����8��������
	static size_t round_up(size_t size)
	{
		return (size + BASE - 1)& ~(BASE - 1);//�ȶ�7ȡ��,Ȼ�����������
	}
	//���������ֽڶ�Ӧ���������
	static size_t FreeList_index(size_t size)
	{
		return round_up(size) / BASE - 1;
	}

public:

	static void *allocate(size_t size);

	static void *reallocate(size_t size);
	//����p��ָ���size��С���ڴ�
	static void deallocate(void *ptr, size_t size);
	//�����������޿��пռ�ʱ����������������,����װ�����Ȼ���ص���,������û����ڴ�
	//ʵ����������chunk���������,ֻ��chunk�����Ż�����ڴ��
	static void* refill(size_t size);
	//���ڴ�ػ�ȡ�ڴ�,���ڴ���ڴ治��ʱ����һ���ռ�������
	//���ش��ڴ����ʵ�ʻ�ȡ�����ڴ�,������װ�����Ӧ�Ŀ�������
	static char *chunk(size_t size, size_t& AllocNum);


};

template<bool threads, int inst>
char* DefaultAlloc<threads, inst>::start = nullptr;

template<bool threads, int inst>
char* DefaultAlloc<threads, inst>::end = nullptr;

template<bool threads, int inst>
size_t DefaultAlloc<threads, inst>::heap = 0;

template<bool threads, int inst>
typename DefaultAlloc<threads, inst>::NODE* //�Ծ�̬����������� ,����ģ�����е�����ʱ��Ϊ��������;�̬��Ա������
//��Ҫ������ǰ����Ϲؼ���typename
DefaultAlloc<threads, inst>::FreeList[FREELIST_NUM] = {
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr };

template<bool threads, int inst>
void* DefaultAlloc<threads, inst>::allocate(size_t size)
{
	if (size>MAX_BYTES)
	{
		return MallocAlloc<0>::allocate(size);//����ģ����ǵü������
	}
	NODE** MyAlloc;
	NODE* result;
	MyAlloc = FreeList_index(size) + FreeList;//MyAllocָ����Ӧ�Ŀ�������
	size = round_up(size);//���������ڴ����䵽8����С������
	if (*MyAlloc != nullptr)//����Ӧ��������Ϊ����ȡ��������
	{
		result = *MyAlloc;
		*MyAlloc = (*MyAlloc)->next;
		return result;
	}
	else
	{
		//	cout<<"testing"<<endl;
		void *r = refill(round_up(size));//����װ��
		return r;
	}
}
//���շ�����ڴ��
template<bool threads, int inst>
void DefaultAlloc<threads, inst>::deallocate(void *ptr, size_t size)
{

	NODE** MyAlloc;
	NODE* FreeNode = (NODE*)ptr;
	MyAlloc = FreeList + FreeList_index(size);
	if (size>MAX_BYTES)//���Ҫ���ͷŵ��ֽ�������128����õ�һ��������
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

//����������ռ䲻��,���´��ڴ������������������Ӧ���ڴ������û�
template<bool threads, int inst>
void* DefaultAlloc<threads, inst>::refill(size_t size)
{//�����size�Ѿ���8��������
	size_t AllocNum = 20;//�����õ���ʽ�������,��Ϊ��ֵ���ܻ����ڴ���ڴ��С���䶯
	NODE* result;
	NODE** MyAlloc;
	NODE* CurrentNode;
	NODE* NextNode;
	char* AllocResult = chunk(size, AllocNum);
	result = (NODE*)AllocResult;
	if (AllocNum == 1)//�����ֻ���䵽��һ���ڴ����ֱ�ӷ�����û�,�������ټ���������
	{
		return result;
	}
	else
	{
		AllocResult += size;//�������ڴ�����һ��ʱ,��Ҫ�����ڵ��ڴ����뵽����������
		MyAlloc = FreeList + FreeList_index(size);
		//	CurrentNode=*MyAlloc;//*MyAllocʵ����ָ���Ӧ���ָ��,
		*MyAlloc = CurrentNode = (NODE*)AllocResult;//����ָ�������
		//ע������Ĳ�������
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
	size_t TotalByte = size*AllocNum;//��Ҫ������ڴ��ֽ���
	size_t LeftByte = end - start;//�ڴ����ʣ����ڴ��ֽ���
	if (TotalByte <= LeftByte)//���ڴ���е��ڴ��㹻��������Ҫ���ڴ�ʱ
	{
		result = start;
		start += TotalByte;
		return result;
	}
	else if (size <= LeftByte)//���ڴ���е��ڴ��㹻����һ�������ϵ��ڴ�ʱ
	{
		AllocNum = LeftByte / size;
		TotalByte = AllocNum*size;
		result = start;
		start += TotalByte;
		return result;
	}
	else//���ڴ���е��ڴ治��һ��ʱ�����ڴ���л���һЩ�ڴ���ͷ,�Ƚ��������ʵ�������
	{
		if (LeftByte>0)//���ﻹʣ�µ��ڴ�һ����8��������������8��������,ȡ��8��������
		{

			NODE**MyAlloc;
			MyAlloc = FreeList + FreeList_index(LeftByte);
			((NODE*)start)->next = *MyAlloc;//�Ƚ�������ת���ٷ��ʳ�Ա
			*MyAlloc = (NODE*)start;
			start = end;
		}

		size_t GetBytes = 2 * size*AllocNum + round_up(heap >> 4);//ÿ����Ҫ�Ӷ������������ڴ�ʱ,������Ҫ���������ϸ���ֵ 
		//����ڴ������µ��ڴ棬���ǵ����ڴ治������
		//��һ��Ҳ��֤��������ô���䣬������refill��ʱ������������ȥ����ڴ�����
		start = (char *)malloc(GetBytes);
		//	cout<<"�ڴ���ڴ治�㣬��������ڴ�"<<endl;
		if (start == NULL)
		{
			//���ڴ治��,���Ǳ��������ڴ������ҵ����е�ʣ���ڴ�
			//Ϊ��Ч��,ֻ��������size������
			NODE** MyAlloc;
			NODE *p;
			for (int i = size; i <= MAX_BYTES; i += BASE)
			{
				MyAlloc = FreeList + FreeList_index(i);
				if (*MyAlloc != nullptr)//���ĳ��������Ȼ���п����ڴ���ȡ��,�����ڴ��,Ȼ���ٴε����Լ�
				{//�����ڴ�� ������start��end�Ͻ�����Ӧ�Ĳ���
					p = *MyAlloc;
					*MyAlloc = (*MyAlloc)->next;
					start = (char *)p;
					end = start + i;
					return chunk(size, AllocNum);
				}
			}
			//�������û���ڴ�������һ���ڴ���������ڴ治�㴦�����
			end = 0;
			start = (char *)MallocAlloc<0>::allocate(GetBytes);
		}
		end = start + GetBytes;
		heap += GetBytes;//һ������ֵ
		return chunk(size, AllocNum);
	}
}

#endif
