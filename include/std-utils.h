#ifndef LIB_STD_UTILS_H
#define LIB_STD_UTILS_H
#include<memory>
#include<algorithm>
#include"error.h"
template<typename L,typename T>
static inline auto std_find_all(const L&list,const T&item){
	return std::find(list.begin(),list.end(),item);
}
template<typename L,typename T>
static inline bool std_contains(const L&list,const T&item){
	return std::find(list.begin(),list.end(),item)!=list.end();
}
template<typename L,typename T>
static inline bool std_contains_key(const L&map,const T&item){
	return map.find(item)!=map.end();
}
template<typename L,typename T>
static inline void std_remove_item(L&list,const T&item){
	auto idx=std::find(list.begin(),list.end(),item);
	if(idx!=list.end())list.erase(idx);
}
template<typename M,typename K>
static inline void std_remove_map_item(M&map,const K&item){
	auto idx=map.find(item);
	if(idx!=map.end())map.erase(idx);
}
template<typename L>
static inline void std_add_list(L&list,const L&add){
	list.insert(list.end(),add.begin(),add.end());
}
template<typename M>
static inline void std_add_map(M&map,const M&add){
	for(const auto&[l,r]:add)
		map[l]=r;
}
template<typename L>
static inline void std_deduplicate(L&list){
	list.sort();
	auto last=std::unique(list.begin(),list.end());
	list.erase(last,list.end());
}
template<typename T,typename U>std::shared_ptr<T>std_check_cast(std::shared_ptr<U>v){
	auto c=std::static_pointer_cast<T,U>(v);
	if(!c)throw RuntimeError("invalid pointer");
	return c;
}
#define align_up(val,alg) (((val)+(alg)-1)&~((alg)-1))
#define align_down(val,alg) ((val)&~((alg)-1))
#define div_round_up(n,d) (((n)+(d)-1)/(d))
#if 1
#define likely(x) (!!(x))
#define unlikely(x) (!!(x))
#else
#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)
#endif
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define set_bit(var,val,bit)do{\
        if(bit)(var)=((typeof(var))((var)|(val)));\
        else (var)=((typeof(var))((var)&(~(val))));\
}while(0)
#define have_bit(var,bit)(((var)&(bit))==(bit))
#define fallthrough
#endif
