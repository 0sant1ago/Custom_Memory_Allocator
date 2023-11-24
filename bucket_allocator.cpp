#include <iostream>
#include <vector>

template<typename T>
class bucket_allocator {
public:
    typedef T value_type;

    bucket_allocator (const size_t threshold_size = 1024):threshold_(threshold_size) {}
    bucket_allocator (const bucket_allocator& other) = delete;
    bucket_allocator operator= (const bucket_allocator& other) = delete;

    ~bucket_allocator () = default;

    T* allocate (const size_t n) {
        if(n > threshold_) {
            buckets_.push_back(bucket(n));
            return buckets_[buckets_.size()-1].alloc(n);///
        }
        else {
            for(size_t i = 0;i < buckets_.size();i++) {
                if(buckets_[i].remaining() >= n) {
                    return buckets_[i].alloc(n);
                }
            }

            buckets_.push_back(bucket(threshold_));
            return buckets_[buckets_.size()-1].alloc(n);
        }
    }

    void deallocate (T* ptr, const size_t n) noexcept  {
        for(size_t i = 0;i < buckets_.size();i++) {
            if(buckets_[i].buffer_ <= ptr && ptr+n <= buckets_[i].buffer_+buckets_[i].sze) {
                buckets_[i].dealloc(n);
                if(buckets_[i].remaining() == 0)
                    buckets_.erase(buckets_.begin()+i);
                break;
            }
        }
    }

    size_t max_size () const noexcept { return threshold_; }

    struct bucket {
        T* buffer_;
        size_t sze, allocations_count, deallocations_count;
        explicit bucket(const size_t n):
            sze(n), allocations_count(0), deallocations_count(0),
            buffer_(reinterpret_cast<T*>(new (std::nothrow) char[sizeof(T)*n])) {}

        T* alloc (const size_t n) {
            if(allocations_count + n > sze)
                throw std::bad_alloc();
            const size_t current_allocations_count = allocations_count;
            allocations_count += n;
            return buffer_ + current_allocations_count;
        }

        void dealloc (const size_t n) noexcept {
            deallocations_count += n;
            if(sze == deallocations_count) {
                delete[] reinterpret_cast<char*>(buffer_);
            }
        }

        size_t remaining() const { return sze-allocations_count;}
    };

private:
    size_t threshold_;
    std::vector<bucket> buckets_;
};
