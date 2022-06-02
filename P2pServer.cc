/**
 * @file P2pServer.cc
 * @author maileizhi (359489892@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "P2pServer.h"

P2pServer::P2pServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg) :
	_server(loop, listenAddr, nameArg, TcpServer::Option::kReusePort), _loop(loop) {
	// 给服务器注册用户连接的创建和断开回调
	_server.setConnectionCallback(std::bind(&P2pServer::onConnection, this, _1));

	// 给服务器注册用户读写事件回调
	_server.setMessageCallback(std::bind(&P2pServer::onMessage, this, _1, _2, _3));

	// 设置服务器端的线程数量 1个I/O线程   5个worker线程
	//也可以提高获取系统核心数进行设置
	//使用sysconf()或者get_nprocs()来获取处理器核数
	_server.setThreadNum(6);
}

void P2pServer::start() {
	_server.start();
}

// 专门处理用户的连接创建和断开  epoll listenfd accept
void P2pServer::onConnection(const TcpConnectionPtr& conn) {
	if (conn->connected()) {
		cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:online" << endl;
	} else {
		cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:offline" << endl;
		conn->shutdown();// close(fd)
		//_loop->quit();
	}
}

// 专门处理用户的读写事件
// 注:接收到数据的时间信息(UTC) 需要自行+0800才是北京时间
void P2pServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) {
	string buf = buffer->retrieveAllAsString();
	cout << "recv data:" << buf << " time:" << time.toFormattedString() << endl;
	conn->send(buf);
}
