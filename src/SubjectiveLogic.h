#ifndef CPPPLAYGROUND_SUBJECTIVELOGIC_H
#define CPPPLAYGROUND_SUBJECTIVELOGIC_H

#include<algorithm>
#include <concepts>
#include<cstddef>
#include <iostream>
#include <numeric>
#include <ranges>
#include<span>
#include<stdexcept>

#include "rangeHelper.h"
#include "floatingPointHelper.h"
#include "FlexArray.h"

template<plain_floating_point F>
class BinomialOpinion final {
public:
    [[nodiscard]] constexpr BinomialOpinion(F belief, F disbelief, F uncertainty, F apriori)
        : belief{belief}, disbelief{disbelief}, uncertainty{uncertainty}, apriori{apriori} {
        if (!verifySelf())
            throw std::invalid_argument{"Binomial opinion is invalid."};
    }

    [[nodiscard]] constexpr F getBelief() const {
        return belief;
    }

    [[nodiscard]] constexpr F getDisbelief() const {
        return disbelief;
    }

    [[nodiscard]] constexpr F getUncertainty() const {
        return uncertainty;
    }

    [[nodiscard]] constexpr F getApriori() const {
        return apriori;
    }

    constexpr friend std::ostream &operator<<(std::ostream &os, const BinomialOpinion &opinion) {
        return (os << "BinomialOpinion{belief: " << opinion.belief
                   << ", disbelief: " << opinion.disbelief
                   << ", uncertainty: " << opinion.uncertainty
                   << ", apriori: " << opinion.apriori << "}");
    }

private:
    [[nodiscard]] constexpr bool verifySelf() const {
        std::array components{belief, disbelief, uncertainty, apriori};
        auto all_between_zero_and_one{std::ranges::all_of(components, is_between_zero_and_one_inclusive<F>)};
        auto sum{belief + disbelief + uncertainty};
        return all_between_zero_and_one && is_approx_one(sum);
    }

private:
    F belief{};
    F disbelief{};
    F uncertainty{};
    F apriori{};
};



template<plain_floating_point F, std::size_t Size>
requires (Size >= 2)
class MultinomialOpinion final {
public:
    template<std::ranges::input_range R>
        requires std::ranges::view<R> && std::ranges::sized_range<R> && std::same_as<std::ranges::range_value_t<R>, F>
    [[nodiscard]] constexpr MultinomialOpinion(const R beliefs, F uncertainty, const R apriories) : beliefsAndApriories{make_FlexArray<F, Double>(std::ranges::size(beliefs) * 2)}, uncertainty{uncertainty}  {
        if (std::ranges::size(beliefs) != std::ranges::size(apriories))
            throw std::invalid_argument("Number of beliefs and number of apriories must be equal.");

        if (Size == std::dynamic_extent && Size < 2)
            throw std::invalid_argument("Require a size of at least 2.");

        std::array combined{std::move(beliefs), std::move(apriories)};
        auto joined{std::ranges::views::join(std::move(combined))};
        std::ranges::copy(joined, std::begin(beliefsAndApriories.getMut()));

        if (!verifySelf())
            throw std::invalid_argument("Invariant for multinomial opinion does not hold.");
    }

    [[nodiscard]] constexpr std::span<const F, Size>  getBeliefs() const {
        if constexpr (Size == std::dynamic_extent) {
            std::span<const F, std::dynamic_extent> s {beliefsAndApriories.get()};
            std::size_t sz {s.size() / 2};
            return s.subspan(0, sz);
        } else {
            std::span<const F, 2 * Size> s{beliefsAndApriories.get()};
            std::span<const F, Size> result{s.subspan(0, Size)};
            return result;
        }
    }

    [[nodiscard]] constexpr std::span<const F, Size>  getApriories() const {
        if constexpr (Size == std::dynamic_extent) {
            std::span<const F, std::dynamic_extent> s {beliefsAndApriories.get()};
            std::size_t sz {s.size() / 2};
            return s.subspan(sz, sz);
        } else {
            std::span<const F, 2 * Size> s{beliefsAndApriories.get()};
            std::span<const F, Size> result{s.subspan(Size, Size)};
            return result;
        }
    }

    [[nodiscard]] inline constexpr std::size_t size() const {
        if constexpr (Size == std::dynamic_extent) {
            return beliefsAndApriories.get().size() / 2;
        } else {
            return Size;
        }
    }

private:
    [[nodiscard]] constexpr bool verifySelf() const {
        bool in_range{std::ranges::all_of(beliefsAndApriories.get(), is_between_zero_and_one_inclusive<F>)};

        auto beliefs{getBeliefs()};
        F belief_sum{std::accumulate(std::begin(beliefs), std::end(beliefs), 0.0)};

        auto apriories{getApriories()};
        F apriories_sum{std::accumulate(std::begin(apriories), std::end(apriories), 0.0)};

        return in_range && is_approx_one(belief_sum + uncertainty) && is_approx_one(apriories_sum);
    }

    [[nodiscard]] static inline constexpr std::size_t double_size(std::size_t size) {
        return size == std::dynamic_extent ? std::dynamic_extent : 2 * size;
    }

    static inline constexpr std::size_t Double = double_size(Size);

    FlexArray<F, Double> beliefsAndApriories;
    F uncertainty;
};

std::array a {0.5, 0.5};
MultinomialOpinion<double, 2> m {std::ranges::ref_view(a), 0.0, std::ranges::ref_view(a)};
MultinomialOpinion<double, std::dynamic_extent> m2 {std::ranges::ref_view(a), 0.0, std::ranges::ref_view(a)};

#endif
