/**
 * @file CTcpServer.cc
 * @author maileizhi (359489892@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#define HOST "10.0.12.3"// 对应服务器的公网IP 119.91.23.80
#define PORT 9527
#define BACKLOG 10
#define MAX_ACCEPT 1024
#define MAX_EVENTS 128

#define ARRAY_SIZE(x)    (sizeof(x)/sizeof(x[0]))

#include "CTcpServer.h"

#include <cstring>
#include <iostream>
using namespace std;

#include <arpa/inet.h>
#include <error.h>
#include <fcntl.h>// non-blocking
#include <netinet/in.h>// address
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>// perror printf
#include <unistd.h>

CTcpServer::CTcpServer() : _lsock(-1) {
}

CTcpServer::~CTcpServer() {
}

bool CTcpServer::init() {
	int ret;

	if (0 < _lsock) {
		perror("listen socket already created.\n");
		return false;
	}

	_lsock = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > _lsock) {
		perror("listen socket create failed.\n");
		return false;
	}

	printf("server listen socket=%d\n", _lsock);

	do {

		if (!set_address_reuse(_lsock)) {
			perror("setsockopt set address reuse()");
			break;
		}

		struct sockaddr_in localaddr;
		bzero(&localaddr, sizeof(localaddr));

		// ret = inet_pton(AF_INET, HOST, &localaddr.sin_addr);
		// if (1 != ret) {
		// 	printf("inet_pton fail ret=%d\n", ret);
		// 	break;
		// }
		localaddr.sin_addr.s_addr = inet_addr(HOST);
		localaddr.sin_family = AF_INET;
		localaddr.sin_port = htons(PORT);// 把端口转化为网络字节序，即大端模式

		ret = bind(_lsock, (struct sockaddr *)&localaddr, sizeof(localaddr));
		if (0 > ret) {
			perror("socket bind server address fail.\n");
			break;
		}

		ret = listen(_lsock, BACKLOG);
		if (0 > ret) {
			perror("socket listen failed.\n");
			break;
		}

		cout << "local host:" << inet_ntoa(localaddr.sin_addr) << endl;

		return true;

	} while (false);

	close(_lsock);
	_lsock = -1;

	return false;
}

void CTcpServer::run(int type) {

	int ret;

	fd_set readfdset;// 读事件集合，包括监听socket和客户端连接上来的socket
	int maxfd;// fd_set中socket的最大值
	// char IP[32];
    // int addrLen = sizeof(struct sockaddr);
    // int client_fd[MAX_ACCEPT];
    // struct sockaddr_in client_addr[MAX_ACCEPT];

	struct pollfd fds[MAX_ACCEPT];
	int max;

	int epfd;
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];// 存放事件的数组

	switch (type) {
		case TCP_SERVER_TYPE_SELECT: {
			FD_ZERO(&readfdset);// 初始化fd集合
			FD_SET(_lsock, &readfdset);// 把监听socket加入到集合
			// for(int i=0; i<MAX_ACCEPT; i++){
			// 	client_fd[i] = -1;
			// 	memset(&client_addr[i], 0x0, sizeof(struct sockaddr_in));
			// }
			maxfd = _lsock;
			break;
		}
		case TCP_SERVER_TYPE_POLL: {
			// 初始化fd数组
			for (int i=0; i<ARRAY_SIZE(fds); i++) {
				fds[i].fd = -1;
			}
			fds[0].fd = _lsock;
			fds[0].events = POLLIN;// 有数据可读事件，包括新客户端连接，客户端socket有数据可读和客户端断开三种情况
			break;
		}
		case TCP_SERVER_TYPE_EPOLL: {
			epfd = epoll_create(1);
			ev.data.fd = _lsock;
			ev.events = EPOLLIN | EPOLLET;
			ret = epoll_ctl(epfd, EPOLL_CTL_ADD, _lsock, &ev);
			if (0 > ret) {
				printf("epoll_ctl add listen fd failed.\n");
				return;
			}
			break;
		}
		default: {
			printf("unkonwn tcp server type%d.\n", type);
			return;
		}
	}
	
	_loop = true;
	while (_loop) {
		if (TCP_SERVER_TYPE_SELECT == type) {

			fd_set tmpfdset = readfdset;

			// 调用select函数时，会改变socket fd集合的内容，所以要把socket fd集合保存下来，传一个临时的给select。
			ret = select(maxfd + 1, &tmpfdset, nullptr, nullptr, nullptr);
			//printf("select infds=%d.\n", ret);
			if (0 > ret) {
				_loop = false;
				printf("select() failed.\n");
				perror("select()");
				break;
			}

			if (0 == ret) {
				printf("select() timeout.\n");
				continue;
			}

			// 处理事件
			for (int eventfd=0; eventfd<(maxfd + 1); eventfd++) {
				if (0 >= FD_ISSET(eventfd, &tmpfdset)) {
					continue;
				}

				if (_lsock == eventfd) {
					// 新的客户端连接
					struct sockaddr_in clientaddr;
					socklen_t len = sizeof(clientaddr);
					int clientsock = accept(_lsock, (struct sockaddr *)&clientaddr, &len);
					if (0 > clientsock) {
						printf("accept() failed.\n");
						continue;
					}

					printf("client(sock=%d) connected ok.\n", clientsock);

					FD_SET(clientsock, &readfdset);// 新的客户端socket加入select集合

					if (clientsock > maxfd) {
						maxfd = clientsock;
					}
				} else {
					// 客户端有数据过来或客户端的socket连接被断开
					char buffer[1024];
					memset(buffer, 0, sizeof(buffer));

					ssize_t readbytes = read(eventfd, buffer, sizeof(buffer));
					if (readbytes <= 0) {
						printf("client(eventfd=%d) disconnected.\n", eventfd);

						close(eventfd);

						FD_CLR(eventfd, &readfdset);// 从集合移除对应的客户端fd socket

						// 重新计算maxfd的值，注意，只有当maxfd==eventfd时才需要计算
						if (eventfd == maxfd) {
							for (int fd=maxfd; fd>0; fd--) {
								if (FD_ISSET(fd, &readfdset)) {
									maxfd = fd;
									break;
								}
							}
							printf("maxfd=%d\n", maxfd);
						}
					} else {
						printf("recv:%s\n", buffer);
						//LOG();
						printf("[%s %s] %s: %s: %d\n", __DATE__, __TIME__, __FILE__, __func__, __LINE__);
					}
				}
			}
			
		} else if (TCP_SERVER_TYPE_POLL == type) {
			/*
			 * poll遍历结构体数组，个数为max+1，永不超时
			 * 返回结构体中 revents 域不为0的文件描述符个数；
			 * 返回0：说明在超时前没有任何事件发生；
			 * 返回-1：说明失败
			 */
			if ((ret = poll(fds, max+1, -1)) < 0)
			{
				printf("poll failure: %s\n", strerror(errno));
				break;
			}
			else if (ret == 0)
			{
				printf("poll timeout\n");
				continue;
			}

			/* 有消息来了 */
			/* 判断是不是listen_fd的消息 */
			if (fds[0].revents & POLLIN)
			{
				/*
				* accept()
				* 接受来自客户端的连接请求
				* 返回一个client_fd与客户通信
				*/
				int client_fd = accept(_lsock, (struct sockaddr *)NULL, NULL);
				if (client_fd < 0)
				{
					printf("accept new client failure: %s\n", strerror(errno));
					continue;
				}
				
				/*
				* 在把client_fd放到数组中的空位中
				* （元素的值为-1的地方）
				*/
				int found = 0;
				for (int i=0; i<ARRAY_SIZE(fds); i++)
				{
					if (fds[i].fd < 0)
					{
						printf("accept new client[%d] and add it to array\n", client_fd);
						fds[i].fd = client_fd;
						fds[i].events = POLLIN;
						found = 1;
						/* 更新结构体数组中的当前最大下标 */
						max = i>max ? i : max;
						break;
					}
				}
				
				/*
				* 如果没找到空位，表示数组满了
				* 不接收这个新客户端，关掉client_fd
				*/
				if (!found)
				{
					printf("accept new client[%d] but full, so refuse\n", client_fd);
					close(client_fd);
				}
			} else {/* 来自已连接客户端的消息 */
				for (int i=0; i<ARRAY_SIZE(fds); i++) {
					char buf[1024];
					/* 判断fd是否有效，并且查看当前fd实际发生的事件是不是POLLIN */
					if (fds[i].fd<0 || !(fds[i].revents & POLLIN))
						continue;
					
					/* 清空buf，以便存放读取的数据 */
					memset(buf, 0, sizeof(buf));
					if ((ret = read(fds[i].fd, buf, sizeof(buf))) <= 0)
					{
						printf("client[%d] read failure or get disconnect\n", fds[i].fd);
						close(fds[i].fd);
						fds[i].fd = -1;
						if (i == max)
							max--;
						continue;
					}
					printf("read %d Bytes data from client[%d]: %s\n", ret, fds[i].fd, buf);
					
					// /* 将小写字母转为大写 */
					// for (int j=0; j<ret; j++)
					// {
					// 	if (buf[j] >= 'a' && buf[j] <= 'z')
					// 		buf[j] = toupper(buf[j]);
					// }
					
					// /* 将数据发送到客户端 */
					// if ((ret = write(fds[i].fd, buf, ret)) < 0)
					// {
					// 	printf("write data to client[%d] failure: %s\n", fds[i].fd, strerror(errno));
					// 	close(fds[i].fd);
					// 	fds[i].fd = -1;
					// 	if (i == max)
					// 		max--;
					// 	continue;
					// }
					// printf("write %d Bytes data to client[%d]: %s\n\n", ret, fds[i].fd, buf);
				} /* end of for() */

			} /* cliet_fd message */
			
		} else if (TCP_SERVER_TYPE_EPOLL == type) {
			int n = epoll_wait(epfd, events, MAX_EVENTS, -1);// 返回发生事件的fd数量
			if (0 > n) {
				printf("epoll_wait failed.\n");
				perror("epoll_wait()");
				break;
			} else if (0 == n) {
				printf("epoll_wait timeout.\n");
				continue;
			} else {
				for (int i=0; i<n; i++) {// 只遍历有事件发生的数组
					if ((events[i].events & EPOLLERR) ||
						(events[i].events & EPOLLHUP) ||
						(!(events[i].events & EPOLLIN))) {

						printf("epoll error\n");
						close(events[i].data.fd);

					} else if (events[i].data.fd == _lsock) {

						// 如果这里有事件发生，表示新的客户端连接上来
						struct sockaddr_in client;
						socklen_t len = sizeof(client);
						int clientsock = accept(_lsock, (struct sockaddr *)&client, &len);
						if (0 > clientsock) {
							printf("accept() failed.\n");
							continue;
						}
						// 设置非阻塞
						// 把连接上来的客户端socket句柄加入epoll中
						memset(&ev, 0, sizeof(struct epoll_event));
						ev.data.fd = clientsock;
						ev.events = EPOLLIN | EPOLLET;
						ret = epoll_ctl(epfd, EPOLL_CTL_ADD, clientsock, &ev);

						printf("client(socket=%d) connected ok.\n", clientsock);

					} else {

						char buffer[1024];
						memset(buffer, 0, sizeof(buffer));
						ssize_t readbytes = read(events[i].data.fd, buffer, sizeof(buffer));
						if (0 >= readbytes) {
							printf("client(eventfd=%d) had been disconnected.\n", events[i].data.fd);
							memset(&ev, 0, sizeof(struct epoll_event));
							ev.events = EPOLLIN;
							ev.data.fd = events[i].data.fd;
							ret = epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
							close(events[i].data.fd);
							continue;
						}
						printf("recv(eventfd=%d,size=%ld):%s\n", events[i].data.fd, readbytes, buffer);

					}
				}
				
			}
		} else {
			_loop = false;
			break;
		}
	} /* while(_loop) */
}

bool CTcpServer::set_address_reuse(int fd) {
	/*
	应用协议 : SOL_SOCKET(套接字) IPPROTO_TCP IPPROTO_IP...
	设置项 : SO_REUSEADDR(是否可以重用bind的地址)...
	opt : 一些设置项指的是开关...
	*/
	int opt = 1;
	if (0 > setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt))){
		return false;
	}
	return true;
}

void CTcpServer::set_non_blocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void CTcpServer::stop() {
	_loop = false;
}
