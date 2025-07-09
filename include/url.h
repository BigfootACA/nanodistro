#ifndef URL_H
#define URL_H
#include<string>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include"net-utils.h"
class url{
	public:
		url(){clear();}
		url(const url&str);
		url(const char*u,size_t len);
		explicit url(const char*u);
		explicit url(const std::string&str);
		explicit url(const std::string*str);
		explicit url(const url*str);
		void parse(const std::string*u);
		void parse(const std::string&u);
		void parse(const char*u);
		void parse(const char*u,size_t len);
		void clear();
		bool go_back();
		url relative(const std::string&path)const;
		void from(const url*url);
		void from(const url&url);
		size_t append_all(std::string&val)const;
		size_t append_origin(std::string&val)const;
		size_t append_hierarchical(std::string&val)const;
		size_t append_user_info(std::string&val)const;
		size_t append_authority(std::string&val)const;
		size_t append_full_path(std::string&val)const;
		size_t append_scheme(std::string&val)const;
		size_t append_username(std::string&val)const;
		size_t append_password(std::string&val)const;
		size_t append_host(std::string&val)const;
		size_t append_port(std::string&val)const;
		size_t append_path(std::string&val)const;
		size_t append_query(std::string&val)const;
		size_t append_fragment(std::string&val)const;
		void set_port(const std::string&val);
		size_t to_sock_addr(socket_address&addr,int prefer=0)const;
		std::string get_origin()const;
		std::string get_hierarchical()const;
		std::string get_user_info()const;
		std::string get_authority()const;
		std::string get_full_path()const;
		std::string get_url()const;
		bool is_in_top()const;
		std::string to_string()const;
		std::string get_scheme()const;
		std::string get_username()const;
		std::string get_password()const;
		std::string get_host()const;
		int get_port()const;
		std::string get_path()const;
		std::string get_query()const;
		std::string get_fragment()const;
		void set_scheme(const std::string&val);
		void set_username_decoded(const std::string&val);
		void set_password_decoded(const std::string&val);
		void set_host_decoded(const std::string&val);
		void set_port(int val);
		void set_path_decoded(const std::string&val);
		void set_query(const std::string&val);
		void set_fragment(const std::string&val);
		void set_username(const std::string&val);
		void set_password(const std::string&val);
		void set_host(const std::string&val);
		void set_path(const std::string&val);
		void set_scheme(const char*val);
		void set_username(const char*val);
		void set_password(const char*val);
		void set_host(const char*val);
		void set_port(const char*val);
		void set_path(const char*val);
		void set_query(const char*val);
		void set_fragment(const char*val);
		void set_username_decoded(const char*val);
		void set_password_decoded(const char*val);
		void set_host_decoded(const char*val);
		void set_path_decoded(const char*val);
		void set_scheme(const char*val,size_t len);
		void set_username(const char*val,size_t len);
		void set_password(const char*val,size_t len);
		void set_host(const char*val,size_t len);
		void set_port(const char*val,size_t len);
		void set_path(const char*val,size_t len);
		void set_query(const char*val,size_t len);
		void set_fragment(const char*val,size_t len);
		void set_username_decoded(const char*val,size_t len);
		void set_password_decoded(const char*val,size_t len);
		void set_host_decoded(const char*val,size_t len);
		void set_path_decoded(const char*val,size_t len);
		int compare(const url*u)const;
		int compare(const std::string&u)const;
		int compare(const url&u)const;
		int compare(const char*str)const;
		int compare(const std::string*u)const;
		bool equals(const char*str)const;
		bool equals(const std::string*u)const;
		bool equals(const std::string&u)const;
		bool equals(const url&u)const;
		bool equals(const url*u)const;
		url&operator=(const char*u);
		url&operator=(std::string*u);
		url&operator=(std::string&u);
		url&operator=(const url*u);
		url&operator=(const url&u);
		bool operator==(const char*u)const;
		bool operator==(std::string*u)const;
		bool operator==(std::string&u)const;
		bool operator==(const url*u)const;
		bool operator==(const url&u)const;
		bool operator!=(const char*u)const;
		bool operator!=(std::string*u)const;
		bool operator!=(std::string&u)const;
		bool operator!=(const url*u)const;
		bool operator!=(const url&u)const;
		bool operator>(const url*u)const;
		bool operator>(const url&u)const;
		bool operator>=(const url*u)const;
		bool operator>=(const url&u)const;
		bool operator<(const url*u)const;
		bool operator<(const url&u)const;
		bool operator<=(const url*u)const;
		bool operator<=(const url&u)const;
		std::ostream&operator<<(std::ostream&os)const;
		explicit operator std::string()const;
		static std::string encode(
			const std::string&src,
			const char*map=nullptr,
			const std::string&skip=""
		);
		static std::string decode(const std::string&src);
	private:
		std::string scheme;
		std::string username;
		std::string password;
		std::string host;
		int port=-1;
		std::string path;
		std::string query;
		std::string fragment;
};
#endif
