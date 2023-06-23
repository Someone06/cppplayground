#include <array>
#include<ranges>
#include <span>
#include <stdexcept>

#include <gmock/gmock.h>

#include "../src/SubjectiveLogic.h"

template<plain_floating_point F>
void testBinomialOpinionThrows(F belief, F disbelief, F uncertainty, F apriori) {
    EXPECT_THROW(
        std::invoke(
            [=]() { BinomialOpinion<F> binomialOpinion{belief, disbelief, uncertainty, apriori }; }
        ),
        std::invalid_argument
    );
}

template<plain_floating_point F>
void testBinomialOpinionSuccess(F belief, F disbelief, F uncertainty, F apriori) {
    EXPECT_NO_THROW(
        std::invoke(
            [=]() { BinomialOpinion<F> binomialOpinion{belief, disbelief, uncertainty, apriori }; }
        )
    );

    BinomialOpinion<F> binomialOpinion{belief, disbelief, uncertainty, apriori };
    ASSERT_EQ(binomialOpinion.getBelief(), belief);
    ASSERT_EQ(binomialOpinion.getDisbelief(), disbelief);
    ASSERT_EQ(binomialOpinion.getUncertainty(), uncertainty);
    ASSERT_EQ(binomialOpinion.getApriori(), apriori);
}

TEST(BinomialOpinion, CreateBinomialOpinionSuccess) {
    testBinomialOpinionSuccess(0.7, 0.2, 0.1, 0.5);
}

TEST(BinomialOpinion, CreateLogicalTrue) {
    testBinomialOpinionSuccess(1.0, 0.0, 0.0, 1.0);
}

TEST(BinomialOpinion, CreateLogicalFalse) {
    testBinomialOpinionSuccess(0.0, 1.0, 0.0, 0.0);
}

TEST(BinomialOpinion, CreateLogicalUnknown) {
    testBinomialOpinionSuccess(0.0, 0.0, 1.0, 0.5);
}

TEST(BinomialOpinion, NotEnoughBeliefMass) {
    testBinomialOpinionThrows(0.1, 0.2, 0.3, 0.4);
}

TEST(BinomialOpinion, TooMuchBeliefMass) {
    testBinomialOpinionThrows(0.7, 0.8, 0.9, 0.8);
}

TEST(BinomialOpinion, TooMuchApriori) {
    testBinomialOpinionThrows(0.1, 0.2, 0.7, 1.1);
}

TEST(BinomialOpinion, NegativeBelief) {
    testBinomialOpinionThrows(-0.1, 0.2, 0.9, 0.2);
}

TEST(BinomialOpinion, NegativeDisbelief) {
    testBinomialOpinionThrows(0.4, -0.2, 0.8, 0.4);
}

TEST(BinomialOpinion, NegativeUncertainty) {
    testBinomialOpinionThrows(0.8, 0.7, -0.5, 0.1);
}

TEST(BinomialOpinion, NegativeApriori) {
    testBinomialOpinionThrows(0.6, 0.1, 0.3, -0.3);
}