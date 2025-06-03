#pragma once

// 对外的服务器编程类
#include "EventLoop.hpp"
#include "Acceptor.hpp"
#include "InetAddress.hpp"
#include "noncopyable.hpp"
#include "EventLoopThreadPool.hpp"
#include "Callback.hpp"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer : noncopyable
{
    public:
        using ThreadInitCallback = std::function<void(EventLoop *)>;
        enum Option
        {
            kNoReusePort,
            kReusePort
        };
        TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option = kNoReusePort);
        ~TcpServer();

        const std::string &ipPort() const { return ipPort_; }
        const std::string &name() const { return name_; }
        EventLoop *getLoop() const { return loop_; }

        void setThreadNum(int numThreads);

        void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }

        std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

        void start();
        void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
        void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
        void setWriteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }


    protected:
    private:
        void newConnection(int sockfd, const InetAddress &peerAddr);
        void removeConnection(const TcpConnectionPtr &conn);
        void removeConnectionInLoop(const TcpConnectionPtr &conn);

        using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

        EventLoop *loop_; // baseLoop
        
        const std::string ipPort_;
        const std::string name_;
        
        std::unique_ptr<Acceptor> acceptor_; // 运行在MainLoop，监听新连接事件
        std::shared_ptr<EventLoopThreadPool> threadPool_; 
        
        ThreadInitCallback threadInitCallback_; // loop线程初始化的回调
        ConnectionCallback connectionCallback_; // 有新连接时的回调
        MessageCallback messageCallback_; // 有读写消息的回调
        WriteCompleteCallback writeCompleteCallback_; // 消息发送完成后的回调
        std::atomic_int started_;

        int nextConnId_;
        ConnectionMap connections_; // 保存所有的连接
};