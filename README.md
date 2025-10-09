# 基于c++ 和 boost的跨平台异步通信库

## 目录

- [核心特性](#核心特性)
- [环境要求](#环境要求)
- [安装指南](#安装指南)
- [简单示例](#简单示例)
- [关于](#关于)

## 核心特性

### 通信协议全栈支持

| 模块 | 协议/功能 | 技术基础 | 核心优势 |
| :--- | :--- | :--- | :--- |
| **TCP通信** | 异步客户端/服务端 | Boost.Asio | 封装连接管理、断线重连、定时器等核心逻辑，用户只需关注业务数据封包/解包 |
| **HTTP处理** | HTTP/1.1 客户端/服务端 | Boost.Beast | 完整抽象HTTP应用层，简化HTTP/HTTPS开发，支持异步请求与响应处理 |
| **CAN总线** | 控制器局域网 | Linux SocketCAN | 提供应用层抽象接口，直接进行CAN ID与数据帧的收发 |
| **串口通信** | UART (RS232/RS485) | Boost.Asio | 内置RS485半双工通信机制与超时处理，保障数据交换的可靠性 |
| **Modbus** | Modbus-TCP/RTU | 基于TcpClient/Uart | 实现工业标准Modbus协议栈，RTU模式完美适配RS485半双工特性 |
| **GPIO控制** | 通用输入/输出 | Linux epoll | 基于事件驱动的GPIO状态监控与中断处理 |

### 使用技术

- **现代C++**: 采用 C++17 标准，充分利用现代C++特性编写高效、安全的代码
- **纯异步架构**: 完全基于事件驱动的异步模型，保障高并发下的卓越性能
- **跨平台设计**: 核心架构支持跨平台，当前完整支持Linux，Windows支持规划中
- **工业级可靠**: 专为工业环境设计，包含连接保活、自动重连、异常恢复等机制
- **简洁API**: 面向接口设计，提供直观的异步回调接口，大幅降低集成复杂度

## 环境要求

### 构建与编译环境

| 组件 | 最低版本 | 推荐版本 | 备注 |
| :--- | :--- | :--- | :--- |
| **CMake** | 3.10 | 3.28.3 | 构建系统 |
| **GCC** | 7.1 | 13.2 | 支持C++17标准 |
| **Boost** | 1.74 | 1.81.0 | 需包含Asio与Beast组件 |
| **OpenSSL** | 1.1.1 | 1.1.1 | HTTPS支持依赖 |

### 平台兼容性

| 平台 | 状态 | 测试环境 |
| :--- | :--- | :--- |
| Linux x86_64 | ✅ 完全支持 | Ubuntu 20.04+, GCC 13.2 |
| Linux ARM64 | ✅ 完全支持 | 交叉编译, GCC 10.3 |
| Windows | 🚧 规划中 | - |

## 安装指南
boost:

* 本地编译(本人采用静态库)：
```boost：
wget https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz
tar -xzf boost_1_81_0.tar.gz
cd boost_1_81_0
./bootstrap.sh --with-toolset=gcc
./b2 link=static cxxflags="-fPIC" install --prefix=../x86
```

* 交叉编译(以aarch64为例)：
```boost：
wget https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz
tar -xzf boost_1_81_0.tar.gz
cd boost_1_81_0
./bootstrap.sh --with-toolset=gcc
vim project-config.jam
if ! gcc in [ feature.values <toolset> ]

{

   using gcc : arm : /home/joersion/tool/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-rockchip1031-linux-gnu-gcc ;

}
./b2 link=static cxxflags="-fPIC" install --prefix=../aarch64
```
## 简单示例（仅部分示例）
* tcp client:
```tcp client
#include <iostream>
#include <thread>

#include "src/Socket.h"
#include "src/TcpClient.h"

std::string gContent = "hello!";

using namespace net::socket;

class testClient : public TcpClient {
public:
    testClient(const std::string &ip, int port, int timeout = 0) : TcpClient(ip, port, timeout) {
    }
    ~testClient() {
    }

public:
    virtual void onRead(const std::string &ip, int port, const char *buf, size_t len, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onRead error:" << error << std::endl;
            return;
        }
        std::cout << "客户端接收数据:" << buf << std::endl;
    }

    virtual void onWrite(const std::string &ip, int port, int len, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onWrite error:" << error << std::endl;
            return;
        }
        std::cout << "客户端发送数据,len:" << len << std::endl;
    }

    virtual void onConnect(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onConnect error:" << error << std::endl;
            return;
        }
        std::cout << "已连接上服务器,ip: " << ip << ",port:" << port << std::endl;
    }

    virtual void onClose(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onClose error:" << error << std::endl;
            return;
        }
        std::cout << "连接已断开,ip" << ip << ",port:" << port << std::endl;
    }

    virtual void onTimer(const std::string &ip, int port) override {
        std::cout << "客户端发送数据,data:" << gContent << std::endl;
        this->send(gContent);
    }

    virtual void onResolver(const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onResolver error:" << error << std::endl;
            return;
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        gContent = argv[1];
    }
    testClient cli("127.0.0.1", 4137, 2000);
    cli.start(1000);
    while (1) {
        char ch = getchar();
        if (ch == 'q') {
            exit(0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}
```
* tcp server:
```tcp server
#include <iostream>
#include <thread>

#include "src/Socket.h"
#include "src/TcpServer.h"

using namespace net::socket;

class testServer : public TcpServer {
public:
    testServer(int port, int timeout = 0) : TcpServer(port, timeout) {
    }
    ~testServer() {
    }

public:
    virtual void onRead(const std::string &ip, int port, const char *buf, size_t len, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onRead error info:" << error << std::endl;
            return;
        }
        std::cout << "收到来自客户端,Ip:" << ip << ",port:" << port << ",data:" << buf << std::endl;
        std::string str(buf, len);
        std::string tmp = "OK!";
        tmp += str;
        std::cout << "发送数据给客户端,Ip:" << ip << ",port:" << port << ",data:" << tmp << std::endl;
        send(ip, tmp);
    }

    virtual void onWrite(const std::string &ip, int port, int len, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onWrite error info:" << error << std::endl;
            return;
        }
        std::cout << "发送数据长度,Ip:" << ip << ",port:" << port << ",len:" << len << std::endl;
    }

    virtual void onConnect(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onConnect error info:" << error << std::endl;
            return;
        }
        std::cout << "新客户端连接,ip: " << ip << ",port:" << port << std::endl;
    }

    virtual void onClose(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "onClose error info:" << error << std::endl;
            return;
        }
        std::cout << "连接已关闭,ip: " << ip << ",port:" << port << std::endl;
    }

    virtual void onTimer(const std::string &ip, int port) override {
    }
};

int main() {
    testServer server(4137);
    server.start();
    while (1) {
        char ch = getchar();
        if (ch == 'q') {
            exit(0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}
```
* http client:
```
#include <iostream>

#include "src/HttpClient.h"

using namespace net;

#define LINES "----------------"
int main() {
    HttpClient::GET({"www.example.com", 80, "/", "1.0", 30}, [](const std::string& err, const HttpClient::Response& resp) {
        if (err.empty()) {
            std::cout << LINES << "GET" << LINES << std::endl;
            std::cout << "resp.code:" << resp.code << std::endl;
            std::cout << "resp.massage:" << resp.massage << std::endl;
            std::cout << "resp.type:" << resp.type << std::endl;
            std::cout << "resp.version:" << resp.version << std::endl;
            std::cout << "resp.body:" << resp.body << std::endl;
        }
    });
    HttpClient::POST({"jsonplaceholder.typicode.com", 80, "/posts", "1.0", 30}, {R"({"title": "foo", "body": "bar", "userId": 1})", ""},
                     [](const std::string& err, const HttpClient::Response& resp) {
                         if (err.empty()) {
                             std::cout << LINES << "POST" << LINES << std::endl;
                             std::cout << "resp.code:" << resp.code << std::endl;
                             std::cout << "resp.massage:" << resp.massage << std::endl;
                             std::cout << "resp.type:" << resp.type << std::endl;
                             std::cout << "resp.version:" << resp.version << std::endl;
                             std::cout << "resp.body:" << resp.body << std::endl;
                         }
                     });

    getchar();
}
```

## 关于

目前cmake构建仅支持linux，后续可能会cmake新增windows兼容


**************************
- 👋 I’m Joersion (WuJiaXiang)
- 👀 I’m interested in code
- 🌱 learning C++ and python and golang
- 📫 e-mail : 1539694343@qq.com

**************************



