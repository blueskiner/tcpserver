/**
 * @file main.cc
 * @author maileizhi (359489892@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "CTcpServer.h"
#include "EchoServer.h"
#include "P2pServer.h"

int main(int argc, const char** argv) {

	// CTcpServer server;
	// if (!server.init()) {
	// 	return -1;
	// }
	// //server.run(TCP_SERVER_TYPE_SELECT);
	// //server.run(TCP_SERVER_TYPE_POLL);
	// server.run(TCP_SERVER_TYPE_EPOLL);

	// EventLoop loop;
	// InetAddress addr("10.0.12.3", 9527);
	// EchoServer server(&loop, addr, "EchoServer");

	EventLoop loop;
	InetAddress addr("10.0.12.3", 9527);
	P2pServer server(&loop, addr, "P2pServer");

	server.start(); // listenfd epoll_ctl=>epoll
	loop.loop();    // epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件等

	return 0;
}
