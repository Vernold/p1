#include "allocator.h"

using namespace std;

typedef unsigned int uint;

//PRIVATE METHODS[
vector<Ptr> Allocator::copy()
{
    vector<Ptr> res;
    
    for (uint i = 0; i < array_size; i++){
        if (ptrs[i].getPtr() != nullptr)
            res.push_back(ptrs[i]);
    }
    sort(res.begin(), res.end(), Ptr::compare);
    return res;
}

void Allocator::getFreeBlocks()
{
    size_t size;
    size_t sum = 0;
    
    vector<Ptr> p = copy();
    blocks.clear();
    for (uint i = 0; i < p.size(); i++){
        if (i != p.size() - 1){
            size = (char*)p[i + 1].getPtr() - ((char*)p[i].getPtr() + p[i].getSize());
            sum += size + p[i].getSize();
        }
        else{
            sum += p[i].getSize();
            size = pool_size - sum;
        }
        if (size > 0){
            blocks.push_back(FreeBlock((char*)p[i].getPtr() + p[i].getSize(), size));
        }
    }
}

int Allocator::setNextBlock(size_t size)
{
    for (auto it = blocks.begin(); it != blocks.end(); it++){
        if (it->size >= size){
            pool = it->address;
            next = pool;
            remain = it->size;
            blocks.erase(it);
            return 0;
        }
    }
    return -1;
}
//]PRIVATE METHODS

//ALLOC[
Pointer Allocator::alloc(size_t N)
{
    if (N > remain){
        if (setNextBlock(N) == -1){
            getFreeBlocks();
            if (setNextBlock(N) == -1){
                if (N > remain)
                    throw AllocError(AllocErrorType::NoMemory, dump());
            }
        }
    }
    Ptr ptr = Ptr(next, N);
    remain -= N;
    next = (char*)next + N;
    if (free_index.empty()){
        ptrs[array_size++] = ptr;
        return Pointer(array_size - 1);
    }
    size_t index = free_index.top();
    free_index.pop();
    ptrs[index] = ptr;
    return Pointer(index);
}
//]ALLOC

//FREE[
void Allocator::free(Pointer &p)
{
    void *addr = p.get();
    size_t size = ptrs[p.getIndex()].getSize();
    
    for (uint i = 0; i < size; i++)
        *((char*)addr + i) = 0;
    ptrs[p.getIndex()].~Ptr();
    free_index.push(p.getIndex());
}
//]FREE

//[DEFRAG
void Allocator::defrag()
{
    vector<Ptr> p = copy();
    size_t space, index;
    
    for (uint i = 0; i < p.size() - 1; i++){
        space = (char*)p[i + 1].getPtr() - ((char*)p[i].getPtr() + p[i].getSize());
        if (space > 0){
            for (uint j = 0; j < array_size; j++){
                if (ptrs[j].getPtr() == p[i + 1].getPtr()){
                    index = j;
                }
            }
            for (uint j = 0; j < p[i + 1].getSize(); j++){
                *((char*)p[i + 1].getPtr() - space + j) = *((char*)p[i + 1].getPtr() + j);
                *((char*)p[i + 1].getPtr() + j) = 0;
            }
            p[i + 1].setPtr((char*)p[i + 1].getPtr() - space);
            ptrs[index].setPtr(p[i + 1].getPtr());
        }
    }
}
//]DEFRAG

//REALLOC[
void Allocator::realloc(Pointer &p, size_t N)
{
    if (p.getIndex() == -1){
        p = alloc(N);
        return;
    }
    
    void *ptr = p.get();
    size_t size = ptrs[p.getIndex()].getSize();
    
    if (size > N){
        for (uint i = N; i < size; i++){
            *((char*)ptr + i) = 0;
        }
        ptrs[p.getIndex()].setSize(N);
    }
    
    if (size < N){
        vector<Ptr> ps = copy();
        for (auto it = ps.begin(); it != ps.end(); it++){
            if (it->getPtr() >= (char*)ptr + size && it->getPtr() < (char*)ptr + N){
                char buf[size];
                
                for (uint i = 0; i < size; i++){
                    buf[i] = *((char*)ptr + i);
                }
                free(p);
                p = alloc(N);
                ptr = p.get();
                for (uint i = 0; i < size; i++){
                    *((char*)ptr + i) = buf[i];
                }
                return;
            }
        }
        getFreeBlocks();
        ptrs[p.getIndex()].setSize(N);
        for (auto it = blocks.begin(); it != blocks.end(); it++){
            if (it->address == next && it->address == (char*)ptr + size){
                next =(char*)next +  N - size;
                remain -= N - size;
                blocks.erase(it);
            }
            else if (it->address == next) blocks.erase(it);
            else if (it->address == (char*)ptr + size){
                it->address = (char*)ptr + N;
                it->size -= N - size;
            }
        }
    }
}
//]REALLOC
