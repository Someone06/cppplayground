#include<cassert>
#include<algorithm>
#include<memory_resource>
#include<ranges>
#include<vector>

template<typename T>
concept TriviallyReconstructable = std::is_trivially_default_constructible_v<T>
                                    && std::is_nothrow_default_constructible_v<T>
                                    && std::is_trivially_destructible_v<T>
                                    &&  std::is_nothrow_destructible_v<T>;


template<typename T>
using ObjectPoolAllocationType = std::pair<T, T*>;

template<TriviallyReconstructable T, typename A = std::pmr::polymorphic_allocator<ObjectPoolAllocationType<T>>>
class ObjectPool final {
public:
    [[nodiscard]] explicit constexpr ObjectPool(std::size_t size) : values(size), free{size} {
        if(size == 0) {
            throw std::invalid_argument("An object pool has to contain at least one element.");
        }

        for(auto& [value, ptr] : values) {
            ptr = &value;
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

    [[nodiscard]] constexpr T& claim() {
        if(freeCount() == 0) {
            throw std::runtime_error("No free objects available.");
        }

        return *values.at(--free).second;
    }

    constexpr void reclaim(T& value) noexcept {
       assert(contains(value));
       values.at(free++).second = &value;
       reuseStorage(value);
    }

private:
   constexpr void reuseStorage(T& value) noexcept {
       // Storage reuse destroys an object and constructs a new object in the
       // same place. The C++ standard mandates certain restrictions on classes
       // to which storage use can be applied. Failed to adhere to these
       // restrictions results in UB. The exact rules which state in which cases
       // storage reuse is applicable are pretty tricky, but an easy way out is
       // to mandate non-throwing trivially construction and destruction. This
       // is why this class requires these restrictions on T.
       std::destroy_at(&value);
       new(&value) T();
   }

   [[nodiscard]] constexpr bool contains(T& value) const noexcept {
       return std::ranges::find(values, value, &ObjectPoolAllocationType<T>::first) != std::end(values);
   }

    std::vector<ObjectPoolAllocationType<T>, A> values;
    std::size_t free;
};

int main(){
    ObjectPool<int> pool{2};
    assert(pool.size() == 2);
    assert(pool.freeCount() == 2);
    assert(pool.usedCount() == 0);
    int& x = pool.claim();
    assert(x == 0);
    assert(pool.freeCount() == 1);
    assert(pool.usedCount() == 1);
    x = 1;
    int y = pool.claim();
    assert(y == 0);
    pool.reclaim(x);
    x = pool.claim();
    assert(x == 0);
}
