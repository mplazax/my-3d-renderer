#pragma once
#include <cstddef>
#include <vector>
#include <unordered_map>
#include <mutex>

class MemoryPool {
public:
    MemoryPool(size_t blockSize = 4096, size_t alignment = 16);
    ~MemoryPool();
    
    void* alloc(size_t size);
    void free(void* ptr);
    
private:
    struct PoolBlock {
        void* memory;
        size_t size;
        size_t used;
        size_t alignment;
    };
    
    std::vector<PoolBlock> m_blocks;
    std::unordered_map<void*, size_t> m_allocations;
    std::mutex m_mutex;
    size_t m_blockSize;
    size_t m_alignment;
    
    PoolBlock createBlock(size_t minSize);
}; 