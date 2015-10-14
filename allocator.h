#include <stdexcept>
#include <string>
#include <list>
#include <iostream>
#include <cstdio>
#include <vector>
#include <array>
#include <stack>

typedef unsigned int uint;

using namespace std;

//AllocErrors[
enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;
    
public:
    AllocError(AllocErrorType _type, std::string message):
        runtime_error(message),type(_type)
    {}
    
    AllocErrorType getType() const { return type; }
};
//]AllocErrors

class Allocator;

//Ptr[
class Ptr {
    void *ptr;
    size_t size;
public:
    Ptr(void *p = nullptr, size_t s = 0): ptr(p), size(s){}
    ~Ptr() { ptr = nullptr; size = 0;}

    void *getPtr() const { return ptr; };
    void setPtr(void *addr) { ptr = addr; }
    size_t getSize() const { return size; }
    void setSize(size_t n) { size = n; };
    
    static Allocator *allocator;
    static bool compare(const Ptr& first, const Ptr& second)
        { return (first.getPtr() < second.getPtr()); }
};
//]Ptr

static array<Ptr, 1024> ptrs;

//Pointer[
class Pointer{
    size_t index;
public:
    Pointer( size_t i = -1): index(i) {}
    
    void *get() { return ptrs[index].getPtr(); }
    void setIndex( size_t i) { index = i; }
    size_t getIndex() { return index; }
};
//]Pointer
    
//Allocator[
class Allocator{
    size_t array_size = 0;
    stack<size_t> free_index;
    
    struct FreeBlock
    {
        void *address;
        size_t size;
        FreeBlock (void *adr = NULL, size_t s = 0): address(adr), size(s){}
    };
    void *pool;
    void *next;
    size_t remain;
    size_t pool_size;
    list<FreeBlock> blocks;
    
public:
    Allocator(void *base = NULL, size_t size = 0): pool(base), next(base), remain(size), pool_size(size)
    {}
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);
    void defrag();
    std::string dump() { return "Error"; }
    
private:
    vector<Ptr> copy();
    int setNextBlock(size_t size);
    void getFreeBlocks();
};
//]Allocator
