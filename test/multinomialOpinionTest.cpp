#include <cstdint>
#include <functional>
#include <span>
#include <stdexcept>

#include <gmock/gmock.h>

#include "../src/floatingPointHelper.h"
#include "../src/MultinomialOpinion.h"

template<plain_floating_point F, std::size_t N, std::size_t S, std::size_t T>
void testBinomialOpinionThrows(std::span<const F, S> beliefs, F uncertainty, std::span<const F, T> apriories) {
    EXPECT_THROW(
        std::invoke(
            [=]() { MultinomialOpinion<F, N> multinomialOpinion{beliefs, uncertainty, apriories }; }
        ),
        std::invalid_argument
    );
}

template<plain_floating_point F, std::size_t N, std::size_t S, std::size_t T>
void testBinomialOpinionDifferentLengthThrows(const std::array<F, S>& beliefs, F uncertainty, const std::array<F, T>& apriories) {
    testBinomialOpinionThrows<F, N, S, T>(std::span(beliefs), uncertainty, std::span(apriories));
    testBinomialOpinionThrows<F, std::dynamic_extent, S, T>(std::span(beliefs), uncertainty, std::span(apriories));
}

template<plain_floating_point F, std::size_t N>
void testBinomialOpinionThrow(const std::array<F, N>& beliefs, F uncertainty, const std::array<F, N>& apriories) {
    testBinomialOpinionDifferentLengthThrows<F, N, N, N>(beliefs, uncertainty, apriories);
}


TEST(CreateMultinomialOpinion, NotEnoughBeliefMass) {
    std::array beliefs {0.1, 0.2, 0.3};
    std::array apriories {0.3, 0.4, 0.3};
    testBinomialOpinionThrow(beliefs, 0.1, apriories);
}

TEST(CreateMultinomialOpinion, TooMuchBeliefMass) {
    std::array beliefs {0.4, 0.1, 0.4, 0.2};
    std::array apriories {0.3, 0.4, 0.3, 0.0};
    testBinomialOpinionThrow(beliefs, 0.1, apriories);
}

TEST(CreateMultinomialOpinion, NotEnoughAprioriMass) {
    std::array beliefs {0.1, 0.2, 0.3};
    std::array apriories {0.2, 0.3, 0.4};
    testBinomialOpinionThrow(beliefs, 0.4, apriories);
}

TEST(CreateMultinomialOpinion, TooMuchAprioriMass) {
    std::array beliefs {0.1, 0.2, 0.3};
    std::array apriories {0.3, 0.7, 0.2};
    testBinomialOpinionThrow(beliefs, 0.4, apriories);
}

TEST(CreateMultinomialOpinion, NegativeBelief) {
    std::array beliefs {0.6, -0.2, 0.1};
    std::array apriories {0.2, 0.3, 0.5};
    testBinomialOpinionThrow(beliefs, 0.5, apriories);
}

TEST(CreateMultinomialOpinion, NegativeUncertainty) {
    std::array beliefs {0.6, 0.8};
    std::array apriories {0.7, 0.3};
    testBinomialOpinionThrow(beliefs, -0.4, apriories);
}

TEST(CreateMultinomialOpinion, NegativApriori) {
    std::array beliefs {0.1, 0.2, 0.3, 0.4};
    std::array apriories {0.2, 0.3, 0.6, -0.1};
    testBinomialOpinionThrow(beliefs, 0.0, apriories);
}

TEST(CreateMultinomialOpinion, MoreBeliefsThanApriories) {
    std::array beliefs {0.5, 0.3, 0.1};
    std::array apriories {0.6, 0.4};
    testBinomialOpinionDifferentLengthThrows<double, 2, 3, 2>(beliefs, 0.1, apriories);
}

TEST(CreateMultinomialOpinion, MoreAprioriesThenBeliefs) {
    std::array beliefs {0.5, 0.3};
    std::array apriories {0.3, 0.4, 0.5};
    testBinomialOpinionDifferentLengthThrows<double, 2, 2, 3>(beliefs, 0.2, apriories);
}

TEST(CreateMultinomialOpinion, OnlyOneEvent) {
    std::array beliefs {0.8};
    std::array apriories {1.0};
    testBinomialOpinionThrows<double, std::dynamic_extent, 1, 1>(beliefs, 0.2, apriories);
}

TEST(CreateMultinomialOpinion, BeliefsAndAprioriesDifferntTypesSuccess) {
    std::array beliefs {0.7, 0.1};
    std::vector apriories {0.2, 0.8};

    EXPECT_NO_THROW(
            std::invoke(
                    [=]() { MultinomialOpinion<double, 2> multinomialOpinion{std::ranges::ref_view(beliefs), 0.2, std::span(apriories) }; }
                    )
    );

    EXPECT_NO_THROW(
            std::invoke(
                    [=]() { MultinomialOpinion<double, std::dynamic_extent> multinomialOpinion{std::span(beliefs), 0.2, std::ranges::ref_view(apriories) }; }
                    )
    );
}

template<plain_floating_point F, std::size_t N, std::size_t M>
void testBinomialOpinionSuccess(std::span<const F, M> beliefs, F uncertainty, std::span<const F, M> apriories) {
    EXPECT_NO_THROW(
            std::invoke(
                    [=]() { MultinomialOpinion<F, N> multinomialOpinion{beliefs, uncertainty, apriories }; }
                    )
    );

    MultinomialOpinion<F, N> multinomialOpinion {beliefs, uncertainty, apriories };
    EXPECT_TRUE(std::ranges::equal(beliefs, multinomialOpinion.getBeliefs()));
    EXPECT_EQ(uncertainty, multinomialOpinion.getUncertainty());
    EXPECT_TRUE(std::ranges::equal(apriories, multinomialOpinion.getApriories()));
    EXPECT_EQ(beliefs.size(), multinomialOpinion.size());
    EXPECT_EQ(N == std::dynamic_extent, multinomialOpinion.is_dynamic_sized());
}

template<plain_floating_point F, std::size_t N>
void testBinomialOpinionSuccess(const std::array<F, N>& beliefs, F uncertainty, const std::array<F, N>& apriories) {
    testBinomialOpinionSuccess<F, N, N>(std::span(beliefs), uncertainty, std::span(apriories));
    testBinomialOpinionSuccess<F, std::dynamic_extent, N>(std::span(beliefs), uncertainty, std::span(apriories));
}

TEST(CreateMultinomialOpinion, NoUncertainty) {
    std::array beliefs {0.1, 0.2, 0.3, 0.4};
    std::array apriories {0.1, 0.1, 0.7, 0.1};
    testBinomialOpinionSuccess(beliefs, 0.0, apriories);
}

TEST(CreateMultinomialOpinion, FullUncertinaty) {
    std::array beliefs {0.0, 0.0};
    std::array apriories {0.6, 0.4};
    testBinomialOpinionSuccess(beliefs, 1.0, apriories);
}
