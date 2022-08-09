#include <iostream>
#include "modern_remote_signal.h"
#include "modern_remote_slots.h"

using namespace modern_framework;
using std::map;
using std::vector;
using std::string;
using std::wstring;
using std::cout;
using std::wcout;
using std::endl;
#define SIGNALS

class sender_t:public modern_framework::interface_remote_function
{
public:
	std::vector<char> buffer;
	virtual void send(const char* buf,size_t len)
	{
		buffer=std::move(std::vector<char>(buf,buf+len));
	}
	void show()
	{
		size_t length=buffer.size();
		for(size_t i=0;i<length;i++)
		{
			if(buffer[i]>32&&buffer[i]<=127)
				printf("%c ",buffer[i]);
			else
				printf("%02X ",(unsigned char)buffer[i]);
		}
		std::cout<<std::endl;
	}
};

class test_t
{
    string _name;
    int _age;
public:
    void test_fun(vector<wstring>&& a,const wchar_t* b,int c)
    {
        cout<<_name<<':'<<_age<<"\na=[";
        for(size_t i=0;i<a.size();i++)
        {
            printf("%ls%c",a[i].c_str(),(i==a.size()-1?']':','));
        }

        printf(",b=%ls,c=%d\n",b,c);
    }
    int test_noarg()
    {
        printf("%ls\n",L"无参");
        return 0;
    }
    test_t()
    {
        _age=0;
    }
    test_t(string name,int age)
    {
        _age=age;
        _name=name;
    }
};


int main(int argc,const char* argv[])
{
    if(argc==2)
        setlocale(LC_ALL,argv[1]);
    else
        std::locale::global(std::locale(""));

	sender_t sender;
	test_t t("雷布斯", 51);
	int value=114514;
	remote_signal<double(vector<wstring>&&,const wchar_t*,int)> fun;
	fun.config(&sender,5);
	std::vector<std::wstring> pvalue({ L"字符串1",L"字符串2",L"字符串3"});

	fun(std::move(pvalue), L"R U OK?", value);
	sender.show();
	char* buf_ptr = sender.buffer.data()+sizeof(uint32_t);
	modern_framework::invoke(&test_t::test_fun, &t, buf_ptr);

    remote_signal<int()> fun2;
	fun2.config(&sender,6);
	fun2();
	modern_framework::invoke(&test_t::test_noarg, &t, buf_ptr);
    return 0;
}