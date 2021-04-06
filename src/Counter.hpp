#ifndef COUNTER_HPP
#define COUNTER_HPP

#define unsigned uint64_t

#include <string>
#include <ostream>

static unsigned BPTreeId = 0;
static unsigned UnitId = 0;
static unsigned NodeId = 0;

class Counter{
    friend std::ostream& operator<<(std::ostream &out, Counter right);
public:
    Counter(std::string name = "counter"){
        _name = name;
        _times = 0;
    }

    void count(unsigned times){
        _times += times;
    }

    unsigned get() const{
        return _times;
    }

    std::string name() const{
        return _name;
    }

private:
    std::string _name;
    unsigned _times;
};

std::ostream& operator<<(std::ostream &out, Counter right){
    out << "<Counter> " << right.name() << ": " << right.get();

    return out;
}

#endif