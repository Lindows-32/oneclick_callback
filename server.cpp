#include <iostream>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "remote_function.h"
#include "invoke_byte_stream.h"
#include "oneclick_call_local_function.h"
#include "simple_t.h"
using namespace modern_framework;

struct simple
{
	double price;
	char name[8];
};

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
	void test_fun(std::list< std::wstring>& a, simple b, std::string* c)
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
		printf("{%s:%lf} ",b.name,b.price);
		std::cout<< *c << std::endl;
	}
	void test_fun2(std::vector<test_t>&& a, const char* b, std::string* c)
	{
		std::cout<<'{';
		for (size_t i=0;i<a.size();i++)
		{
			std::cout << a[i].name << ':' << a[i].age <<(i==a.size()-1?"} ":",");
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

class sender_t:public interface_remote_function
{
public:
	std::vector<char> buffer;
	virtual void send(const char* buf,size_t len) override
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
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::enable_shared_from_this;
using boost::system::error_code;
using boost::asio::ip::tcp;
using boost::asio::mutable_buffer;
#define _1 std::placeholders::_1
#define _2 std::placeholders::_2
#define SIGNALS

class session:public interface_remote_function
{
	remote_slot _slots;
	tcp::socket _sock;
	int32_t _reference_count;
	char buffer[1024];
	char send_buffer[1024];
	virtual void send(const char* buffer,size_t length) override
	{
		*reinterpret_cast<uint32_t*>(send_buffer)=length;
		memcpy(send_buffer+sizeof(uint32_t),buffer,length);
		_sock.async_send(mutable_buffer(const_cast<char*>(send_buffer),length+sizeof(uint32_t)),std::bind([this](const error_code& e,size_t byte_sent)
		{
			if(e)
				return;
		},_1,_2));
	}
	class cat
	{
		int _count;
		string _name;
	public:
		void eat(int32_t count,vector<string>& names)
		{
			_count+=count;
			cout<<_name<<"吃了"<<count<<"只老鼠,分别是[";
			for(size_t i=0;i<names.size();i++)
				cout<<names[i]<<(i==names.size()-1?"]":",");
			cout<<",总计共吃了"<<_count<<"只老鼠"<<endl;
		}
		void set_name(string&& name)
		{
			_name=std::move(name);
		}
	} _cat;
	void say_hello()
	{
		string name="服务器";
		send_name(&name);
	}
	void on_recv(const error_code& e,size_t byte_recved,bool getting_len)
	{
		if (e)
		{
			delete this;
			return;
		}
		if (getting_len)
		{
			uint32_t length=*reinterpret_cast<uint32_t*>(buffer);
			_sock.async_read_some(mutable_buffer(buffer,length),std::bind(&session::on_recv,this,_1,_2,false));
		}
		else
		{
			_slots.parse_and_call(buffer);
			_sock.async_read_some(mutable_buffer(buffer,sizeof(uint32_t)),std::bind(&session::on_recv,this,_1,_2,true));
		}
	}

SIGNALS
	remote_function<void(std::string*)> send_name;
public:
	session(tcp::socket&& sock):_sock(std::move(sock))
	{
		_slots.bind(0,RS_WRAP(&session::say_hello),this);
		_slots.bind(1,RS_WRAP(&cat::set_name),&_cat);
		_slots.bind(2,RS_WRAP(&cat::eat),&_cat);
		send_name.config(this,1);
		_sock.async_read_some(mutable_buffer(buffer,sizeof(uint32_t)),std::bind(&session::on_recv,this,_1,_2,true));
	}
};


class server_demo
{
	boost::asio::io_context _ioc;
	tcp::acceptor _server_sock;
	void accept()
	{
		std::shared_ptr<tcp::socket> sock(new tcp::socket(_ioc));
		_server_sock.async_accept(*sock,std::bind(&server_demo::on_accept,this,_1,sock));
	}
	void on_accept(const error_code& e,std::shared_ptr<tcp::socket> sock)
	{
		if(e)
			return;
		new session(std::move(*sock));
		accept();
	}
public:
	server_demo(uint16_t port):_server_sock(_ioc,tcp::endpoint(tcp::v4(),port))
	{
		accept();
	}
	void run()
	{
		_ioc.run();
	}
};

int main(int argc,char* argv[])
{
	if(argc>=2)
		setlocale(LC_ALL,argv[1]);
	else
		setlocale(LC_ALL,"zh_CN.UTF-8");


	server_demo server(8998);
	server.run();
}