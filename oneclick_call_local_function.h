#ifndef REMOTE_SLOT_H
#define REMOTE_SLOT_H
#include <map>
#include "invoke_byte_stream.h"
namespace modern_framework
{
    using warped_function=void(*)(void*,char*);
    template<class T>
    class from_member_function;

    template<class R,class C,class... args>
    class from_member_function<R(C::*)(args...)>
    {
    public:
        template<R(C::*funptr)(args...)>
        static void exec(void* object_ptr,char* buffer)
        {
            invoke(funptr,reinterpret_cast<C*>(object_ptr),buffer);
        }
    };

    class remote_slot
    {
        struct function_pack
        {
            void* object_ptr;
            warped_function function_ptr;
        };
        std::map<uint32_t,function_pack> _function_map;
    public:
        void parse_and_call(char* buffer)
        {
            uint32_t function_id=*reinterpret_cast<uint32_t*>(buffer);
            std::map<uint32_t,function_pack>::iterator it=_function_map.find(function_id);
            if(it!=_function_map.end())
            {
                it->second.function_ptr(it->second.object_ptr,buffer+sizeof(uint32_t));
            }
        }
        void bind(uint32_t function_id,warped_function funptr,void* object_ptr)
        {
            _function_map[function_id]={object_ptr,funptr};
        }
    };

    #define RS_WRAP(member_function) from_member_function<decltype(member_function)>::exec<member_function>
}


#endif