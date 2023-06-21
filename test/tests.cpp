#include <array>
#include <span>

#include <gmock/gmock.h>

#include "../src/SubjectiveLogic.h"

TEST(SubjectiveOpinion, CreateBinomialOpinion) {
    EXPECT_NO_THROW(std::invoke(
            []() { BinomialOpinion<double> b{0.7, 0.2, 0.1, 0.5}; }
    ));
}

TEST(SubjectiveOpinion, CreateMultinomialOpinion) {
    std::array beliefs {0.3, 0.4, 0.2};
    std::array apriories {0.2, 0.3, 0.5};

    EXPECT_NO_THROW(std::invoke(
        [&](){
            MultinomialOpinion<double, 3> m {std::ranges::ref_view(beliefs), 0.1, std::ranges::ref_view(apriories)};
        }
    ));

    EXPECT_NO_THROW(std::invoke(
            [&](){
                MultinomialOpinion<double, std::dynamic_extent> m{std::ranges::ref_view(beliefs), 0.1, std::ranges::ref_view(apriories)};
            }
    ));
}
