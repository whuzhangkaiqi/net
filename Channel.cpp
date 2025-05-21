#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


// 注意，在调用Channel的析构函数前，要确保1 remove函数已执行 2 所有的回调函数都已执行完毕
Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{

}

Channel::~Channel()
{

}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

// 当改变channel表示的fd的event事件时，需要update在poller中更新epoll_ctl
void Channel::update()
{
    // 通过channel所属的Eventloop,调用poller相应方法，注册fd的events
    // loop_->updateChannel(this);
}

void Channel::remove()
{
    // 在Channel所属的eventloop中，删除对应的channel
    // loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp& receiveTime)
{
    std::shared_ptr<void> guard;

    if (tied_)
    {
        guard = tie_.lock(); // 将弱智能指针提升为强智能指针。
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp& receiveTime)
{
    LOG_INFO("Channel handleEvent revents: %d\n", revents_);
    // 基于事件执行回调操作
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    if ((revents_ & (EPOLLERR)))
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }

    if ((revents_ & (EPOLLIN | EPOLLPRI)))
    {
        if (readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if ((revents_ & (EPOLLOUT)))
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}