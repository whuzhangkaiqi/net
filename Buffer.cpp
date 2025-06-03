#include "Buffer.hpp"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
/**
 * 从fd上读取数据，Poller工作在LT模式，如果数据没有读取完毕，poller会一直提示
 * Buffer是有大小限制的，但从fd上的数据大小是未知的
 */
ssize_t Buffer::readFd(int fd, int *saveErrno)
{
    char extrabuf[65536] = {0}; //栈上空间
    struct iovec vec[2];
    const size_t writable = writableBytes(); // Buffer底层缓冲区剩余的可写空间大小
    
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0)
    {
        *saveErrno = errno;
    }
    else if(n <= writable)
    {
        writerIndex_ += n;
    }
    else // extrabuf也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}
ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    } 
    return n;
}