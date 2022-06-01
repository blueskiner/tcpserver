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
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写时间的回调函数
5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/
class ChatServer
{
public:
	ChatServer(EventLoop *loop,               // 事件循环
				const InetAddress &listenAddr, // IP+Port
				const string &nameArg)
		: _server(loop, listenAddr, nameArg), _loop(loop)
	{
		// 给服务器注册用户连接的创建和断开回调
		_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

		// 给服务器注册用户读写事件回调
		_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

		// 设置服务器端的线程数量 1个I/O线程   5个worker线程
		//也可以提高获取系统核心数进行设置
		//使用sysconf()或者get_nprocs()来获取处理器核数
		_server.setThreadNum(6);
	}

	void start()
	{
		_server.start();
	}
 
private:
	// 专门处理用户的连接创建和断开  epoll listenfd accept
	void onConnection(const TcpConnectionPtr &conn)
	{
		if (conn->connected())
		{
			cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:online" << endl;
		}
		else
		{
			cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:offline" << endl;
			conn->shutdown(); // close(fd)
			// _loop->quit();
		}
	}

	// 专门处理用户的读写事件
	void onMessage(const TcpConnectionPtr &conn,// 连接
					Buffer *buffer,             // 缓冲区
					Timestamp time)             // 接收到数据的时间信息(UTC)，需要自行+0800才是北京时间
	{
		string buf = buffer->retrieveAllAsString();
		cout << "recv data:" << buf << " time:" << time.toFormattedString() << endl;
		conn->send(buf);
	}

	TcpServer _server; // #1
	EventLoop *_loop;  // #2 epoll
};

int main(int argc, const char** argv) {

	// CTcpServer server;
	// if (!server.init()) {
	// 	return -1;
	// }
	// //server.run(TCP_SERVER_TYPE_SELECT);
	// //server.run(TCP_SERVER_TYPE_POLL);
	// server.run(TCP_SERVER_TYPE_EPOLL);

	EventLoop loop; // epoll
	InetAddress addr("10.0.12.3", 9527);
	ChatServer server(&loop, addr, "ChatServer");
 
	server.start(); // listenfd epoll_ctl=>epoll
	loop.loop();    // epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件等

	return 0;
}
