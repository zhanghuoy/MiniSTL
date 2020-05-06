#include"MiniVector.h"
#include<cstdlib>
#include<string>
#include<vector>
using namespace std;
int main()
{
	MiniStl::MiniVector<string> vec(10,"han");
	MiniStl::MiniVector<string> vec1(6, "xin");
	//vector<string>vec1(10,"xin");
	string str1 = "han";
	string str2 = "xin";
	string str3 = "zhou";

	auto iter = vec.begin();
	iter++;
	vec.insert(iter,vec1.begin(),vec1.end());
	vec.resize(23,"zhou");
	cout << vec.size() << "  " << vec.capacity() << endl;
	for (auto p = vec.begin(); p != vec.end(); p++)
	{
		cout << *p << " ";
	}
	/*vector<string>vec(10, "han");
	vec.push_back("xin");
	vec.reserve(23);
	//vec.resize(23, "zhou");
	cout << vec.size() << "  " << vec.capacity() << endl;
	for (auto p = vec.begin(); p != vec.end(); p++)
	{
		cout << *p << " ";
	}*/
	system("pause");
	return 0;
}