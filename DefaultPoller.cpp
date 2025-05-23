#include "Poller.hpp"
#include "EPollPoller.hpp"

#include <stdlib.h>
Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; //此处生成Poll的实例
    }
    else
    {
        return new EPollPoller(loop); //此处生成EPoll的实例
    }
}