#include "TcpServer.hpp"
#include "Logger.hpp"

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s, %s, %d mainLoop is null!", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
    :loop_(CheckLoopNotNull(loop)), 
    ipPort_(listenAddr.toIpPort()), 
    name_(nameArg), 
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}
TcpServer::~TcpServer()
{

}
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64];
    // sprintf(buf, sizeof buf, );
}
void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{

}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{

}
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}
void TcpServer::start()
{
    // 防止一个tcpServer对象被start多次
    if (started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get())); 
        // loop_->runInLoop(&)
    }
}