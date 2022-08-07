#ifndef INVOKE_BYTE_STREAM_H
#define INVOKE_BYTE_STREAM_H
#include <vector>
#include <string>
#include <list>
#include "invoke_helper.h"
#include "modify_rule.h"
#include "modern_class_analysis.h"
namespace modern_framework
{
	template<class T>
	void deserialize_arg(char*& buffer,T& object)
	{
		std::cout<<typeid(T).name()<<std::endl;
		uint32_t length = *reinterpret_cast<uint32_t*>(buffer);
		buffer+=sizeof(uint32_t);
		object=*reinterpret_cast<T*>(buffer);
		buffer+=length;
	}

	template<size_t index, class param_T, class T>
	class extractor_type_select;

	template<size_t index, class T>
	class extractor_type_select<index, char*, T&>
	{
		class for_basic_type
		{
			T value;
		public:
			inline for_basic_type(char*& data_source)
			{
				value = *reinterpret_cast<T*>(data_source);
				data_source += sizeof(T);
			}
			inline operator T& ()
			{
				return value;
			}
		};
		class for_class_type
		{
			T value;
		public:
			inline for_class_type(char*& data_source)
			{
				deserialize_arg(data_source,value);
			}
			inline operator T& ()
			{
				return value;
			}
		};
	public:
		using type= typename IF<type<T>::is_basic_type::result, for_basic_type, for_class_type>::THEN;
	};

	template<size_t index, class T>
	class extractor_type_select<index, char*,const T&>
	{
	public:
		using type= typename extractor_type_select<index, char*, T&>::type;
	};

	template<size_t index, class T>
	class extractor_type_select<index, char*, T*>
	{
		class for_basic_type
		{
			T value;
		public:
			inline for_basic_type(char*& data_source)
			{
				value = *reinterpret_cast<T*>(data_source);
				data_source += sizeof(T);
			}
			inline operator T* ()
			{
				return &value;
			}
		};
		class for_class_type
		{
			T value;
		public:
			inline for_class_type(char*& data_source)
			{
				deserialize_arg(data_source,value);
			}
			inline operator T* ()
			{
				return &value;
			}
		};
	public:
		using type= typename IF<type<T>::is_basic_type::result, for_basic_type, for_class_type>::THEN;
	};

	template<size_t index, class T>
	void vector_construct_common(std::vector<T>& value, char*& data_source)
	{
		uint32_t length = *reinterpret_cast<uint32_t*>(data_source);
		value.resize(length);
		data_source += sizeof(length);
		for (uint32_t i = 0; i < length; i++)
		{
			typename extractor_type_select<index, char*, T&>::type data(data_source);
			value[i] = std::move(data);
		}
	}
	

	template<size_t index, class T>
	class extractor_type_select<index, char*, std::vector<T>&>
	{
		class for_class_type
		{
			std::vector<T> value;
		public:
			inline for_class_type(char*& data_source)
			{
				vector_construct_common<index,T>(value, data_source);
			}
			inline operator std::vector<T>& ()
			{
				return value;
			}
		};
	public:
		using type = for_class_type;
	};

	template<size_t index, class T>
	class extractor_type_select<index, char*, std::vector<T>*>
	{
		class for_class_type
		{
			std::vector<T> value;
		public:
			inline for_class_type(char*& data_source)
			{
				vector_construct_common<index, T>(value, data_source);
			}
			inline operator std::vector<T>* ()
			{
				return &value;
			}
		};
	public:
		using type = for_class_type;
	};

	template<size_t index, class T>
	class extractor_type_select<index, char*,const std::vector<T>*>
	{
	public:
		using type = typename extractor_type_select<index, char*, std::vector<T>*>::type;
	};

	template<size_t index, class T>
	void list_construct_common(std::list<T>& value, char*& data_source)
	{
		uint32_t length = *reinterpret_cast<uint32_t*>(data_source);
		value.resize(length);
		data_source += sizeof(length);
		typename std::list<T>::iterator it = value.begin();
		for (uint32_t i = 0; i < length; i++)
		{
			typename extractor_type_select<index, char*, T&>::type data(data_source);
			(*it) = std::move(data);
			it++;
		}
	}

	template<size_t index, class T>
	class extractor_type_select<index, char*, std::list<T>&>
	{
		class for_class_type
		{
			std::list<T> value;
		public:
			inline for_class_type(char*& data_source)
			{
				list_construct_common<index,T>(value, data_source);
			}
			inline operator std::list<T>& ()
			{
				return value;
			}
		};
	public:
		using type = for_class_type;
	};

	template<size_t index, class T>
	class extractor_type_select<index, char*, std::list<T>*>
	{
		class for_class_type
		{
			std::list<T> value;
		public:
			inline for_class_type(char*& data_source)
			{
				list_construct_common<index, T>(value, data_source);
			}
			inline operator std::list<T>* ()
			{
				return &value;
			}
		};
	public:
		using type = for_class_type;
	};

	template<size_t index, class T>
	class extractor_type_select<index, char*,const std::list<T>*>
	{
	public:
		using type = typename extractor_type_select<index, char*, std::list<T>*>::type;
	};

	template<size_t index, class T>
	class extractor_type_select<index, char*,const std::list<T>&>
	{
	public:
		using type = typename extractor_type_select<index, char*, std::list<T>&>::type;
	};
	template<size_t index, class T>
	class extractor_type_select<index, char*,const T*>
	{
	public:
		using type= typename extractor_type_select<index, char*, T*>::type;
	};

	template<size_t index>
	class extractor_type_select<index, char*, const char*>
	{
		class for_basic_type
		{
			const char* value;
		public:
			inline for_basic_type(char*& data_source)
			{
				uint32_t length=*reinterpret_cast<uint32_t*>(data_source);
				data_source+=sizeof(uint32_t);
				value = data_source;
				data_source += length;
			}
			inline operator const char* ()
			{
				return value;
			}
		};
	public:
		using type= for_basic_type;
	};

	template<size_t index>
	class extractor_type_select<index, char*, const wchar_t*>
	{
		class for_basic_type
		{
			std::wstring value;
		public:
			inline for_basic_type(char*& data_source)
			{
				uint32_t length = *reinterpret_cast<uint32_t*>(data_source);
				uint32_t length2 = length / 2;
				data_source += sizeof(uint32_t);
				value.resize(length2);
				for (uint32_t i = 0; i < length2; i++)
				{
					value[i] = reinterpret_cast<uint16_t*>(data_source)[i];
				}
				data_source += length;
			}
			inline operator const wchar_t* ()
			{
				return value.c_str();
			}
		};
	public:
		using type = for_basic_type;
	};

	template<>
	void deserialize_arg(char*& buffer,std::string& object)
	{
		uint32_t length=*reinterpret_cast<uint32_t*>(buffer);
		buffer+=sizeof(uint32_t);
		object = std::move(std::string(buffer,buffer + length));
		buffer += length;
	}

	template<>
	void deserialize_arg(char*& buffer, std::wstring& object)
	{
		uint32_t length = *reinterpret_cast<uint32_t*>(buffer);
		uint32_t length2 = length / 2;
		object.resize(length2);
		buffer += sizeof(uint32_t);
		for (uint32_t i = 0; i < length2; i++)
			object[i] = reinterpret_cast<uint16_t*>(buffer)[i];
		buffer += length;
	}
	
	template <class C,class R, class... args>
	R invoke(R(C::* member_fun)(args...), C* this_ptr, char* buffer)
	{
		R result_value;
		invoke_helper<preset_nodify_rule::value_to_ref, char*, extractor_type_select>::type<R(C::*)(args...), C*, R, args...>::
			exec(member_fun, this_ptr, &result_value, buffer);
		return result_value;
	}

	template <class C, class... args>
	void invoke(void (C::* member_fun)(args...), C* this_ptr, char* buffer)
	{
		invoke_helper<preset_nodify_rule::value_to_ref, char*, extractor_type_select>::type<void(C::*)(args...), C*, void, args...>::
			exec(member_fun, this_ptr, nullptr, buffer);
	}

}

#endif
