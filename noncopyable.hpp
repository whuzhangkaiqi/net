#pragma once


/**
 * noncopyable被继承以后，派生类对象可以正常构造和析构，但派生类对象无法进行拷贝构造和赋值操作
 */
class noncopyable
{
    public:
        // 删除掉赋值构造函数和拷贝构造函数，从而使得派生类无法进行拷贝赋值和拷贝构造
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    protected:
        // 默认构造函数和析构函数
        noncopyable() = default;
        ~noncopyable() = default;
    private:
};