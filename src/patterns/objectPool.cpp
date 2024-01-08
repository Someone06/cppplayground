#include<array>
#include<cassert>
#include<algorithm>
#include<ranges>
#include<vector>

#define NDEBUG

template<typename T, typename A = std::allocator<T>>
class RawMemory final {
public:
    [[nodiscard]] explicit constexpr RawMemory(std::size_t size, A allocator = A()) : memory{nullptr}, sz {size}, alloc{std::move(allocator)} {
        memory = alloc.allocate(sz);
    }

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory&) = delete;

    constexpr RawMemory(RawMemory&& other) noexcept
        : memory {other.memory}, sz{other.sz}, alloc{std::move(other.alloc)} {
        other.memory = nullptr;
        other.sz = 0;
    }

    constexpr RawMemory& operator=(RawMemory&& other) noexcept {
        std::swap(this->memory, other.memory);
        std::swap(this->sz, other.sz);
        std::swap(this->alloc, other.alloc);
        return *this;
    }

    constexpr ~RawMemory() noexcept {
        alloc.deallocate(memory, sz);
    }

    [[nodiscard]] constexpr std::span<const T> get() const noexcept {
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
        for(auto [value, ptr] : std::views::zip(values.getMut(), freeList.getMut())) {
            ptr = &value;
        }
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    constexpr ObjectPool(ObjectPool&& other) noexcept
        : values{std::move(other.values)}, freeList{std::move(other.freeList)}, free{other.free} {
         other.free = other.size();
    };

   constexpr ObjectPool& operator=(ObjectPool&& other) noexcept {
         std::swap(this->values, other.values);
         std::swap(this->freeList, other.freeList);
         std::swap(this->free, other.free);
        return *this;
    }

    constexpr ~ObjectPool() noexcept {
        /*
         * The complexity of this function is O(freeValues*log(freeValues))
         * where freeValues is the number of non-claimed values because of the
         * sorting step in case freeValues > 0. In general, all values should be
         * re-claimed before the pool is destructed, but the time complexity of
         * the destruction should still depend on the number of not re-claimed
         * aka still used values, not on the number of free ones.
         * This can be archived by memorizing all values that are in use, e.g.
         * by using a bitset.
         */

        if(free == size()) {
            // There are no objects to destroy if no objects are in use.
            return;
        }

       auto freeValues {freeList.getMut().subspan(0, free)};
       auto remainder {freeList.getMut().subspan(free)};
       std::ranges::sort(freeValues);

       auto getAddress {[](auto& val) {return std::addressof(val);}};
       std::ranges::set_difference(values.getMut() | std::views::transform(getAddress), freeValues, std::ranges::begin(remainder));
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
    x = other.claim();
    y = other.claim();

    pool = ObjectPool<int>{1};
    pool = std::move(other);
    pool = ObjectPool<int>{0};
}
