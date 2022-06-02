/**
 * @file P2pProtocol.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string>

using namespace std;

#define BYTE1  __attribute__((packed, aligned(1)))  // aligned(1)：1字节对齐
#define BYTE4  __attribute__((packed, aligned(4)))  // aligned(4)：4字节对齐

typedef struct __p2p_proto_header_t {
    uint32_t len;// protocol object total length
    uint32_t cmd;// 命令字
    uint32_t reverse1;
    uint32_t reverse2;
    uint32_t reverse3;
    uint32_t reverse4;
    uint32_t reverse5;
    uint32_t reverse6;
    uint32_t reverse7;
    uint32_t reverse8;
    uint8_t payload[0];
} BYTE4 P2P_PROTO_HEADER;// 40 bytes

class P2pProtocol {
public:
    P2pProtocol();
    P2pProtocol(uint32_t cmd);

private:
    P2P_PROTO_HEADER _header;
};
