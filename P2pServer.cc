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
#include "p2p.pb.h"

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
void P2pServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
	// string msg(buf->retrieveAllAsString());
	// LOG_INFO << conn->name() << " echo " << msg.size()
    //        << " bytes at " << receiveTime.toString();
	// conn->send(buf);

	while (buf->readableBytes() >= kHeaderLen) {// kHeaderLen == 8
		// FIXME: use Buffer::peekInt32()
		//const void* data = buf->peek();
		//const int32_t* p = static_cast<const int32_t*>(data);
		//int32_t be32 = *p;// SIGBUS
		//const int32_t len = muduo::net::sockets::networkToHost32(be32);
		//const int32_t cmd = muduo::net::sockets::networkToHost32(be32);

		const int32_t len = buf->peekInt32();
		LOG_INFO << "Received len:" << len;
		buf->retrieveInt32();
		const int32_t cmd = buf->peekInt32();
		LOG_INFO << "Received cmd:" << cmd;
		buf->retrieveInt32();

		if (len > 65536 || len < 0) {
			LOG_ERROR << "Invalid length " << len;
			conn->shutdown();// FIXME: disable reading
			break;
		} else if (buf->readableBytes() >= len) {
			//buf->retrieve(kHeaderLen);// 收缩8字节(指示payload的长度 + 命令字长度)
			// 根据协议命令字判断协议包类型
			if (PING == cmd) {
				LOG_INFO << "收到PING";
				muduo::string message(buf->peek(), len);
				P2pPing ping;
				if (!ping.ParseFromString(message)) {
					LOG_WARN << "P2pPing parse error";
				} else {
					LOG_INFO << "age:" << ping.id();
				}
				//_messageCallback(conn, message, receiveTime);
			} else if (REGISTER_REQ == cmd) {
				LOG_INFO << "收到注册请求";
			} else {

			}
			buf->retrieve(len);
		} else {// 不足协议对象长度
			break;
		}
	}

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
	//dumpConnectionBuckets();
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
