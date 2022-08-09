#ifndef MODERN_REMOTE_SIGNAL_H
#define MODERN_REMOTE_SIGNAL_H
#include "modern_modify_rule.h"
#include "modern_parameters_pack_modify.h"
#include "modern_class_analysis.h"
#include <string.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>

namespace modern_framework
{
	template <class T>
	void serialize_arg(std::stringstream& buffer, T& arg)
	{
		uint32_t len=static_cast<uint32_t>(sizeof(T));
		buffer.write(reinterpret_cast<const char*>(&len),sizeof(uint32_t));
		buffer.write(reinterpret_cast<const char*>(&arg),len);
	}

	template<class T>
	class type_select;
	template<class T>
	class type_select<T&>
	{
		class for_basic_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const T& arg)
			{
				buffer.write(reinterpret_cast<const char*>(&arg), sizeof(T));
			}
		};
		class for_class_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const T& arg)
			{
				serialize_arg(buffer,const_cast<T&>(arg));
			}
		};
	public:
		using type= typename IF<type<T>::is_basic_type::result, for_basic_type, for_class_type>::THEN;
	};

	template<class T>
	class type_select<const T&>
	{
	public:
		using type= typename type_select<T&>::type;
	};

	template<class T>
	class type_select<T*>
	{
		class for_basic_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const T* arg)
			{
				buffer.write(reinterpret_cast<const char*>(arg), sizeof(T));
			}
		};
		class for_class_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const T* arg)
			{
				serialize_arg(buffer, const_cast<T&>(*arg));
			}
		};
	public:
		using type= typename IF<type<typename std::remove_const<T>::type>::is_basic_type::result, for_basic_type, for_class_type>::THEN;
	};

	template<class T>
	class type_select<const T*>
	{
	public:
		using type= typename type_select<T*>::type;
	};

	using const_string = const char*;

	template<>
	class type_select<const_string>
	{
		class for_basic_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const char* arg)
			{
				uint32_t len=static_cast<uint32_t>(strlen(arg))+1;
				buffer.write(reinterpret_cast<const char*>(&len),sizeof(uint32_t));
				buffer.write(arg,len);
			}
		};
	public:
		using type= for_basic_type;
	};
	template<>
	class type_select<const_string&>
	{
	public:
		using type = type_select<const_string>::type;
	};

	using const_wstring = const wchar_t*;

	template<>
	class type_select<const_wstring>
	{
		class for_basic_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer, const wchar_t* arg)
			{
				uint32_t len = static_cast<uint32_t>(wcslen(arg)) + 1;
				uint32_t len2 = len*2;
				buffer.write(reinterpret_cast<const char*>(&len2), sizeof(uint32_t));
				std::vector<uint16_t> temp(len+1);
				for (uint32_t i = 0; i < len; i++)
				{
					temp[i] = arg[i];
				}
				temp[len] = L'\0';
				buffer.write(reinterpret_cast<const char*>(temp.data()), len2);
			}
		};
	public:
		using type = for_basic_type;
	};
	template<>
	class type_select<const_wstring&>
	{
	public:
		using type = type_select<const_wstring>::type;
	};

	template<class T>
	class type_select<std::vector<T>&>
	{
		class for_class_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const std::vector<T>& arg)
			{
				uint32_t len=static_cast<uint32_t>(arg.size());
				buffer.write(reinterpret_cast<const char*>(&len),sizeof(uint32_t));
				for(auto& r:arg)
					type_select<T&>::type::_serialize_arg(buffer, r);
			}
		};
	public:
		using type= for_class_type;
	};

	template<class T>
	class type_select<std::list<T>&>
	{
		class for_class_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const std::list<T>& arg)
			{
				uint32_t len=static_cast<uint32_t>(arg.size());
				buffer.write(reinterpret_cast<const char*>(&len),sizeof(uint32_t));
				for(auto& r:arg)
					type_select<T&>::type::_serialize_arg(buffer, r);
			}
		};
	public:
		using type= for_class_type;
	};

	template<class _key,class _value>
	class type_select<std::map<_key,_value>&>
	{
		class for_class_type
		{
		public:
			static void _serialize_arg(std::stringstream& buffer,const std::map<_key,_value>& arg)
			{
				uint32_t len=static_cast<uint32_t>(arg.size());
				buffer.write(reinterpret_cast<const char*>(&len),sizeof(uint32_t));
				for(auto& r:arg)
				{
					type_select<_key&>::type::_serialize_arg(buffer, r.first);
					type_select<_value&>::type::_serialize_arg(buffer, r.second);
				}
			}
		};
	public:
		using type= for_class_type;
	};

	template <>
	void serialize_arg(std::stringstream& buffer, std::string& arg)
	{
		uint32_t len=static_cast<uint32_t>(arg.length())+1;
		buffer.write(reinterpret_cast<const char*>(&len),sizeof(uint32_t));
		buffer.write(arg.c_str(),len);
	}
	
	template <>
	void serialize_arg(std::stringstream& buffer, std::wstring& arg)
	{
		uint32_t len = static_cast<uint32_t>(arg.length()) + 1;
		uint32_t len2 = len * 2;
		std::vector<uint16_t> temp(len);
		for(uint32_t i=0;i<len-1;i++)
		{
			temp[i]=arg[i]&0xFFFF;
		}
		buffer.write(reinterpret_cast<const char*>(&len2), sizeof(uint32_t));
		buffer.write(reinterpret_cast<const char*>(temp.data()), len2);
	}

	template <class... fargs>
	class serialize;

	template <class T, class... fargs>
	class serialize<T, fargs...>
	{
	public:
		static void exec(std::stringstream& buffer, T arg0, fargs... args)
		{
			type_select<T>::type::_serialize_arg(buffer, arg0);
			serialize<fargs...>::exec(buffer, args...);
		}
	};
	template <class T>
	class serialize<T>
	{
	public:
		static void exec(std::stringstream& buffer, T arg0)
		{
			type_select<T>::type::_serialize_arg(buffer, arg0);
		}
	};

	class interface_remote_function
	{
	public:
		virtual void send(const char* buffer,size_t length)=0;
		virtual ~interface_remote_function(){}
	};

	class remote_function_base:public interface_remote_function
	{
	protected:
		interface_remote_function* _sender;
		int32_t _function_id;
	public:
		
		void config(interface_remote_function* sender,int32_t function_id)
		{
			_sender=sender;
			_function_id=function_id;
		}
		virtual void send(const char* buffer,size_t length)override
		{
			_sender->send(buffer,length);
		}
	};

	template <class T>
	class remote_signal;

	template <class R, class... args >
	class remote_signal<R(args...)> :public remote_function_base
	{
	public:
		R operator()(args... arg)
		{
			std::stringstream ss;
			parameters_pack_modify<preset_nodify_rule::value_to_ref, serialize, std::stringstream&>::template modify<int32_t, args...>::
				exec(ss, _function_id, arg...);
			std::string buffer_string = std::move(ss.str());
			send(buffer_string.c_str(), buffer_string.length());
			return typename std::remove_reference<R>::type();

		}
	};

	template <class... args>
	class remote_signal<void(args...)>:public remote_function_base
	{
	public:
		void operator()(args... arg)
		{
			std::stringstream ss;
			parameters_pack_modify<preset_nodify_rule::value_to_ref, serialize, std::stringstream&>::template modify<int32_t, args...>::
			exec(ss, _function_id,arg...);
			std::string buffer_string=std::move(ss.str());
			send(buffer_string.c_str(),buffer_string.length());
		}
	};

}

#endif