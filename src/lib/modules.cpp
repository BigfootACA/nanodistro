#include"modules.h"
#ifdef USE_LIB_LIBKMOD
#include<libkmod.h>

class kmod_context{
	public:
		~kmod_context();
		operator kmod_ctx*();
	private:
		kmod_ctx*ctx=nullptr;
};
kmod_context kmod_auto_ctx{};

kmod_context::~kmod_context(){
	if(ctx)kmod_unref(ctx);
	ctx=nullptr;
}

kmod_context::operator kmod_ctx*(){
	if(!ctx)ctx=kmod_new(nullptr,nullptr);
	return ctx;
}

bool module_load(const std::string&name){
	int ret;
	bool success=false;
	kmod_list*list=nullptr,*l;
	kmod_ctx*ctx=kmod_auto_ctx;
	if(!ctx)return false;
	ret=kmod_module_new_from_lookup(ctx,name.c_str(),&list);
	if(ret<0)return false;
	kmod_list_foreach(l,list){
		auto mod=kmod_module_get_module(l);
		if(!mod)continue;
		ret=kmod_module_probe_insert_module(
			mod,0,nullptr,nullptr,nullptr,nullptr
		);
		if(ret>=0)success=true;
		kmod_module_unref(mod);
	}
	kmod_module_unref_list(list);
	return success;
}

#else

bool module_load(const std::string&){
	return false;
}

#endif
