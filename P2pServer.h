/**
 * @file P2pServer.h
 * @author maileizhi (359489892@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __JUNGLE_P2P_SERVER_H__
#define __JUNGLE_P2P_SERVER_H__

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

class P2pServer {
public:
	P2pServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);

	void start();
    
private:
	// 处理用户的连接创建和断开  epoll listenfd accept
	void onConnection(const TcpConnectionPtr& conn);

	// 处理用户的读写事件
	void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);

	TcpServer _server; // #1
	EventLoop* _loop;  // #2 epoll
};

#endif // !__JUNGLE_P2P_SERVER_H__