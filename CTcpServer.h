/**
 * @file CTcpServer.h
 * @author maileizhi (359489892@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __JUNGLE_TCP_SERVER_H__
#define __JUNGLE_TCP_SERVER_H__

typedef enum __tcp_server_type {
	TCP_SERVER_TYPE_NORMAL = 0,
	TCP_SERVER_TYPE_SELECT = 1,
	TCP_SERVER_TYPE_POLL = 2,
	TCP_SERVER_TYPE_EPOLL = 3
} TcpServerType;

class CTcpServer {
public:
	CTcpServer();
	~CTcpServer();

	bool init();
	void run(int type);
	void stop();

private:

private:
	int _lsock;// 监听socket

	bool _loop;
};

#endif // !__JUNGLE_TCP_SERVER_H__
