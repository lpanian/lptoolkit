#pragma once
#ifndef INCLUDED_tests_network_testmsg_HH
#define INCLUDED_tests_network_testmsg_HH

#include <iostream>
#include <type_traits>

enum MsgType : uint16_t {
    MSG_Echo = 0,
};

struct EchoMessage {
    char m_buf[256];
};

template<class T>
inline const T *Cast(const void* data, uint32_t len) {
    if(len >= sizeof(T)) {
        return reinterpret_cast<const T*>(data);
    } else {
        std::cerr << "Invalid cast" << std::endl;
        return nullptr;
    }
}

#endif

