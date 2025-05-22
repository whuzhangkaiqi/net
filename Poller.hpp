#pragma once

#include "noncopyable.hpp"
#include "Timestamp.hpp"

#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

// 多路事件分发器的核心——IO复用模块，用于事件监听
class Poller : noncopyable
{
    public:
        using ChannelList = std::vector<Channel*>;

        Poller(EventLoop* loop);
        virtual ~Poller() = default;

        // 保留统一的，给所有IO复用的重写接口
        virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0; // 本质上是poll_wait/epoll_wait, 通过多态在子类中继承。
        virtual void updateChannel(Channel *channel) = 0;
        virtual void removeChannel(Channel *channel) = 0;
        // 判断参数channel是否在当前Poller当中
        virtual bool hasChannel(Channel *channel) const;
        // 这个函数要返回一个具体的Poller实例，而考虑到实例化是在Poller的子类中实现的，因此该方法的定义不在Poller.cpp中实现，避免在父类cpp中引用子类hpp
        static Poller *newDefaultPoller(EventLoop *loop); // 面向接口而不是面向实例，EventLoop可以通过该接口获取默认的IO复用具体实现

    protected:
        // Map的key就是sockfd，value就是sockfd所属的Channel；
        using ChannelMap = std::unordered_map<int, Channel*>;
        ChannelMap channels_;
    private:
        EventLoop *ownerLoop_;
};