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

P2pServer::P2pServer(EventLoop* loop,
	const InetAddress& listenAddr,
	const string& nameArg,
	int idleSeconds,
	int maxConnections) :
	_server(loop, listenAddr, nameArg, TcpServer::Option::kReusePort),
	_loop(loop),
	_connectionBuckets(idleSeconds),
	_numConnected(0),
    _kMaxConnections(maxConnections)
{
	// 设置服务器端的线程数量 1个I/O线程 5个worker线程
	// 也可以提高获取系统核心数进行设置
	// 使用sysconf()或者get_nprocs()来获取处理器核数
//	_server.setThreadNum(6);

	_server.setConnectionCallback(
		std::bind(&P2pServer::onConnection, this, _1));
	_server.setMessageCallback(
		std::bind(&P2pServer::onMessage, this, _1, _2, _3));
	loop->runEvery(1.0, std::bind(&P2pServer::onTimer, this));
	_connectionBuckets.resize(idleSeconds);
	dumpConnectionBuckets();
}

void P2pServer::start() {
	_server.start();
}

// 专门处理用户的连接创建和断开  epoll listenfd accept
void P2pServer::onConnection(const TcpConnectionPtr& conn) {
	LOG_INFO << "P2pServer - " << conn->peerAddress().toIpPort() << " -> "
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");

	if (conn->connected()) {
		++_numConnected;
		if (_numConnected > _kMaxConnections) {
			conn->shutdown();
			conn->forceCloseWithDelay(3.0);  // > round trip of the whole Internet.
		}

		cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:online" << endl;
		
		conn->setTcpNoDelay(true);
		
		EntryPtr entry(new Entry(conn));
		_connectionBuckets.back().insert(entry);
		dumpConnectionBuckets();
		WeakEntryPtr weakEntry(entry);
		conn->setContext(weakEntry);
	} else {
		// cout << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " state:offline" << endl;
		// conn->shutdown();// close(fd)
		//_loop->quit();
		assert(!conn->getContext().empty());
		WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
		LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();

		--_numConnected;
	}
	LOG_INFO << "numConnected = " << _numConnected;
}

// 专门处理用户的读写事件
// 注:接收到数据的时间信息(UTC) 需要自行+0800才是北京时间
void P2pServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) {
	string msg(buffer->retrieveAllAsString());
	LOG_INFO << conn->name() << " echo " << msg.size()
           << " bytes at " << time.toString();
	conn->send(buffer);

	assert(!conn->getContext().empty());
	WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
	EntryPtr entry(weakEntry.lock());
	if (entry) {
		_connectionBuckets.back().insert(entry);
		dumpConnectionBuckets();
	}
}

void P2pServer::onTimer() {
	_connectionBuckets.push_back(Bucket());
	dumpConnectionBuckets();
}

void P2pServer::dumpConnectionBuckets() const {
	LOG_INFO << "size = " << _connectionBuckets.size();
	int idx = 0;
	for (WeakConnectionList::const_iterator bucketI = _connectionBuckets.begin();
		bucketI != _connectionBuckets.end();
		++bucketI, ++idx)
	{
		const Bucket& bucket = *bucketI;
		printf("[%d] len = %zd : ", idx, bucket.size());
		for (const auto& it : bucket) {
			bool connectionDead = it->weakConn_.expired();
			printf("%p(%ld)%s, ", get_pointer(it), it.use_count(), connectionDead ? " DEAD" : "");
		}
		puts("");
	}
}
