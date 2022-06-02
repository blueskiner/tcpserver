/**
 * @file EchoServer.h
 * @author maileizhi (359489892@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __JUNGLE_ECHO_SERVER_H__
#define __JUNGLE_ECHO_SERVER_H__

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
3.明确TcpServer构造函数需要什么参数，输出EchoServer的构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写时间的回调函数
5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/
class EchoServer {
public:
	EchoServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);

	void start();
 
private:
	// 处理用户的连接创建和断开  epoll listenfd accept
	void onConnection(const TcpConnectionPtr& conn);

	// 处理用户的读写事件
	void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);

	TcpServer _server; // #1
	EventLoop* _loop;  // #2 epoll
};

#endif // !__JUNGLE_ECHO_SERVER_H__