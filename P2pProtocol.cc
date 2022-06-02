/**
 * @file P2pProtocol.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "P2pProtocol.h"
#include <cstring>

P2pProtocol::P2pProtocol() {
    size_t header_size = sizeof(_header);
    memset(&_header, 0, header_size);
    _header.len = header_size;
}

P2pProtocol::P2pProtocol(uint32_t cmd) {
    size_t header_size = sizeof(_header);
    memset(&_header, 0, header_size);
    _header.len = header_size;
    _header.cmd = cmd;
}
