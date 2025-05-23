#pragma once
#include "Poller.hpp"
#include "Timestamp.hpp"

#include <vector>
#include <sys/epoll.h>

class EventLoop;
class Channel;

/**
 * Epoll的使用
 * epoll_create，创建epoll
 * epoll_ctl，添加感兴趣的事件 add/ modify/ del
 * epoll_wait，监听
 */
class EPollPoller : public Poller
{
    public:
        EPollPoller(EventLoop *loop);
        ~EPollPoller() override;

        Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
        void updateChannel(Channel *channel) override;
        void removeChannel(Channel *channel) override;

    protected:
    private:
        static const int kInitEventListSize = 16;

        // 填写活跃的链接
        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
        // 更新活跃的通道，调用epoll_ctl
        void update(int operation, Channel *channel);

        using EventList = std::vector<epoll_event>;
        int epollfd_;
        EventList events_;
};