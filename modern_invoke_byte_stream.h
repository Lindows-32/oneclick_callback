#ifndef MODERN_INVOKE_BYTE_STREAM_H
#define MODERN_INVOKE_BYTE_STREAM_H
#include <vector>
#include <string>
#include <list>
#include <map>
#include "modern_invoke_helper.h"
#include "modern_modify_rule.h"
#include "modern_class_analysis.h"
namespace modern_framework
{
	template <class T>
	void deserialize_arg(char *&buffer, T &object)
	{
		uint32_t length = *reinterpret_cast<uint32_t *>(buffer);
		buffer += sizeof(uint32_t);
		object = *reinterpret_cast<T *>(buffer);
		buffer += length;
	}

	template <size_t index, class T>
	class extractor_type_select<index, char *, T &>
	{
		class for_basic_type
		{
			char value[sizeof(long double)];

		public:
			inline for_basic_type(char *&data_source)
			{
				*reinterpret_cast<T *>(value) = *reinterpret_cast<T *>(data_source);
				data_source += sizeof(T);
			}
			inline for_basic_type(char *&data_source, T &prealloced)
			{
				prealloced = *reinterpret_cast<T *>(data_source);
				data_source += sizeof(T);
			}
			inline operator T &()
			{
				return *reinterpret_cast<T *>(value);
			}
			inline operator T *()
			{
				return reinterpret_cast<T *>(value);
			}
		};
		class for_class_type
		{
			uint8_t space[sizeof(T)];
			bool need2free;

		public:
			inline for_class_type(char *&data_source)
			{
				new (space) T;
				need2free = true;
				deserialize_arg(data_source, *reinterpret_cast<T *>(space));
			}
			inline for_class_type(char *&data_source, T &prealloced)
			{
				need2free = false;
				deserialize_arg(data_source, prealloced);
			}
			inline operator T &()
			{
				return *reinterpret_cast<T *>(space);
			}
			inline operator T *()
			{
				return reinterpret_cast<T *>(space);
			}
			~for_class_type()
			{
				if (need2free)
					reinterpret_cast<T *>(space)->~T();
			}
		};

	public:
		using type = typename IF<type<T>::is_basic_type::result, for_basic_type, for_class_type>::THEN;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, const T &>
	{
	public:
		using type = typename extractor_type_select<index, char *, T &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, T *>
	{
	public:
		using type = typename extractor_type_select<index, char *, T &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, const T *>
	{
	public:
		using type = typename extractor_type_select<index, char *, T &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, std::vector<T> &>
	{
		class for_class_type
		{
			uint8_t space[sizeof(std::vector<T>)];
			bool need2free;
			void vector_construct_common(char *&data_source, std::vector<T> &value)
			{
				uint32_t length = *reinterpret_cast<uint32_t *>(data_source);
				value.resize(length);
				data_source += sizeof(length);
				for (uint32_t i = 0; i < length; i++)
				{
					typename extractor_type_select<index, char *, T &>::type data(data_source, value[i]);
				}
			}

		public:
			inline for_class_type(char *&data_source)
			{
				new (space) std::vector<T>;
				need2free = true;
				vector_construct_common(data_source, *reinterpret_cast<std::vector<T> *>(space));
			}
			inline for_class_type(char *&data_source, std::vector<T> &prealloced)
			{
				need2free = false;
				vector_construct_common(data_source, prealloced);
			}
			inline operator std::vector<T> &()
			{
				return *reinterpret_cast<std::vector<T> *>(space);
			}
			inline operator std::vector<T> *()
			{
				return reinterpret_cast<std::vector<T> *>(space);
			}
			~for_class_type()
			{
				if (need2free)
					reinterpret_cast<std::vector<T> *>(space)->~vector();
			}
		};

	public:
		using type = for_class_type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, const std::vector<T> &>
	{
	public:
		using type = typename extractor_type_select<index, char *, std::vector<T> &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, std::vector<T> *>
	{
	public:
		using type = typename extractor_type_select<index, char *, std::vector<T> &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, const std::vector<T> *>
	{
	public:
		using type = typename extractor_type_select<index, char *, std::vector<T> &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, std::list<T> &>
	{
		class for_class_type
		{
			uint8_t space[sizeof(std::list<T>)];
			bool need2free;

			void list_construct_common(char *&data_source, std::list<T> &value)
			{
				uint32_t length = *reinterpret_cast<uint32_t *>(data_source);
				value.resize(length);
				data_source += sizeof(length);
				typename std::list<T>::iterator it = value.begin();
				for (uint32_t i = 0; i < length; i++)
				{
					typename extractor_type_select<index, char *, T &>::type data(data_source, *it);
					it++;
				}
			}

		public:
			inline for_class_type(char *&data_source)
			{
				new (space) std::list<T>;
				need2free = true;
				list_construct_common(data_source, *reinterpret_cast<std::list<T> *>(space));
			}
			inline for_class_type(char *&data_source, std::list<T> &prealloced)
			{
				need2free = false;
				list_construct_common(data_source, prealloced);
			}
			inline operator std::list<T> &()
			{
				return *reinterpret_cast<std::list<T> *>(space);
			}
			inline operator std::list<T> *()
			{
				return reinterpret_cast<std::list<T> *>(space);
			}
			~for_class_type()
			{
				if (need2free)
					reinterpret_cast<std::list<T> *>(space)->~list();
			}
		};

	public:
		using type = for_class_type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, std::list<T> *>
	{
	public:
		using type = typename extractor_type_select<index, char *, std::list<T> &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, const std::list<T> *>
	{
	public:
		using type = typename extractor_type_select<index, char *, std::list<T> &>::type;
	};

	template <size_t index, class T>
	class extractor_type_select<index, char *, const std::list<T> &>
	{
	public:
		using type = typename extractor_type_select<index, char *, std::list<T> &>::type;
	};

	template <size_t index, class _Key, class _Value>
	class extractor_type_select<index, char *, std::map<_Key, _Value> &>
	{
		
		class for_class_type
		{
			uint8_t space[sizeof(std::map<_Key, _Value>)];
			bool need2free;
			void map_construct_common(char *&data_source, std::map<_Key, _Value> &value)
			{
				uint32_t length = *reinterpret_cast<uint32_t *>(data_source);
				data_source += sizeof(length);
				for (uint32_t i = 0; i < length; i++)
				{
					std::pair<_Key, _Value> it;
					typename extractor_type_select<index, char *, _Key &>::type data_key(data_source, it.first);
					typename extractor_type_select<index, char *, _Value &>::type data_value(data_source, it.second);
					value.insert(std::move(it));
				}
			}

		public:
			inline for_class_type(char *&data_source)
			{
				new (space) std::map<_Key, _Value>;
				need2free = true;
				map_construct_common(data_source, *reinterpret_cast<std::map<_Key, _Value> *>(space));
			}
			inline for_class_type(char *&data_source, std::map<_Key, _Value> &prealloced)
			{
				need2free = false;
				map_construct_common(data_source, prealloced);
			}
			inline operator std::map<_Key, _Value> &()
			{
				return *reinterpret_cast<std::map<_Key, _Value> *>(space);
			}
			inline operator std::map<_Key, _Value> *()
			{
				return reinterpret_cast<std::map<_Key, _Value> *>(space);
			}
			~for_class_type()
			{
				if (need2free)
					reinterpret_cast<std::map<_Key, _Value> *>(space)->~map();
			}
		};
	public:
		using type = for_class_type;
	};

	template <size_t index, class _Key, class _Value>
	class extractor_type_select<index, char *,const std::map<_Key, _Value> &>
	{
	public:
		using type =  typename extractor_type_select<index, char *,std::map<_Key, _Value> &>::for_class_type;
	};

	template <size_t index, class _Key, class _Value>
	class extractor_type_select<index, char *,std::map<_Key, _Value> *>
	{
	public:
		using type =  typename extractor_type_select<index, char *,std::map<_Key, _Value> &>::for_class_type;
	};

	template <size_t index, class _Key, class _Value>
	class extractor_type_select<index, char *,const std::map<_Key, _Value> *>
	{
	public:
		using type =  typename extractor_type_select<index, char *,std::map<_Key, _Value> &>::for_class_type;
	};
	template <size_t index>
	class extractor_type_select<index, char *, const char *>
	{
		class for_basic_type
		{
			const char *value;

		public:
			inline for_basic_type(char *&data_source)
			{
				uint32_t length = *reinterpret_cast<uint32_t *>(data_source);
				data_source += sizeof(uint32_t);
				value = data_source;
				data_source += length;
			}
			inline for_basic_type(char *&data_source, const char *&prealloced)
			{
				uint32_t length = *reinterpret_cast<uint32_t *>(data_source);
				data_source += sizeof(uint32_t);
				prealloced = data_source;
				data_source += length;
			}
			inline operator const char *()
			{
				return value;
			}
		};

	public:
		using type = for_basic_type;
	};

	template <size_t index>
	class extractor_type_select<index, char *, const char *&>
	{
	public:
		using type = typename extractor_type_select<index, char *, const char *>::type;
	};

	template <size_t index>
	class extractor_type_select<index, char *, const wchar_t *>
	{
		class for_basic_type
		{
			std::wstring value;

		public:
			inline for_basic_type(char *&data_source)
			{
				uint32_t length = *reinterpret_cast<uint32_t *>(data_source);
				uint32_t length2 = length / 2;
				data_source += sizeof(uint32_t);
				value.resize(length2);
				for (uint32_t i = 0; i < length2; i++)
				{
					value[i] = reinterpret_cast<uint16_t *>(data_source)[i];
				}
				data_source += length;
			}
			inline operator const wchar_t *()
			{
				return value.c_str();
			}
		};

	public:
		using type = for_basic_type;
	};

	template <>
	void deserialize_arg(char *&buffer, std::string &object)
	{
		uint32_t length = *reinterpret_cast<uint32_t *>(buffer);
		buffer += sizeof(uint32_t);
		object = std::move(std::string(buffer, buffer + length));
		buffer += length;
	}

	template <>
	void deserialize_arg(char *&buffer, std::wstring &object)
	{
		uint32_t length = *reinterpret_cast<uint32_t *>(buffer);
		uint32_t length2 = length / 2;
		object.resize(length2);
		buffer += sizeof(uint32_t);
		for (uint32_t i = 0; i < length2; i++)
			object[i] = reinterpret_cast<uint16_t *>(buffer)[i];
		buffer += length;
	}

	template <class C, class R, class... args>
	R invoke(R (C::*member_fun)(args...), C *this_ptr, char *buffer)
	{
		R result_value;
		invoke_helper<preset_nodify_rule::value_to_ref, char *, extractor_type_select>::type<R (C::*)(args...), C *, R, args...>::
			exec(member_fun, this_ptr, &result_value, buffer);
		return result_value;
	}

	template <class C, class... args>
	void invoke(void (C::*member_fun)(args...), C *this_ptr, char *buffer)
	{
		invoke_helper<preset_nodify_rule::value_to_ref, char *, extractor_type_select>::type<void (C::*)(args...), C *, void, args...>::
			exec(member_fun, this_ptr, nullptr, buffer);
	}

}

#endif
