#pragma once
#include "noncopyable.hpp"
#include "InetAddress.hpp"
#include "Callback.hpp"
#include "Buffer.hpp"
#include "Timestamp.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <atomic>
class Channel;
class Socket;
class EventLoop;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
    public :
        TcpConnection(EventLoop* loop,const std::string& nameArg, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr);
        ~TcpConnection();

        EventLoop *getLoop() const { return loop_; }
        const std::string &getName() const { return name_; }
        const InetAddress &localAddress() const { return localAddr_; }
        const InetAddress &peerAddress() const { return peerAddr_; }

        bool connected() const { return state_ == kConnected; }
        bool disconnected() const { return state_ == kDisconnected; }

        void send(const std::string& buf);
        // 关闭当前连接
        void shutdown();

        void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
        void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
        void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
        void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) { highWaterMarkCallback_ = cb; }
        void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }


        void connectEstablished();
        void connectDestroyed();
        

    private:
        enum stateE
        {
            kDisconnected, kConnecting, kConnected, kDisconnecting
        };
        void setState(stateE state) { state_ = state; }
        void handleRead(Timestamp receiveTime);
        void handleWrite();
        void handleClose();
        void handleError();

        void sendInLoop(const void *message, size_t len);
        void shutdownInLoop();

        EventLoop *loop_; // 这里不一定是baseLoop，因为TcpConnection都是在subloop中管理的（多线程情况下）
        const std::string name_;
        std::atomic_int state_;
        bool reading_;

        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;

        const InetAddress localAddr_;
        const InetAddress peerAddr_;
        
        ConnectionCallback connectionCallback_; // 有新连接时的回调
        MessageCallback messageCallback_; // 有读写消息的回调
        WriteCompleteCallback writeCompleteCallback_; // 消息发送完成后的回调
        CloseCallback closeCallback_;
        HighWaterMarkCallback highWaterMarkCallback_;
        size_t highWaterMark_;
        Buffer inputBuffer_;
        Buffer outputBuffer_;
};