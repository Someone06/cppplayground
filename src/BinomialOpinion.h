#ifndef CPPPLAYGROUND_BINOMIALOPINION_H
#define CPPPLAYGROUND_BINOMIALOPINION_H

#include <ostream>
#include <stdexcept>

#include "floatingPointHelper.h"

template<plain_floating_point F>
class BinomialOpinion final {
public:
    [[nodiscard]] constexpr BinomialOpinion(F belief, F disbelief, F uncertainty, F apriori)
        : belief{belief}, disbelief{disbelief}, uncertainty{uncertainty}, apriori{apriori} {
        if (!verifySelf())
            throw std::invalid_argument{"Binomial opinion is invalid."};
    }

    [[nodiscard]] constexpr F getBelief() const noexcept {
        return belief;
    }

    [[nodiscard]] constexpr F getDisbelief() const noexcept {
        return disbelief;
    }

    [[nodiscard]] constexpr F getUncertainty() const noexcept {
        return uncertainty;
    }

    [[nodiscard]] constexpr F getApriori() const noexcept {
        return apriori;
    }

private:
    [[nodiscard]] constexpr bool verifySelf() const noexcept {
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
constexpr std::ostream &operator<<(std::ostream &os, const BinomialOpinion<F>& opinion) {
    return (os << "BinomialOpinion{belief: " << opinion.getBelief()
               << ", disbelief: " << opinion.getDisbelief()
               << ", uncertainty: " << opinion.getUncertainty()
               << ", apriori: " << opinion.getApriori() << "}");
}

#endif
