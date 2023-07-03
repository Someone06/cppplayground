#ifndef CPPPLAYGROUND_SUBJECTIVELOGIC_H
#define CPPPLAYGROUND_SUBJECTIVELOGIC_H

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <ostream>
#include <numeric>
#include <ranges>
#include <span>
#include <stdexcept>

#include "floatingPointHelper.h"
#include "FlexArray.h"

template<plain_floating_point F, std::size_t Size>
requires (Size >= 2)
class MultinomialOpinion final {
public:
    template<std::ranges::input_range R1, std::ranges::input_range R2>
        requires std::ranges::view<R1> && std::ranges::sized_range<R1> && std::same_as<std::remove_cv_t<std::ranges::range_value_t<R1>>, F>
              && std::ranges::view<R2> && std::ranges::sized_range<R2> && std::same_as<std::remove_cv_t<std::ranges::range_value_t<R2>>, F>
    [[nodiscard]] constexpr MultinomialOpinion(const R1 beliefs, F uncertainty, const R2 apriories)
            : beliefsAndApriories{make_FlexArray<F, ArraySize>(std::ranges::size(beliefs) * 2)},
              uncertainty{uncertainty}  {
        if (std::ranges::size(beliefs) != std::ranges::size(apriories))
            throw std::invalid_argument("Number of beliefs and number of apriories must be equal.");

        if constexpr (Size == std::dynamic_extent)
            if(std::ranges::size(beliefs) < 2)
                throw std::invalid_argument("Require a size of at least 2.");

        auto buffer {beliefsAndApriories.getMut()};
        std::ranges::copy(beliefs, std::begin(buffer));
        std::ranges::copy(apriories, std::next(std::begin(buffer), std::ranges::size(beliefs)));

        if (!verifySelf())
            throw std::invalid_argument("Invariant for multinomial opinion does not hold.");
    }

    [[nodiscard]] constexpr std::span<const F, Size>  getBeliefs() const noexcept {
        return getComponent<Component::Beliefs>();
    }

    [[nodiscard]] constexpr std::span<const F, Size>  getApriories() const noexcept {
        return getComponent<Component::Apriories>();
    }

    [[nodiscard]] constexpr F getUncertainty() const noexcept {
        return uncertainty;
    }

    [[nodiscard]] inline constexpr bool is_dynamic_sized() const noexcept {
        return Size == std::dynamic_extent;
    }

    [[nodiscard]] inline constexpr std::size_t size() const noexcept {
        if constexpr (Size == std::dynamic_extent) {
            return beliefsAndApriories.get().size() / 2;
        } else {
            return Size;
        }
    }

private:
    enum class Component {Beliefs, Apriories};

    template<Component c>
    [[nodiscard]] inline constexpr std::span<const F, Size> getComponent() const noexcept {
        if constexpr (Size == std::dynamic_extent) {
            std::span<const F, std::dynamic_extent> s {beliefsAndApriories.get()};
            std::size_t sz {s.size() / 2};
            std::size_t begin = c == Component::Beliefs ? 0 : sz;
            return s.subspan(begin, sz);
        } else {
            std::span<const F, 2 * Size> s{beliefsAndApriories.get()};
            std::size_t begin = c == Component::Beliefs ? 0 : Size;
            std::span<const F, Size> result{s.subspan(begin, Size)};
            return result;
        }
    }

    [[nodiscard]] constexpr bool verifySelf() const noexcept {
        bool in_range{is_between_zero_and_one_inclusive(uncertainty)
                && std::ranges::all_of(beliefsAndApriories.get(),
                                       is_between_zero_and_one_inclusive<F>)};
        if(!in_range)
            return false;

        auto beliefs{getBeliefs()};
        F belief_sum{std::accumulate(std::begin(beliefs), std::end(beliefs), 0.0)};
        if(!is_approx_one(belief_sum + uncertainty))
            return false;

        auto apriories{getApriories()};
        F apriories_sum{std::accumulate(std::begin(apriories), std::end(apriories), 0.0)};
        if(!is_approx_one(apriories_sum))
            return false;

        return true;
    }

    static inline constexpr std::size_t ArraySize
            = Size == std::dynamic_extent ? std::dynamic_extent : 2 * Size;

    FlexArray<F, ArraySize> beliefsAndApriories;
    F uncertainty;
};

template<plain_floating_point F, std::size_t Size>
constexpr std::ostream& operator<<(std::ostream &os, const MultinomialOpinion<F, Size>& opinion) {

    auto printSpan {
            [&os](std::span<F, Size> s){
                auto printElement {[&os](F x) { os << x << ", "; }};
                os << "[";
                std::for_each_n(std::begin(s), s.size() - 1, printElement);
                return (os << s.last() << "]");
            }
    };

    return (os << "MultinomialOpinion{size: " << opinion.size()
               << ", isDynamicSized: " << opinion.is_dynamic_sized()
               << ", beliefs: " << printSpan(opinion.getBeliefs())
               << ", uncertainty: " << opinion.getUncertainty()
               << ", apriories: " << printSpan(opinion.getApriories())
               << "}");
}

#endif
