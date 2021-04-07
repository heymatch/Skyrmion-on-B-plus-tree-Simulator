#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <string>
#include <ostream>

static uint64_t BPTreeId = 0;
static uint64_t UnitId = 0;
static uint64_t NodeId = 0;

class Counter{
    friend std::ostream& operator<<(std::ostream &out, Counter right);
public:
    Counter(std::string name = "counter"){
        _name = name;
        _times = 0;
    }

    void count(uint64_t times){
        _times += times;
    }

    uint64_t get() const{
        return _times;
    }

    std::string name() const{
        return _name;
    }

private:
    std::string _name;
    uint64_t _times;
};

std::ostream& operator<<(std::ostream &out, Counter right){
    out << "<Counter> " << right.name() << ": " << right.get();

    return out;
}

#endif