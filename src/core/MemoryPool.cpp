#include "core/MemoryPool.h"
#include <stdexcept>

#ifdef __APPLE__
#include <stdlib.h>
#endif

MemoryPool::MemoryPool(size_t blockSize, size_t alignment)
    : m_blockSize(blockSize), m_alignment(alignment) {
    // Start with one block
    m_blocks.push_back(createBlock(blockSize));
}

MemoryPool::~MemoryPool() {
    for (auto& block : m_blocks) {
        free(block.memory);
    }
}

void* MemoryPool::alloc(size_t size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Align size
    size_t alignedSize = (size + m_alignment - 1) & ~(m_alignment - 1);
    
    // Find a block with enough space
    for (size_t i = 0; i < m_blocks.size(); ++i) {
        auto& block = m_blocks[i];
        if (block.used + alignedSize <= block.size) {
            void* ptr = static_cast<char*>(block.memory) + block.used;
            block.used += alignedSize;
            m_allocations[ptr] = alignedSize;
            return ptr;
        }
    }
    
    // No block has enough space, create a new one
    size_t newBlockSize = std::max(m_blockSize, alignedSize);
    auto newBlock = createBlock(newBlockSize);
    void* ptr = newBlock.memory;
    newBlock.used = alignedSize;
    m_blocks.push_back(newBlock);
    m_allocations[ptr] = alignedSize;
    return ptr;
}

void MemoryPool::free(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
        m_allocations.erase(it);
        // Note: We don't actually reclaim memory within blocks
        // A more sophisticated implementation would track free regions
    }
}

MemoryPool::PoolBlock MemoryPool::createBlock(size_t minSize) {
    PoolBlock block;
    block.size = minSize;
    block.used = 0;
    block.alignment = m_alignment;
    
    // Use posix_memalign on macOS for aligned allocation
    #ifdef __APPLE__
    if (posix_memalign(&block.memory, m_alignment, minSize) != 0) {
        throw std::runtime_error("Failed to allocate aligned memory");
    }
    #else
    block.memory = aligned_alloc(m_alignment, minSize);
    if (!block.memory) {
        throw std::runtime_error("Failed to allocate aligned memory");
    }
    #endif
    
    return block;
} 