#ifndef CPPPLAYGROUND_SUBJECTIVELOGIC_H
#define CPPPLAYGROUND_SUBJECTIVELOGIC_H

#include <concepts>
#include <iostream>
#include <numeric>
#include <ranges>
#include <vector>

#include "rangeHelper.h"
#include "floatingPointHelper.h"

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

template<typename R>
concept OpinionView = std::ranges::contiguous_range<R>
                        && std::ranges::sized_range<R>
                        && std::ranges::view<R>
                        && plain_floating_point<std::ranges::range_value_t<R>>;

template<plain_floating_point F, std::size_t Size = std::dynamic_extent>
requires (Size >= 2)
class MultinomialOpinionBase {
protected:
    constexpr MultinomialOpinionBase(std::size_t size, F uncertainty) : beliefsAndApriories{}, uncertainty{uncertainty} {
        if (size != Size) throw std::invalid_argument("Size missmatch.");
    }

    std::array<F, 2*Size> beliefsAndApriories;
    F uncertainty;
};

template<plain_floating_point F>
class MultinomialOpinionBase<F, std::dynamic_extent> {
protected:
    constexpr MultinomialOpinionBase(std::size_t size, F uncertainty) : beliefsAndApriories{std::vector<F>(2*size)}, uncertainty{uncertainty} {
        if(size < 2) throw std::invalid_argument("Require size of at least 2.");
    }

    std::vector<F> beliefsAndApriories;
    F uncertainty;
};

template<plain_floating_point F, std::size_t Size>
requires (Size >= 2)
class MultinomialOpinion final : private MultinomialOpinionBase<F, Size> {
public:
    template<std::ranges::input_range R>
        requires std::ranges::view<R> && std::ranges::sized_range<R> && std::same_as<std::ranges::range_value_t<R>, F>
    [[nodiscard]] constexpr MultinomialOpinion(const R beliefs, F uncertainty, const R apriories) : MultinomialOpinionBase<F, Size>{std::ranges::size(beliefs), uncertainty} {
        if (std::ranges::size(beliefs) != std::ranges::size(apriories))
            throw std::invalid_argument("Number of beliefs and number of apriories must be equal.");

        std::array combined{std::move(beliefs), std::move(apriories)};
        auto joined{std::ranges::views::join(std::move(combined))};
        std::ranges::copy(joined, std::begin(MultinomialOpinionBase<F, Size>::beliefsAndApriories));

        if (!verifySelf())
            throw std::invalid_argument("Invariant for multinomial opinion does not hold.");
    }

    [[nodiscard]] constexpr OpinionView auto getBeliefs() const {
        return left_half(MultinomialOpinionBase<F, Size>::beliefsAndApriories);
    }

    [[nodiscard]] constexpr OpinionView auto getApriories() const {
        return right_half(MultinomialOpinionBase<F, Size>::beliefsAndApriories);
    }

    [[nodiscard]] inline constexpr std::size_t size() const {
        if constexpr (Size == std::dynamic_extent)
            return std::size(MultinomialOpinionBase<F, Size>::beliefsAndPrioiries);

        return Size;
    }

private:
    [[nodiscard]] constexpr bool verifySelf() const {
        bool in_range{std::ranges::all_of(MultinomialOpinionBase<F, Size>::beliefsAndApriories, is_between_zero_and_one_inclusive<F>)};

        auto beliefs{getBeliefs()};
        F belief_sum{std::accumulate(std::begin(beliefs), std::end(beliefs), 0.0)};

        auto apriories{getApriories()};
        F apriories_sum{std::accumulate(std::begin(apriories), std::end(apriories), 0.0)};

        return in_range && is_approx_one(belief_sum + MultinomialOpinionBase<F, Size>::uncertainty) && is_approx_one(apriories_sum);
    }
};

std::array a {0.5, 0.5};
MultinomialOpinion<double, 2> m {std::ranges::ref_view(a), 0.0, std::ranges::ref_view(a)};
MultinomialOpinion<double, std::dynamic_extent> m2 {std::ranges::ref_view(a), 0.0, std::ranges::ref_view(a)};

#endif
