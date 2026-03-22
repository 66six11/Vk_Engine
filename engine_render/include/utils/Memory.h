#pragma once

#include "core/Types.h"
#include <set>

namespace render::utils
{

    /**
 * 内存分配器基类
 */
    class Allocator {
        public:
            virtual ~Allocator() = default;

            virtual void* allocate(size_t size, size_t alignment = 16) = 0;
            virtual void  deallocate(void* ptr) = 0;
            virtual void* reallocate(void* ptr, size_t newSize, size_t alignment = 16) = 0;

            // 统计
            virtual size_t getTotalAllocated() const = 0;
            virtual size_t getTotalUsed() const = 0;
    };

    /**
 * 线性分配器
 * 用于帧内临时分配，每帧重置
 */
    class LinearAllocator : public Allocator {
        public:
            explicit LinearAllocator(size_t capacity);
            ~LinearAllocator() override;

            void* allocate(size_t size, size_t alignment = 16) override;
            void  deallocate(void* ptr) override;
            void* reallocate(void* ptr, size_t newSize, size_t alignment = 16) override;

            size_t getTotalAllocated() const override;
            size_t getTotalUsed() const override;

            // 重置（每帧调用）
            void reset();

            // 获取已使用大小
            size_t getUsed() const { return offset; }
            size_t getCapacity() const { return capacity; }

        private:
            std::unique_ptr<u8[]> buffer;
            size_t                capacity;
            size_t                offset = 0;
    };

    /**
 * 池分配器
 * 用于相同大小的对象分配
 */
    template<typename T>
    class PoolAllocator {
        public:
            explicit PoolAllocator(size_t blockSize = 1024);
            ~PoolAllocator();

            T*   allocate();
            void deallocate(T* ptr);

            // 统计
            size_t getTotalAllocated() const;
            size_t getFreeCount() const;

        private:
            struct Block {
                std::unique_ptr<u8[]> memory;
                std::vector<T*>       freeList;
            };

            std::vector<std::unique_ptr<Block>> blocks;
            size_t                              blockSize;
            size_t                              totalAllocated = 0;
            size_t                              freeCount      = 0;

            void allocateNewBlock();
    };

    /**
 * 堆分配器
 * 用于GPU内存管理
 */
    class HeapAllocator {
        public:
            struct Allocation {
                u64 offset;
                u64 size;
                u32 blockIndex;
            };

            HeapAllocator();
            ~HeapAllocator();

            bool initialize(u64 heapSize, u32 alignment = 256);
            void shutdown();

            Allocation allocate(u64 size);
            void       deallocate(const Allocation& allocation);

            // 合并空闲块
            void coalesce();

            // 统计
            u64 getTotalSize() const { return heapSize; }
            u64 getFreeSize() const;
            u64 getUsedSize() const;
            u32 getAllocationCount() const;

        private:
            struct FreeBlock {
                u64 offset;
                u64 size;

                bool operator<(const FreeBlock& other) const {
                    return offset < other.offset;
                }
            };

            u64                          heapSize  = 0;
            u32                          alignment = 256;
            std::set<FreeBlock>          freeBlocks;
            std::unordered_map<u64, u64> allocatedBlocks; // offset -> size
    };

    /**
 * 内存池
 * 用于管理GPU内存堆
 */
    class MemoryPool {
        public:
            MemoryPool();
            ~MemoryPool();

            bool initialize(IRenderDevice* device, MemoryType type, u64 initialSize);
            void shutdown();

            // 分配子分配
            struct Suballocation {
                IHeap* heap;
                u64    offset;
                u64    size;
            };

            Suballocation allocate(u64 size, u32 alignment = 256);
            void          deallocate(const Suballocation& suballocation);

            // 统计
            u64 getTotalSize() const;
            u64 getFreeSize() const;
            u64 getUsedSize() const;

        private:
            IRenderDevice* device = nullptr;
            MemoryType     memoryType;

            struct HeapEntry {
                IHeap*        heap;
                HeapAllocator allocator;
                u64           size;
            };

            std::vector<std::unique_ptr<HeapEntry>> heaps;
            u64                                     heapSizeIncrement = 64 * 1024 * 1024; // 64MB
    };

    /**
 * 智能指针包装
 * 用于GPU资源生命周期管理
 */
    template<typename T, typename Deleter>
    class GpuHandle {
        public:
            GpuHandle() = default;
            explicit GpuHandle(T handle, Deleter deleter)
                : handle(handle), deleter(std::move(deleter)) {}

            ~GpuHandle() {
                reset();
            }

            // 禁用拷贝
            GpuHandle(const GpuHandle&)            = delete;
            GpuHandle& operator=(const GpuHandle&) = delete;

            // 启用移动
            GpuHandle(GpuHandle&& other) noexcept
                : handle(other.handle), deleter(std::move(other.deleter)) {
                other.handle = {};
            }

            GpuHandle& operator=(GpuHandle&& other) noexcept {
                if (this != &other) {
                    reset();
                    handle       = other.handle;
                    deleter      = std::move(other.deleter);
                    other.handle = {};
                }
                return *this;
            }

            T        get() const { return handle; }
            explicit operator bool() const { return handle.get(); }

            void reset() {
                if (handle.get()) {
                    deleter(handle);
                    handle = {};
                }
            }

            T release() {
                T tmp  = handle;
                handle = {};
                return tmp;
            }

        private:
            T       handle;
            Deleter deleter;
    };

    /**
 * 帧资源管理器
 * 用于管理每帧需要释放的资源
 */
    class FrameResourceManager {
        public:
            explicit FrameResourceManager(u32 maxFramesInFlight = 3);
            ~FrameResourceManager();

            // 注册资源释放回调
            using Deleter = std::function<void()>;
            void deferDelete(Deleter deleter);

            // 帧结束
            void nextFrame();

            // 当前帧索引
            u32 getCurrentFrame() const { return currentFrame; }

        private:
            u32                               maxFramesInFlight;
            u32                               currentFrame = 0;
            std::vector<std::vector<Deleter>> pendingDeleters;
    };

    /**
 * 内存对齐辅助函数
 */
    inline size_t alignUp(size_t value, size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    inline u32 alignUp(u32 value, u32 alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    inline void* alignUp(void* ptr, size_t alignment) {
        return reinterpret_cast<void*>(
            alignUp(reinterpret_cast<size_t>(ptr), alignment));
    }

    /**
 * 内存块
 */
    struct MemoryBlock {
        void*  data = nullptr;
        size_t size = 0;

        MemoryBlock() = default;
        MemoryBlock(void* d, size_t s) : data(d), size(s) {}

        bool isValid() const { return data != nullptr && size > 0; }
        u8*  begin() const { return static_cast<u8*>(data); }
        u8*  end() const { return static_cast<u8*>(data) + size; }
    };

}
