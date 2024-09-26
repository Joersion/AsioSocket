#pragma once

#include <map>

#include "Session.h"

class ServerSocket : public Connection {
public:
    ServerSocket(int port, int timeout = 0);
    ~ServerSocket() {
    }
    void start();
    void send(const std::string& ip, const char* msg, int len);

protected:
    virtual void onRead(const char* buf, size_t len) = 0;
    virtual void onWrite(std::string& msg) = 0;
    virtual void onConnect() = 0;
    virtual void onClose(const std::string& error) = 0;
    virtual void onTimer() = 0;

private:
    void accept();
    void acceptHandle(std::shared_ptr<Session> session, const boost::system::error_code& error);
    void addSession(std::shared_ptr<Session> session);
    void delSession(const std::string& ip);

private:
    boost::asio::io_context ioContext_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::map<std::string, std::shared_ptr<Session>> sessions_;
    std::mutex mutex_;
    int timeout_;
};
