#include <algorithm>
#include <array>
#include <cassert>
#include <ranges>
#include <vector>

#define NDEBUG

template<typename T, typename A = std::allocator<T>>
class RawMemory final {
public:
    [[nodiscard]] explicit constexpr RawMemory(std::size_t size, A allocator = A())
        : memory{nullptr}, sz{size}, alloc{std::move(allocator)} {
        memory = alloc.allocate(sz);
    }

    RawMemory(const RawMemory &) = delete;
    RawMemory &operator=(const RawMemory &) = delete;

    [[nodiscard]] constexpr RawMemory(RawMemory &&other) noexcept
        : memory{other.memory}, sz{other.sz}, alloc{std::move(other.alloc)} {
        other.memory = nullptr;
        other.sz = 0;
    }

    constexpr RawMemory &operator=(RawMemory &&other) noexcept {
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

    [[nodiscard]] constexpr T &at(std::size_t index) {
        if (index >= sz) {
            throw std::out_of_range("Index out of range");
        }

        return memory[index];
    }

    [[nodiscard]] constexpr const T &at(std::size_t index) const {
        if (index >= sz) {
            throw std::out_of_range("Index out of range");
        }

        return memory[index];
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return sz;
    }

    [[nodiscard]] constexpr bool containsWithinMemoryRange(const T &value) const noexcept {
        auto first{memory};
        auto last{memory + sz};
        auto v{&value};
        std::array<T const *, 3> addresses{first, v, last};
        std::ranges::sort(addresses);
        return sz != 0 & addresses[1] == v;
    }

private:
    T *memory;
    std::size_t sz;
    A alloc;
};

template<typename T>
struct Entry final {
    using Inner = union {T value; Entry<T>* next;};
    Inner inner;

    [[nodiscard]] constexpr T* getValue() noexcept {
        return &inner.value;
    }

    [[nodiscard]] constexpr Entry<T>*& getNextPtr() noexcept {
        return inner.next;
    }
};


template<typename T, template<typename> typename A = std::allocator>
class ObjectPool final {
public:
    [[nodiscard]] explicit constexpr ObjectPool(std::size_t size)
        : values{size}, head{size != 0 ? &values.at(0) : nullptr}, free{size} {
        if(values.size() == 0) {
            return;
        }

       for(std::size_t i = 1; i < free; ++i) {
            values.at(i - 1).getNextPtr() = &values.at(i);
       }

       values.at(free - 1).getNextPtr() = nullptr;
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
    [[nodiscard]] constexpr T& claim(Args &&...args) {
        if (freeCount() == 0) {
            throw std::runtime_error("No free objects available.");
        }

        T *value = head->getValue();
        Entry<T>* next = head->getNextPtr();
        new (value) T{std::forward(args)...};
        head = next;
        --free;
        return *value;
    }

    constexpr void reclaim(T &value) {
        if (usedCount() == 0) {
            throw std::logic_error("Cannot reclaim a value if no value is in use.");
        }

        //if (!values.containsWithinMemoryRange(value)) {
        //    throw std::logic_error("Cannot reclaim a value that is allocated outside the pool.");
        //}

        std::destroy_at(&value);
        auto* entry = reinterpret_cast<Entry<T>*>(&value);
        entry->getNextPtr() = head;
        head = entry;
        ++free;
    }

private:
    RawMemory<Entry<T>, A<Entry<T>>> values;
    Entry<T>* head;
    std::size_t free;
};

int main() {
    ObjectPool<int> pool{3};
    assert(pool.size() == 3);
    assert(pool.freeCount() == 3);
    assert(pool.usedCount() == 0);
    int &x = pool.claim();
    assert(x == 0);
    assert(pool.freeCount() == 2);
    assert(pool.usedCount() == 1);
    x = 1;
    int &y{pool.claim()};
    int &z{pool.claim()};
    assert(y == 0);
    pool.reclaim(x);
    x = pool.claim();
    assert(x == 0);
    pool.reclaim(z);
    pool.reclaim(x);
    pool.reclaim(y);

    int &a{pool.claim()};
    ObjectPool<int> other{std::move(pool)};
    other.reclaim(a);
    x = other.claim();
    y = other.claim();

    pool = ObjectPool<int>{1};
    pool = std::move(other);
    pool = ObjectPool<int>{0};
}
