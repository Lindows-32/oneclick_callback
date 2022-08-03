#include <iostream>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "remote_function.h"
#include "invoke_byte_stream.h"
#include "oneclick_call_local_function.h"
using namespace modern_framework;

class test_t
{
	std::string name;
	int age;
	
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& name;
		ar& age;
	}
public:
	test_t& operator=(test_t&& that)
	{
		name=std::move(that.name);
		age=that.age;
		return *this;
	}
	test_t& operator=(const test_t& that)
	{
		name = that.name;
		age = that.age;
		return *this;
	}
	test_t(test_t&& that)
	{
		std::cout << "test_t(test_t&&)" << std::endl;
		operator=(that);
	}
	test_t(const test_t& that)
	{
		std::cout << "test_t(const test_t&)" << std::endl;
		operator=(that);
	}
	test_t()
	{
		age = 0;
	}
	test_t(std::string name, int age)
	{
		this->name = name;
		this->age = age;
		std::cout << "test_t(std::string name,int age)" << std::endl;
	}
	void test_fun(std::list< std::wstring>& a, bool b, std::string* c)
	{
		std::cout << name << ':' << age << std::endl;
		std::cout << '{';
		auto it = a.begin();
		for (size_t i = 0; i < a.size(); i++)
		{
			printf("%ls",it->c_str());
			it++;
			if (i == a.size() - 1)
				 std::cout<< '}';
			else
				std::cout << ',';
		}
		std::cout << ' ';
		printf("%ls ",b?L"true":L"false");
		std::cout<< *c << std::endl;
	}
	void test_fun2(std::vector<test_t>&& a, const char* b, std::string* c)
	{
		for (auto& r : a)
		{
			std::cout << r.name << ':' << r.age << std::endl;
		}
		std::cout << name << ':' << age << std::endl;
		std::cout <<  b << ' ' << *c << std::endl;
	}
	void test3()
	{
		std::cout << "R U OK?" << std::endl;
		return ;
	}
};


namespace modern_framework
{
	template <>
	void serialize_arg(std::stringstream &buffer, test_t& arg)
	{
		std::stringstream ss;
		boost::archive::binary_oarchive ar(ss);
		ar<<arg;
		std::string temp=std::move(ss.str());
		uint32_t length = static_cast<uint32_t>(temp.size());
		buffer.write(reinterpret_cast<const char*>(&length), sizeof(length));
		buffer.write(temp.c_str(),length);
	}
	template<>
	void deserialize_arg(char*& buffer, test_t& object)
	{
		uint32_t length = *reinterpret_cast<uint32_t*>(buffer);
		buffer += sizeof(uint32_t);
		std::stringstream ss;
		ss.write(buffer, length);
		boost::archive::binary_iarchive ar(ss);
		ar >> object;
		buffer += length;
	}
}

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

int main()
{
	setlocale(LC_ALL,"zh_CN.UTF-8");
	
	sender_t sender;
	remote_slot slots;
	test_t t("雷布斯", 51);
	//把成员函数注册一下
	slots.bind(5,WRAP4REMOTE_SLOT(&test_t::test_fun),&t);
	slots.bind(6,WRAP4REMOTE_SLOT(&test_t::test_fun2),&t);
	slots.bind(7,WRAP4REMOTE_SLOT(&test_t::test3),&t);
	
	//配置发送端的对象
	remote_function<void(std::vector<std::wstring>&&,bool&&,std::string*)> fun;
	fun._function_id=5;
	fun.sender=&sender;
	//准备参数
	std::string p = "乃好吗?";
	std::vector<std::wstring> pvalue({ L"字符串1",L"字符串2",L"字符串3"});
	fun(std::move(pvalue), false, &p);//假设一端调用远程函数
	slots.parse_and_call(sender.buffer.data());//另一端接收到字节流后直接运行

	//配置发送端的对象
	remote_function <void(std::vector<test_t>&&, const char*, std::string*) > fun2;
	fun2._function_id = 6;
	fun2.sender = &sender;
	//准备参数
	fun2({ test_t("卢十瓦", 45),test_t("卢十瓦2", 456) }, "太棒了", &p);
	slots.parse_and_call(sender.buffer.data());

	//无参版
	remote_function <void()> fun3;
	fun3._function_id = 7;
	fun3.sender = &sender;
	fun3();
	slots.parse_and_call(sender.buffer.data());
}