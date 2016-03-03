// Server.cpp : �������̨Ӧ�ó������ڵ㡣
//http://www.cnblogs.com/lzjsky/archive/2011/05/05/2037254.html

#include "stdafx.h"

#include <iostream> 
#include <boost/asio.hpp>
using namespace boost::asio; 
int main(int argc, char* argv[]) 
{ 
	// ����asio�඼��Ҫio_service���� 
	io_service iosev; 
	ip::tcp::acceptor acceptor(iosev, 
	ip::tcp::endpoint(ip::tcp::v4(), 2000)); 
	for(;;) 
	{ 
		// socket���� 
		ip::tcp::socket socket(iosev); 
		// �ȴ�ֱ���ͻ������ӽ��� 
		acceptor.accept(socket); 
		// ��ʾ���ӽ����Ŀͻ��� 
		std::cout << socket.remote_endpoint().address() << std::endl; 
		// ��ͻ��˷���hello world! 
		boost::system::error_code ec; 
		socket.write_some(buffer("hello world!"), ec); 
		// ���������ӡ������Ϣ 
		if(ec) 
		{ 
			std::cout <<boost::system::system_error(ec).what() << std::endl; 
			break; 
		} 
		// �뵱ǰ�ͻ�������ɺ�ѭ�������ȴ���һ�ͻ����� 
	} 
	return 0; 
}