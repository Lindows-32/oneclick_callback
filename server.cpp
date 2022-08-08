#include <iostream>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "remote_function.h"
#include "invoke_byte_stream.h"
#include "oneclick_call_local_function.h"
#include "cs_common_type.h"
using namespace modern_framework;

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
		int eat(int32_t count,vector<complex>& mice)
		{
			_count+=count;
			cout<<_name<<"吃了"<<count<<"只老鼠,分别是[";
			for(size_t i=0;i<mice.size();i++)
				cout<<'{'<<mice[i].name <<':'<<mice[i].age<<'}'<<(i==mice.size()-1?"]":",");
			cout<<",总计共吃了"<<_count<<"只老鼠"<<endl;
			return 0;
		}
		void set_name(string&& name)
		{
			_name=std::move(name);
		}
	} _cat;
	void say_hello(simple s)
	{
		cout<<"say_hello:{"<<s.name<<':'<<s.price<<'}'<<endl;;
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