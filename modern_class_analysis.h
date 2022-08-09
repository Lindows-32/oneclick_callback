#ifndef MODERN_CLASS_ANALYSIS_H
#define MODERN_CLASS_ANALYSIS_H
#include <cstdint>
namespace modern_framework
{

	struct can
	{
		static constexpr bool value = true;
	};
	struct can_t
	{
		static constexpr bool value = false;
	};
	template <class T1,class T2>
	struct is_same
	{
		static constexpr bool result=false;
	};
	template <class T>
	struct is_same<T,T>
	{
		static constexpr bool result=true;
	};

	template <class T>
	class type
	{

	public:
		class is_class
		{
			struct is_complex
			{
				static constexpr bool result=true;
			};
			struct is_simple
			{
				static constexpr bool result=false;
			};
			template <class C>
			static is_complex check(int C::*);

			template <class C>
			static is_simple check(...);

		public:
			static constexpr bool result = decltype(check<T>(nullptr))::value;
		};
#define IS(type) is_same<T,type>::result
		class is_basic_type
		{
		public:
			static constexpr bool result = IS(bool) || IS(char) || IS(wchar_t) || IS(float) || IS(double) || IS(long double)
				|| IS(int8_t) || IS(uint8_t) || IS(int16_t) || IS(uint16_t)
				|| IS(int32_t) || IS(uint32_t) || IS(int64_t) || IS(uint64_t);
		};
#undef  IS

		class is_pointer
		{
			static constexpr bool result = false;
		};

		template <class Target>
		class can_cast_to
		{
			static can check(Target t);
			static can_t check(...);

		public:
			static constexpr bool result = decltype(check(*(T *)nullptr))::value;
		};

		template <class Source>
		class can_cast_from
		{
			static can check(T);
			static can_t check(...);

		public:
			static constexpr bool result = decltype(check(*(Source *)nullptr))::value;
		};

		template <class Source_array, size_t size>
		class can_cast_from<const Source_array (&)[size]>
		{
			static can check(T);
			static can_t check(...);
			typedef const Source_array (&array_type)[size];
			static constexpr array_type m = {};

		public:
			static constexpr bool result = decltype(check(m))::value;
		};
	};

	template <class T>
	class type<T *>
	{
	public:
		class is_pointer
		{
		public:
			static constexpr bool result = true;
		};

		class is_class
		{
		public:
			static constexpr bool result = false;
		};

		class is_basic_type
		{
		public:
			static constexpr bool result = false;
		};

		template <class Target>
		class can_cast_to
		{
			static can check(Target t);
			static can_t check(...);

		public:
			static constexpr bool result = decltype(check((T *)nullptr))::value;
		};
		template <class Source>
		class can_cast_from
		{
			static can check(T *t);
			static can_t check(...);

		public:
			static constexpr bool result = decltype(check(*(Source *)nullptr))::value;
		};

		template <class Source_array, size_t size>
		class can_cast_from<const Source_array (&)[size]>
		{
			static can check(T *);
			static can_t check(...);
			typedef const Source_array (&array_type)[size];
			static constexpr array_type m = {};

		public:
			static constexpr bool result = decltype(check(m))::value;
		};
	};
	template <size_t index, size_t target_index, class head, class... args_s>
	class get_argtype_helper
	{
	public:
		using type = typename get_argtype_helper<index + 1, target_index, args_s...>::type;
	};

	template <size_t target_index, class head, class... args_s>
	class get_argtype_helper<target_index, target_index, head, args_s...>
	{
	public:
		using type = head;
	};

	template <size_t target_index, class... args_s>
	class get_argtype_at
	{
	public:
		using type = typename get_argtype_helper<0, target_index, args_s...>::type;
	};
	template <size_t sizeof_pointer>
	struct pointer_conv2integer;

	template <>
	struct pointer_conv2integer<1U>
	{
		using type = uint8_t;
	};

	template <>
	struct pointer_conv2integer<2U>
	{
		using type = uint16_t;
	};

	template <>
	struct pointer_conv2integer<4U>
	{
		using type = uint32_t;
	};

	template <>
	struct pointer_conv2integer<8U>
	{
		using type = uint64_t;
	};

	using ptr2int = pointer_conv2integer<sizeof(void *)>::type;

	template <class T>
	class cast_meta;

	template <class T, bool is_class>
	class non_class_type;

	template <class T>
	class non_class_type<T, true>
	{
	};

	template <bool condition, class yes, class no>
	class IF;

	template <class yes, class no>
	class IF<true, yes, no>
	{
	public:
		using THEN = yes;
	};

	template <class yes, class no>
	class IF<false, yes, no>
	{
	public:
		using THEN = no;
	};

	template <class... args>
	class compare_classes;

	template <class T, class... args>
	class compare_classes<T, args...>
	{
	public:
		static constexpr size_t max_size = sizeof(T) >= compare_classes<args...>::max_size ? sizeof(T) : compare_classes<args...>::max_size;
		static constexpr size_t min_size = sizeof(T) < compare_classes<args...>::min_size ? sizeof(T) : compare_classes<args...>::max_size;
		using get_max = typename IF<sizeof(T) >= compare_classes<args...>::max_size, T, typename compare_classes<args...>::get_max>::THEN;
		using get_min = typename IF<sizeof(T) < compare_classes<args...>::min_size, T, typename compare_classes<args...>::get_min>::THEN;
	};
	template <class T>
	class compare_classes<T>
	{
	public:
		static constexpr size_t max_size = sizeof(T);
		static constexpr size_t min_size = sizeof(T);
		using get_max = T;
		using get_min = T;
	};

}

#endif