#ifndef CPPPLAYGROUND_OPERATIONS_H
#define CPPPLAYGROUND_OPERATIONS_H

#include <cstddef>

#include "floatingPointHelper.h"
#include "MultinomialOpinion.h"
#include "BinomialOpinion.h"


template<plain_floating_point F, std::size_t N>
    requires (N != std::dynamic_extent)
[[nodiscard]] constexpr MultinomialOpinion<F, std::dynamic_extent> make_dynamic(const MultinomialOpinion<F, N>& multinomialOpinion) {
    return MultinomialOpinion<F, std::dynamic_extent>{multinomialOpinion.getBeliefs(), multinomialOpinion.getUncertainty(), multinomialOpinion.getApriories()};
}

template<plain_floating_point F, std::size_t N>
    requires (N != std::dynamic_extent)
[[nodiscard]] constexpr MultinomialOpinion<F, N> make_static(const MultinomialOpinion<F, std::dynamic_extent>& multinomialOpinion) {
    if(N != multinomialOpinion.size()) {
        throw std::invalid_argument("Static and dynamic size do not match.");
    }

    return MultinomialOpinion<F, N>{multinomialOpinion.getBeliefs(), multinomialOpinion.getUncertainty(), multinomialOpinion.getApriories()};
}

template<plain_floating_point F, std::size_t N, bool IsNoexcept = false>
[[nodiscard]] constexpr BinomialOpinion<F> coarsen_unsafe(const MultinomialOpinion<F, N>& multinomialOpinion, std::size_t to) noexcept(IsNoexcept) {
    F belief {multinomialOpinion.getBeliefs().at(to)};
    F uncertainty {multinomialOpinion.getUncertainty()};
    F disbelief {1.0 - belief - uncertainty};
    F apriori {multinomialOpinion.getApriories().at(to)};
    return BinomialOpinion<F>{belief, disbelief, uncertainty, apriori};
}

template<plain_floating_point F, std::size_t N>
[[nodiscard]] constexpr BinomialOpinion<F> coarsen(const MultinomialOpinion<F, N>& multinomialOpinion, std::size_t to) {
    if(multinomialOpinion.size() >= to) {
        throw std::invalid_argument("Cannot coarsen to argument that is out of range.");
    }

    return coarsen_unsafe<F, N, true>(multinomialOpinion, to);
}


template<plain_floating_point F, std::size_t N, std::size_t TO>
requires ((N != std::dynamic_extent) && (TO < N)) || (TO < 2)
[[nodiscard]] constexpr BinomialOpinion<F> coarsen(const MultinomialOpinion<F, N>& multinomialOpinion) noexcept {
    return coarsen_unsafe<F, N, true>(multinomialOpinion, TO);
}

#endif
