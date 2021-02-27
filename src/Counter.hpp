#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <string>

static unsigned BPTreeId = 0;
static unsigned UnitId = 0;
static unsigned NodeId = 0;

class Counter{
public:
    Counter(std::string name){
        _name = name;
        _times = 0;
    }

    void count(){
        _times++;
    }

    unsigned get(){
        return _times;
    }

    std::string name(){
        return _name;
    }

private:
    std::string _name;
    unsigned _times;
};

#endif