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

#include <muduo/base/Logging.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/Timer.h>
#include <iostream>
#include <functional>
#include <string>
#include <unordered_set>

#include <boost/circular_buffer.hpp>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

class P2pServer {
public:
	P2pServer(EventLoop* loop,
		const InetAddress& listenAddr,
		const string& nameArg,
		int idleSeconds,
		int maxConnections);

	void start();
    
private:
	// 处理用户的连接创建和断开  epoll listenfd accept
	void onConnection(const TcpConnectionPtr& conn);

	// 处理用户的读写事件
	void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);

	void onTimer();

	void dumpConnectionBuckets() const;

	typedef std::weak_ptr<muduo::net::TcpConnection> WeakTcpConnectionPtr;

	struct Entry : public muduo::copyable {
	explicit Entry(const WeakTcpConnectionPtr& weakConn) : weakConn_(weakConn) {
	}

	~Entry() {
		muduo::net::TcpConnectionPtr conn = weakConn_.lock();
		if (conn)
		{
			conn->shutdown();
		}
	}

	WeakTcpConnectionPtr weakConn_;
	};
	typedef std::shared_ptr<Entry> EntryPtr;
	typedef std::weak_ptr<Entry> WeakEntryPtr;
	typedef std::unordered_set<EntryPtr> Bucket;
	typedef boost::circular_buffer<Bucket> WeakConnectionList;

	TcpServer _server; // #1
	EventLoop* _loop;  // #2 epoll
	WeakConnectionList _connectionBuckets;
	int _numConnected; // should be atomic_int
	const int _kMaxConnections;
};

#endif // !__JUNGLE_P2P_SERVER_H__