#ifndef MODIFY_RULE_H
#define MODIFY_RULE_H

namespace modern_framework
{
	namespace preset_nodify_rule
	{
		template<class T>
		class value_to_ref
		{
		public:
			using type = T&;
		};

		template<class T>
		class value_to_ref<const T>
		{
		public:
			using type = const T&;
		};

		template<class T>
		class value_to_ref<T*>
		{
		public:
			using type = T*;
		};

		template<class T>
		class value_to_ref<const T*>
		{
		public:
			using type = const T*;
		};

		template<class T>
		class value_to_ref<T*&>
		{
		public:
			using type = T*;
		};

		template<class T>
		class value_to_ref<const T*&>
		{
		public:
			using type = const T*;
		};

		template<class T>
		class value_to_ref<T&>
		{
		public:
			using type = T&;
		};
		template<class T>
		class value_to_ref<const T&>
		{
		public:
			using type = const T&;
		};

		template<class T>
		class value_to_ref<T&&>
		{
		public:
			using type = T&;
		};

		
	}
}

#endif
