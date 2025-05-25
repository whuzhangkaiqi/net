#include "EPollPoller.hpp"
#include "Logger.hpp"
#include "Channel.hpp"

#include <errno.h>
#include <strings.h>
#include <unistd.h>

const int kNew = -1; // 一个channel尚未添加，对应的是channel中的一个成员index_ = -1
const int kAdded = 1;// 一个channel已经添加
const int kDeleted = 2;// 一个channel已经删除

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("EPOLL Create Error: %d", errno);
    }
}
EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}
Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_INFO("function=%s => fd total count: %lu \n", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now = Timestamp::now();

    if (numEvents > 0)
    {
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == static_cast<int>(events_.size()))
        {
            events_.resize(events_.size() * 2);
        }
        else if (numEvents == 0)
        {
            LOG_INFO("function=%s timeout! \n", __FUNCTION__);
        }
        else
        {
            if (saveErrno != EINTR)
            {
                errno = saveErrno;
                LOG_ERROR("EPollPoller::poll error!");
            }
        }
    }
    
    return now;
}

void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("function=%s, fd=%d, events=%d, index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);
    if (index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            channels_[fd] = channel;
        }
        else // index == kDelete
        {
            
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else 
    {
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else 
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    int index = channel->index();
    LOG_INFO("function=%s, fd=%d \n", __FUNCTION__, fd);
    size_t n = channels_.erase(fd);
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    // memset(&event, 0, sizeof event);
    bzero(&event, sizeof event);
    int fd = channel->fd();
    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("EPOLL_CTL delete error: %d \n", errno);
        }
        else
        {
            LOG_FATAL("EPOLL_CTL add/ modified error: %d \n", errno);
        }
    }
}