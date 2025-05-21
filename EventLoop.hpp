#pragma once

// 事件循环类 主要包含了两个大模块，Channel 和 Poller
// 1. Channel 主要是对文件描述符的封装，主要包含了文件描述符的读写事件的注册和处理
// 2. Poller 主要是对事件的分发和处理，主要包含了epoll的实现
class EventLoop 
{
    public:
    protected:
    private:
};