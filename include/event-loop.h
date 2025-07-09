#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include<functional>
#include<sys/epoll.h>

class event_loop;
struct event_handler_data;
struct event_handler_context;

struct event_handler_data{
	uint64_t id=0;
	int fd=-1;
	uint32_t events=0;
	uint32_t error=0;
	uint32_t max_error=0;
	uint32_t event_total=0;
	bool want_remove=false;
	std::function<void(event_handler_context*ev)>handler;
};

enum event_callback_type{
	type_init,
	type_events,
	type_error,
	type_fatal,
	type_remove,
};

struct event_handler_context{
	event_loop*loop=nullptr;
	event_handler_data*ev=nullptr;
	epoll_event*epoll=nullptr;
	event_callback_type type;
	uint32_t event=0;
};

class event_loop{
	friend struct event_handler_context;
	public:
		using event_handler=std::function<void(event_handler_context*ev)>;
		using job_handler=std::function<void(void)>;
		virtual void interrupt()=0;
		virtual uint64_t add_handler(int fd,uint32_t events,const event_handler&handler)=0;
		virtual uint64_t add_loop_job(const job_handler&handler)=0;
		virtual void remove_loop_job(const uint64_t&id)=0;
		virtual void remove_handler(const uint64_t&id)=0;
		virtual void close_fd(int fd)=0;
		virtual void process_event(epoll_event*ev)=0;
		virtual void process_events(epoll_event*evs,int count)=0;
		virtual void run_once_loop(int timeout=-1)=0;
		virtual void became_default(bool force=false)=0;
		virtual int run()=0;
		virtual void start()=0;
		virtual void stop(int ret=0)=0;
		static int create_epoll();
		static event_loop*get();
		static event_loop*get_start();
		static event_loop*create();
		static event_loop*create(int fd);
		static uint64_t push_handler(int fd,uint32_t events,const event_handler&handler);
		static uint64_t push_loop_job(const job_handler&handler);
		static void pop_handler(uint64_t id);
};
#endif
