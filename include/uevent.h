#ifndef UEVENT_H
#define UEVENT_H
#include<map>
#include<string>
#include<cstdint>
#include<functional>

class uevent_listener{
	public:
		using handler=std::function<bool(uint64_t id,std::map<std::string,std::string>&uevent)>;
		virtual uint64_t listen(const handler&h)=0;
		virtual void unlisten(uint64_t id)=0;
		static uevent_listener*get();
		static uevent_listener*create();
		static uint64_t add(const handler&h);
		static void remove(uint64_t id);
};
#endif
