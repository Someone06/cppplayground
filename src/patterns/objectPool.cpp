#include<array>
#include<cassert>
#include<algorithm>
#include<ranges>
#include<vector>

template<typename T, typename A = std::allocator<T>>
class RawMemory final {
public:
    [[nodiscard]] explicit constexpr RawMemory(std::size_t size, A allocator = A()) : memory{nullptr}, sz {size}, alloc{std::move(allocator)} {
        memory = alloc.allocate(sz);
    }

    RawMemory(const RawMemory<T, A>&) = delete;
    RawMemory& operator=(const RawMemory<T, A>&) = delete;

    constexpr RawMemory(RawMemory<T, A>&& other) noexcept
        : memory {other.memory}, sz{other.sz}, alloc{std::move(other.alloc)} {
        other.memory = nullptr;
        other.sz = 0;
    }

    constexpr RawMemory& operator=(RawMemory<T, A>&& other) noexcept {
        this->memory = other.memory;
        this->sz = other.sz;
        this->alloc = std::move(other.alloc);
        other.memory = nullptr;
        other.sz = 0;
        return *this;
    }

    constexpr ~RawMemory() noexcept {
        alloc.deallocate(memory, sz);
    }

    [[nodiscard]] constexpr const std::span<T> get() const noexcept {
        return std::span(memory, sz);
    }

    [[nodiscard]] constexpr std::span<T> getMut() noexcept {
        return std::span(memory, sz);
    }

    [[nodiscard]] constexpr T& at(std::size_t index) {
        if(index >= sz) {
            throw std::out_of_range("Index out of range");
        }

        return memory[index];
    }

    [[nodiscard]] constexpr const T& at(std::size_t index) const {
        if (index >= sz) {
            throw std::out_of_range("Index out of range");
        }

        return memory[index];
    }

   [[nodiscard]] constexpr std::size_t size() const noexcept {
        return sz;
    }

    private:
    T* memory;
    std::size_t sz;
    A alloc;
};


template<typename T, template <typename > typename A = std::allocator>
class ObjectPool final {
public:
    [[nodiscard]] explicit constexpr ObjectPool(std::size_t size) : values{size}, freeList{size}, free{size} {
        for(auto [value, ptr] : std::views::zip(values.get(), freeList.getMut())) {
            ptr = &value;
        }
    }

    ObjectPool(const ObjectPool<T>&) = delete;
    ObjectPool& operator=(const ObjectPool<T>&) = delete;

    constexpr ObjectPool(ObjectPool<T>&& other) noexcept
        : values{std::move(other.values)}, freeList{std::move(other.freeList)}, free{other.free} {
         other.free = other.size();
    };

   constexpr ObjectPool& operator=(ObjectPool<T>&& other) noexcept {
         this->values = std::move(other.values);
         this->freeList = std::move(other.freeList);
         this->free = other.free;
         other.free = other.size();
        return *this;
    }

    constexpr ~ObjectPool() noexcept {
        if(free == size()) {
            // If no objects are claimed, then there are no objects to destroy.
            return;
        }

       auto freeValues {freeList.getMut().subspan(0, free)};
       auto remainder {freeList.getMut().subspan(free)};
       std::ranges::sort(freeValues);

       auto getAddress {[](auto& val) {return &val;}};
       std::ranges::set_difference(values.get() | std::views::transform(getAddress), freeValues, std::ranges::begin(remainder));
       for(T const * ptr : remainder) {
            std::destroy_at(ptr);
       }
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return values.size();
    }

    [[nodiscard]] constexpr std::size_t freeCount() const noexcept {
        return free;
    }

    [[nodiscard]] constexpr std::size_t usedCount() const noexcept {
        return size() - freeCount();
    }

    template<typename... Args>
    [[nodiscard]] constexpr T& claim(Args&&... args) {
        if(freeCount() == 0) {
            throw std::runtime_error("No free objects available.");
        }

        T* value {freeList.at(free - 1)};
        new(value) T{std::forward(args)...};
        --free;
        return *value;
    }

    constexpr void reclaim(T& value) {
       if(usedCount() == 0) {
           throw std::logic_error("Cannot reclaim a value if no value is in use.");
       }

       if(!contains(value)) {
           throw std::logic_error("Cannot reclaim a value that is allocated outside the pool.");
       }

       std::destroy_at(&value);
       freeList.at(free) = &value;
       ++free;
    }

private:
   [[nodiscard]] constexpr bool contains(const T& value) const noexcept {
       // Check whether the address of value is range of addresses spanned by
       // the raw memory.
       auto span {values.get()};
       auto first {&*span.begin()};
       auto last {&*span.end()};
       auto v {&value};
       std::array<T const*, 3> addresses {first, v, last};
       std::ranges::sort(addresses);
       return addresses[1] == v;
   }

    RawMemory<T, A<T>> values;
    RawMemory<T*, A<T*>> freeList;
    std::size_t free;
};

int main(){
    ObjectPool<int> pool{3};
    assert(pool.size() == 3);
    assert(pool.freeCount() == 3);
    assert(pool.usedCount() == 0);
    int& x = pool.claim();
    assert(x == 0);
    assert(pool.freeCount() == 2);
    assert(pool.usedCount() == 1);
    x = 1;
    int& y {pool.claim()};
    int& z {pool.claim()};
    assert(y == 0);
    pool.reclaim(x);
    x = pool.claim();
    assert(x == 0);
    pool.reclaim(z);
    pool.reclaim(x);
    pool.reclaim(y);

    int& a {pool.claim()};
    ObjectPool<int> other{std::move(pool)};
    other.reclaim(a);

    pool = std::move(other);
}
