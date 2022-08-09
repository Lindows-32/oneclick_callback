#ifndef CS_COMMON_TYPE_H
#define CS_COMMON_TYPE_H
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
//不包含指针的简单类(可以直接二进制发送)
struct simple
{
	double price;
	char name[8];
};


//包含指针的复杂类型,需要自己提供序列化和反序列化的特化
class complex
{
public:
	std::string name;
	int age;
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& name;
		ar& age;
	}

	complex(complex&& that)
	{
		std::cout << "complex(complex&&)" << std::endl;
		name=std::move(that.name);
		age=that.age;
	}
	complex(const complex& that)
	{
		std::cout << "complex(const complex&)" << std::endl;
		name=that.name;
		age=that.age;
	}
	complex()
	{
		std::cout << "complex()" << std::endl;
		age = 0;
	}
	complex(std::string name, int age)
	{
		this->name = name;
		this->age = age;
		std::cout << "complex(std::string name,int age)" << std::endl;
	}
};


namespace modern_framework//加入特化
{
	template <>
	void serialize_arg(std::stringstream &buffer, complex& arg)//序列化
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
	void deserialize_arg(char*& buffer, complex& object)//反序列化
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
#endif