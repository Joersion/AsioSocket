# 基于c++ + boost/boost.beast的跨平台异步网络库

## 目录

- [说明](#说明)
- [安装](#安装)
- [简单示例](#简单示例)
- [关于](#关于)

## 说明

这是一个基于boost/boost.beast库，采用c++17开发的，支持跨平台的异步网络库，具体实现功能：

* tcp 异步客户端/服务端：基于boost::asio，抽象了读/写/连接/关闭/定时器功能，服务端抽象tcp连接管理，客户端抽象断线重连。用户层需自行封包、解包以及数据处理
* http 异步客户端/服务端：基于boost.beast，抽象了http应用层，用户仅需关心消息递达时的数据处理，和错误处理

构建工具：
* 建议采用 cmake 3.10 以上，本人采用 3.28.3

编译器：
* 建议采用 GCC 7.1 以上，本人采用 13.2

boost库：
* 本人采用 1.81.0，其他版本需支持boost::asio::io_context，早期该类被替换成boost::asio::io_server

boost.beast库：
* 本人采用 1.75.0

openSSL库（boost.beast依赖）：
* 本人采用 1.1.1

## 安装

本地编译(本人采用静态库)：

```boost：
wget https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.gz
tar -xzf boost_1_81_0.tar.gz
cd boost_1_81_0
./bootstrap.sh --with-toolset=gcc
./b2 link=static cxxflags="-fPIC" install --prefix=../x86
```

```openSSL：
wget https://www.openssl.org/source/openssl-1.1.1t.tar.gz
tar -xzvf openssl-1.1.1t.tar.gz
./Configure linux-x86_64 --prefix=/home/joersion/work/openSSL/x86
make
make install
```


交叉编译(以aarch64为例)：

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

```openSSL：
wget https://www.openssl.org/source/openssl-1.1.1t.tar.gz
tar -xzvf openssl-1.1.1t.tar.gz
./Configure linux-aarch64 --cross-compile-prefix=aarch64-rockchip1031-linux-gnu- --prefix=/home/joersion/work/openSSL/aarch64
make
make install
```

### boost安装：


### boost.beast安装：

## 简单示例

```tcp client
#include <iostream>

#include "src/ClientConnection.h"
#include "src/Session.h"

std::string gContent = "hello!";

class testClient : public ClientConnection {
public:
    testClient(const std::string &ip, int port, int timeout = 0) : ClientConnection(ip, port, timeout) {
    }
    ~testClient() {
    }

public:
    virtual void onRead(const std::string &ip, int port, const char *buf, size_t len,
                        const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
        std::cout << buf << std::endl;
    }

    virtual void onWrite(const std::string &ip, int port, const std::string &msg, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
    }

    virtual void onConnect(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
        std::cout << "对端已连接,ip: " << ip << ",port:" << port << std::endl;
    }

    virtual void onClose(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
        std::cout << "error info:" << error << std::endl;
    }

    virtual void onTimer(const std::string &ip, int port) override {
        this->send(gContent);
    }

    virtual void onResolver(const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        gContent = argv[1];
    }
    auto t = std::thread([&]() {
        testClient cli("127.0.0.1", 4137, 5000);
        cli.start(5000);
    });
    t.join();
    return 0;
}
```

```tcp server
#include <iostream>

#include "src/ServerConnection.h"
#include "src/Session.h"

class testServer : public ServerConnection {
public:
    testServer(int port, int timeout = 0) : ServerConnection(port, timeout) {
    }
    ~testServer() {
    }

public:
    virtual void onRead(const std::string &ip, int port, const char *buf, size_t len,
                        const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
        std::string str(buf, len);
        std::cout << str << std::endl;
        std::string tmp = "OK!";
        tmp += str;
        send(ip, tmp);
    }

    virtual void onWrite(const std::string &ip, int port, const std::string &msg, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
    }

    virtual void onConnect(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
        std::cout << "对端已连接,ip: " << ip << ",port:" << port << std::endl;
    }

    virtual void onClose(const std::string &ip, int port, const std::string &error) override {
        if (!error.empty()) {
            std::cout << "error info:" << error << std::endl;
            return;
        }
        std::cout << "对端已断开,ip: " << ip << ",port:" << port << std::endl;
    }

    virtual void onTimer(const std::string &ip, int port) override {
    }
};

int main() {
    auto t = std::thread([&]() {
        testServer server(4137);
        server.start();
    });
    t.join();
    return 0;
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



