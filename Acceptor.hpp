#pragma once

#include "noncopyable.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
    public:
        using newConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;
        Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
        ~Acceptor();

        void setNewConnectionCallback(const newConnectionCallback &cb) { newConnectionCallback_ = std::move(cb); }
        bool listening() const { return listening_; }
        void listen();

    private:
        void handleRead();
        EventLoop *loop_; // Accept用的就是用户定义的baseLoop，也就是mainLoop
        Socket acceptSocket_;
        Channel acceptChannel_;
        newConnectionCallback newConnectionCallback_;
        bool listening_;
};