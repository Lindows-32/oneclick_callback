#ifndef INVOKE_HELPER
#define INVOKE_HELPER

namespace modern_framework
{
	template<template <class _T> class _modify_rule, class _data_source_type, template<size_t _index, class _T, class _target_type> class _param_extractor_type>
	class invoke_helper
	{
	
		template<class T>
		using F = typename _modify_rule<T>::type;

		template<class _member_function, class _class_type, class _return_value_type, class... pargs>
		class parameter_pack_expansion;

		template<class _member_function, class _class_type, class _return_value_type, class T, class... pargs>
		class parameter_pack_expansion<_member_function, _class_type, _return_value_type, T, pargs...>
		{
		public:
			using head_type = T;
			using next = parameter_pack_expansion<_member_function, _class_type, _return_value_type, pargs...>;

			template<class member_function, class class_type, class return_value_type, class... cargs>
			class construct_new_parameter_pack
			{
			public:
				static constexpr size_t current_index = sizeof...(cargs);
				using result_type = typename next::template construct_new_parameter_pack<member_function, class_type, return_value_type, cargs..., F<head_type>>;
				inline static void exec(member_function memfun_ptr, class_type this_ptr, return_value_type* res_ptr, _data_source_type& data_source, cargs... args)
				{
					typename _param_extractor_type<current_index, _data_source_type, F<head_type>>::type data(data_source);
					result_type::exec(memfun_ptr, this_ptr, res_ptr, data_source, args..., data);
				}
			};
			template<class member_function, class class_type, class return_value_type>
			class construct_new_parameter_pack<member_function, class_type, return_value_type>
			{
			public:
				static constexpr size_t current_index = 0;
				using result_type = typename next::template construct_new_parameter_pack<member_function, class_type, return_value_type, F<head_type>>;
				inline static void exec(member_function memfun_ptr, class_type this_ptr, return_value_type* res_ptr, _data_source_type& data_source)
				{
					typename _param_extractor_type<current_index, _data_source_type, F<head_type>>::type data(data_source);
					result_type::exec(memfun_ptr, this_ptr, res_ptr, data_source, data);
				}
			};

			using result_type = construct_new_parameter_pack<_member_function, _class_type, _return_value_type>;
		};

		template<class T>
		class call_funptr;

		template<class C, class R,class... args>
		class call_funptr<R(C::*)(args...)>
		{
			template<class T>
			class maybe_move
			{
			public:
				constexpr static T exec(T arg) noexcept
				{
					return arg;
				}
			};
			template<class T>
			class maybe_move<T&&>
			{
			public:
				constexpr static T&& exec(T& arg) noexcept
				{
					return std::move(arg);
				}
			};
		public:
			template<class member_function, class class_type, class return_value_type, class... fargs>
			inline static void exec(member_function memfun_ptr, class_type this_ptr, return_value_type* res_ptr, fargs... final_args)
			{
				*res_ptr =(this_ptr->*memfun_ptr)(std::move(final_args)...);
			}

			template<class member_function, class class_type, class... fargs>
			inline static void exec(member_function memfun_ptr, class_type this_ptr,void* res_ptr, fargs... final_args)
			{
				(this_ptr->*memfun_ptr)(maybe_move<args>::exec(final_args)...);
			}
		};

		template<class _member_function, class _class_type, class _return_value_type, class T>
		class parameter_pack_expansion<_member_function, _class_type, _return_value_type, T>
		{
		public:
			using head_type = T;
			template<class member_function, class class_type, class return_value_type, class... cargs>
			class construct_new_parameter_pack
			{
			public:
				static constexpr size_t current_index = sizeof...(cargs);
				using result_type = construct_new_parameter_pack;
				inline static void exec(member_function memfun_ptr, class_type this_ptr, return_value_type* res_ptr, _data_source_type& data_source, cargs... args)
				{
					typename _param_extractor_type<current_index, _data_source_type, F<head_type>>::type data(data_source);
					call_funptr<member_function>::template exec<member_function, class_type, cargs...>(memfun_ptr, this_ptr, res_ptr, args..., data);
				}
			};
		};

		template<class _member_function, class _class_type, class T>
		class parameter_pack_expansion<_member_function, _class_type, void, T>
		{
		public:
			using head_type = T;
			template<class member_function, class class_type, class return_value_type, class... cargs>
			class construct_new_parameter_pack
			{

			public:
				static constexpr size_t current_index = sizeof...(cargs);
				inline static void exec(member_function memfun_ptr, class_type this_ptr, return_value_type* res_ptr, _data_source_type& data_source, cargs... args)
				{
					typename _param_extractor_type<current_index, _data_source_type, F<head_type>>::type data(data_source);
					call_funptr<member_function>::template exec<member_function, class_type, cargs...>(memfun_ptr, this_ptr, res_ptr,args..., data);
				}
			};
		};

		template<class member_function, class class_type, class return_value_type>
		class parameter_pack_expansion<member_function, class_type, return_value_type>
		{
		public:
			using result_type = parameter_pack_expansion;
			inline static void exec(member_function memfun_ptr, class_type this_ptr, return_value_type* res_ptr, _data_source_type& data_source)
			{
				call_funptr<member_function>::template exec<member_function, class_type>(memfun_ptr, this_ptr, res_ptr);
			}
		};
	public:
		template<class _class_type, class _member_function_type, class _return_type, class... _args >
		using type = typename parameter_pack_expansion<_class_type, _member_function_type, _return_type, _args...>::result_type;
	};
}

#endif