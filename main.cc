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

int main(int argc, const char** argv) {

	CTcpServer server;

	if (!server.init()) {
		return -1;
	}

	//server.run(TCP_SERVER_TYPE_SELECT);
	//server.run(TCP_SERVER_TYPE_POLL);
	server.run(TCP_SERVER_TYPE_EPOLL);

	return 0;
}
