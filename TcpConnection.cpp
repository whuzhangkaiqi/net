#include "TcpConnection.hpp"
#include "Callback.hpp"
#include "Logger.hpp"
#include "Channel.hpp"
#include "Socket.hpp"
#include "EventLoop.hpp"
#include <functional>
#include <memory>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <string>
static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s, %s, %d TcpConnectionLoop is null!", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,const std::string& nameArg, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
    :loop_(CheckLoopNotNull(loop)), name_(nameArg),
    state_(kConnecting), reading_(true), socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),localAddr_(localAddr), peerAddr_(peerAddr), highWaterMark_(64*1024*1024)
{
    channel_->SetReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->SetWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->SetErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n", name_.c_str(), channel_->fd(), (int)state_);
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        // 已建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else 
    {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}
void TcpConnection::handleWrite()
{
    if(channel_->isWriting())
    {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    // 唤醒loop_对应的thread线程，执行回调
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite Error");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing \n", channel_->fd());
    }
}
void TcpConnection::handleClose()
{
    LOG_INFO("fd=%d state=%d \n", channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}
void TcpConnection::handleError()
{
    int optval;
    int err = 0;
    socklen_t optlen = sizeof optval;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen)< 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}

void TcpConnection::send(const std::string& buf)
{
    if (state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::sendInLoop(const void *message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    // 之前已经调用了connection的shutdown
    if (state_ == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing \n");
        return;
    }
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            // 数据一次性发送完成，不需要再给channel设置epollout事件了
            if (remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) // SIGPIPE RESET
                {
                    faultError = true;
                }
            }
        }
    }
    if (!faultError && remaining > 0) // 说明一次write没有发送完毕全部数据，剩余数据需要保存到缓冲区中，然后给channel注册epollout事件
    {
        size_t oldLen = outputBuffer_.readableBytes(); // 目前缓冲区待发送长度
        if (oldLen + remaining > highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append((char *)message + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting(); // 这里一定要给channel注册写事件，否则poller不会通知epollout，没办法调用channel处理写事件的回调，也就没办法调用handlewrite
        }
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}
void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting()) // 说明当前outputBuffer中的数据已经全部发送完成
    {
        socket_->shutdownwrite(); // 关闭写端
    }
}

void TcpConnection::connectEstablished() // tcpConnection是给到用户调用的，所以要考虑到可能的链接断开情况
{
    setState(kConnected);
    channel_->tie(shared_from_this()); // 因为channel调用的回调都是tcpConnection绑定的，所以要避免对应的链接被释放掉。
    channel_->enableReading(); // 向Poller注册EPOLLIN事件
    connectionCallback_(shared_from_this());
}
void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove(); // 把Channel从Poller中删除掉
}