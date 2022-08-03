#ifndef PARAMETER_PACK_MODIFY_H
#define PARAMETER_PACK_MODIFY_H
namespace modern_framework
{
    template< template<class..._args> class template_type ,class specialized_type, class... args>
    class if_empty_args_select
    {
    public:
        using type=template_type<args...>;
    };

    template<template<class..._args> class template_type,class specialized_type>
    class if_empty_args_select<template_type,specialized_type>
    {
        using type=specialized_type;
    };

    template <template <class T> class _modify_rule, template <class... _function_args> class _callback_class, class... _fixed_args>
	class parameters_pack_modify
	{
		template <class T>
		using F = typename _modify_rule<T>::type;

		template <class... args>
		class parameter_pack_expansion;

		template <class T, class... pargs>
		class parameter_pack_expansion<T, pargs...>
		{
		public:
			using head_type = T;
			using next = parameter_pack_expansion<pargs...>;
			template <class... cargs>
			class construct_new_parameter_pack
			{
			public:
				using result_type = typename next::template construct_new_parameter_pack<cargs..., F<typename next::head_type>>::result_type;
			};

			class begin
			{
			public:
				using result_type = typename construct_new_parameter_pack<F<T>>::result_type;
			};
		};

		template <class T>
		class parameter_pack_expansion<T>
		{
		public:
			using head_type = T;
			template <class... cargs>
			class construct_new_parameter_pack
			{
			public:
				using result_type = construct_new_parameter_pack;
				using type = _callback_class<_fixed_args..., cargs...>;
				inline static void exec(_fixed_args... fixed_args, cargs... args)
				{
					_callback_class<cargs...>::exec(std::forward<_fixed_args>(fixed_args)..., std::forward<cargs>(args)...);
				}
			};
			class begin
			{
			public:
				using result_type = typename construct_new_parameter_pack<F<T>>::result_type;
			};
		};

		class parameter_pack_expansion_no_args
		{
			class construct_new_parameter_pack
			{
			public:
				using result_type = construct_new_parameter_pack;
				using type = _callback_class<>;
				inline static void exec(_fixed_args... fixed_args)
				{
					_callback_class<>::exec(fixed_args...);
				}
			};

		public:
			class begin
			{
			public:
				using result_type = typename construct_new_parameter_pack::result_type;
			};
		};

	public:
		template <class... _type_args>
		using modify = typename if_empty_args_select<parameter_pack_expansion,parameter_pack_expansion_no_args,_type_args...>::type::begin::result_type;
	};

}
#endif