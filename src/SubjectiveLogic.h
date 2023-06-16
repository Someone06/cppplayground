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


template<plain_floating_point F>
class MultinomialOpinion final {
public:
    template<std::ranges::input_range R>
        requires std::ranges::sized_range<R> && std::same_as<std::ranges::range_value_t<R>, F>
    [[nodiscard]] constexpr MultinomialOpinion(const R &beliefs, F uncertainty, const R &apriories) : beliefsAndApriories{std::vector<F>()}, uncertainty{uncertainty} {
        if (std::ranges::size(beliefs) != std::ranges::size(apriories))
            throw std::invalid_argument("Number of beliefs and number of apriories must be equal.");

        if (std::ranges::size(beliefs) < 2)
            throw std::invalid_argument("Require at least two beliefs and apriories.");

        std::array combined{std::ranges::ref_view(beliefs), std::ranges::ref_view(apriories)};
        auto joined{std::ranges::views::join(std::move(combined))};
        std::vector<F> vec{std::begin(joined), std::end(joined)};
        std::swap(vec, beliefsAndApriories);

        if (!verifySelf())
            throw std::invalid_argument("Invariant for multinomial opinion does not hold.");
    }

    using size_type = typename std::vector<F>::size_type;

    [[nodiscard]] constexpr auto getBeliefs() const {
        return left_half(beliefsAndApriories);
    }

    [[nodiscard]] constexpr auto getApriories() const {
        return right_half(beliefsAndApriories);
    }

    [[nodiscard]] inline constexpr size_type size() const {
        return beliefsAndApriories.size() / 2;
    }
    friend std::ostream &operator<<(std::ostream &os, const MultinomialOpinion &opinion) {
        return (os << "MultinomialOpinion{beliefsAndApriories: "
                   << opinion.beliefsAndApriories
                   << ", uncertainty: "
                   << opinion.uncertainty << "}");
    }

private:
    [[nodiscard]] constexpr bool verifySelf() const {
        bool in_range{std::ranges::all_of(beliefsAndApriories, is_between_zero_and_one_inclusive<F>)};

        auto beliefs{getBeliefs()};
        F belief_sum{std::accumulate(std::begin(beliefs), std::end(beliefs), 0.0)};

        auto apriories{getApriories()};
        F apriories_sum{std::accumulate(std::begin(apriories), std::end(apriories), 0.0)};

        return in_range && is_approx_one(belief_sum + uncertainty) && is_approx_one(apriories_sum);
    }

    std::vector<F> beliefsAndApriories{};
    F uncertainty{};
};
#endif
