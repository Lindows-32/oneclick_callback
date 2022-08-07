#include <iostream>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio/steady_timer.hpp>
#include "remote_function.h"
#include "invoke_byte_stream.h"
#include "oneclick_call_local_function.h"
#include "simple_t.h"
using namespace modern_framework;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::enable_shared_from_this;
using boost::system::error_code;
using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::mutable_buffer;
using boost::asio::steady_timer;
#define _1 std::placeholders::_1
#define _2 std::placeholders::_2
#define SIGNALS
#define SLOTS
class session:public interface_remote_function
{
	remote_slot _slots;
	tcp::socket _sock;
    steady_timer _timer;
	char buffer[1024];
	char send_buffer[1024];
	virtual void send(const char* buffer,size_t length) override
	{
		*reinterpret_cast<uint32_t*>(send_buffer)=length;
		memcpy(send_buffer+sizeof(uint32_t),buffer,length);
		_sock.async_send(mutable_buffer(const_cast<char*>(send_buffer),length+sizeof(uint32_t)),std::bind([this](const error_code& e,size_t byte_sent){},_1,_2));
	}
	
	void on_recv(const error_code& e,size_t byte_recved,bool getting_len)
	{
		if(e)
			return;
		if(getting_len)
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
    void on_connected(const error_code& e )
    {
        if(e)
            return;
        _sock.async_read_some(mutable_buffer(buffer,sizeof(uint32_t)),std::bind(&session::on_recv,this,_1,_2,true));
        _timer.expires_after(std::chrono::seconds(1));
        _timer.async_wait(std::bind(&session::on_timer,this,_1));
    }
    void on_timer(const error_code& e)
    {
        if(e)
            return;
        int x=rand()%3;
        switch(x)
        {
        case 0:
            say_hello();
            break;
        case 1:
            {
                vector<string> a={"Jerry","Mickey","Shuke"};
                give_mouse(3,a);
                break;
            }
        default:
            tell_name2cat(string(rand()%2?"Tom":"Black"));
        }
        _timer.expires_after(std::chrono::seconds(1));
        _timer.async_wait(std::bind(&session::on_timer,this,_1));
        
    }
SLOTS void server_say_hello(string* name)
	{
		cout<<*name<<endl;
	}
SIGNALS
	remote_function<void()> say_hello;
    remote_function<void(int32_t,vector<string>&)>  give_mouse;
    remote_function<void(string&&)> tell_name2cat;
public:
	session(boost::asio::io_context& ioc,tcp::socket&& sock):_sock(std::move(sock)),_timer(ioc)
	{
		_slots.bind(1,RS_WRAP(&session::server_say_hello),this);
		say_hello.config(this,0);
        tell_name2cat.config(this,1);
        give_mouse.config(this,2);        
        _sock.async_connect(tcp::endpoint(address::from_string("127.0.0.1"),8998),std::bind(&session::on_connected,this,_1));
	}
};

int main()
{
    boost::asio::io_context ioc;
    tcp::socket sock(ioc);
    session ss(ioc,std::move(sock));
    ioc.run();
}
