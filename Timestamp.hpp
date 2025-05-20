#pragma once

#include <iostream>
#include <string>
class Timestamp 
{
    public:
        Timestamp();
        explicit Timestamp(Timestamp& other);
        explicit Timestamp(int64_t microSecondsSinceEpoch);
        static Timestamp now();
        std::string toString() const;

    protected:

    private:
        int64_t microSecondsSinceEpoch_;
};