#ifndef WLAN_INTERNAL_H
#define WLAN_INTERNAL_H
#include"wlan.h"
#include<atomic>
#include<semaphore>
#include"event-loop.h"

class wlan_client_impl:public wlan_client,public std::enable_shared_from_this<wlan_client_impl>{
	friend class wlan_poll_worker;
	public:
		inline ~wlan_client_impl()override{close();}
		std::string recv();
		void send(const std::string&cmd);
		void run(const std::string&cmd,const std::string&except="OK");
		std::string exec(const std::string&cmd);
		void open(const std::string&dev);
		void close()override;
		void terminate()override;
		void start_scan()override;
		std::map<std::string,std::string>get_status()override;
		std::list<std::shared_ptr<wlan_scan_result>>scan_results()override;
		std::list<std::shared_ptr<wlan_network>>list_networks()override;
		std::shared_ptr<wlan_network>add_network()override;
		uint64_t listen_event(const std::function<void(uint64_t id,const std::string&)>&cb)override;
		void unlisten_event(uint64_t id)override;
	private:
		bool read_event();
		void process_data(const std::string&data);
		void on_event(event_handler_context*ev);
		void handle_event(int level,const std::string&msg);
		void handle_data(const std::string&data);
		std::string recv_();
		void send_(const std::string&cmd);
		std::string exec_(const std::string&cmd);
		std::map<uint64_t,std::function<void(uint64_t id,const std::string&)>>on_events{};
		std::atomic<uint64_t>event_id{0};
		std::string buffer{},local_sock{},dev{};
		std::mutex lock{};
		std::mutex data_lock{};
		std::mutex event_lock{};
		uint64_t poll_id=0;
		std::binary_semaphore sem{0};
		int fd=-1;
};

class wlan_network_impl:public wlan_network{
	public:
		wlan_network_impl(const std::shared_ptr<wlan_client_impl>&clt,const wlan_network_item&item);
		~wlan_network_impl()override=default;
		void set_value(const std::string&var,const std::string&val)override;
		std::string get_value(const std::string&var)override;
		void select()override;
		void enable()override;
		void disable()override;
		void remove()override;
	private:
		std::shared_ptr<wlan_client_impl>cur_wifi();
		std::weak_ptr<wlan_client_impl>wifi;
};

extern std::string get_wpa_supplicant_dir();
extern void start_wpa_supplicant(const std::string&dev);

#endif
