/*
 * gtest-aten-exhaustive.cpp
 *
 * Exhaustive unit tests for ATen tensor library
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include <cmath>
#include <limits>
#include "../aten/tensor.hpp"
#include "../aten/tensor_ops.hpp"

using namespace gnc::aten;

// ============================================================================
// Tensor Construction Tests
// ============================================================================

class TensorConstructionTest : public ::testing::Test {};

TEST_F(TensorConstructionTest, DefaultConstructor_CreatesEmptyTensor)
{
    DoubleTensor tensor;
    EXPECT_EQ(tensor.size(), 0);
    EXPECT_TRUE(tensor.empty());
}

TEST_F(TensorConstructionTest, InitializerList_CreatesTensorWithData)
{
    DoubleTensor tensor({1.0, 2.0, 3.0, 4.0});

    EXPECT_EQ(tensor.size(), 4);
    EXPECT_EQ(tensor[0], 1.0);
    EXPECT_EQ(tensor[3], 4.0);
}

TEST_F(TensorConstructionTest, ShapeConstructor_CreatesZeroInitializedTensor)
{
    DoubleTensor tensor({3, 4});

    EXPECT_EQ(tensor.size(), 12);
    EXPECT_EQ(tensor.ndim(), 2);
    EXPECT_EQ(tensor.sizes()[0], 3);
    EXPECT_EQ(tensor.sizes()[1], 4);
}

TEST_F(TensorConstructionTest, ShapeAndValue_CreatesFilled)
{
    DoubleTensor tensor({2, 3}, 5.0);

    EXPECT_EQ(tensor.size(), 6);
    for (size_t i = 0; i < tensor.size(); ++i) {
        EXPECT_EQ(tensor[i], 5.0);
    }
}

TEST_F(TensorConstructionTest, ShapeAndVector_UsesProvidedData)
{
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    DoubleTensor tensor({2, 3}, data);

    EXPECT_EQ(tensor.at({0, 0}), 1.0);
    EXPECT_EQ(tensor.at({1, 2}), 6.0);
}

TEST_F(TensorConstructionTest, Zeros_CreatesAllZeros)
{
    auto tensor = DoubleTensor::zeros({3, 4, 5});

    EXPECT_EQ(tensor.size(), 60);
    for (size_t i = 0; i < tensor.size(); ++i) {
        EXPECT_EQ(tensor[i], 0.0);
    }
}

TEST_F(TensorConstructionTest, Ones_CreatesAllOnes)
{
    auto tensor = DoubleTensor::ones({2, 3});

    EXPECT_EQ(tensor.size(), 6);
    for (size_t i = 0; i < tensor.size(); ++i) {
        EXPECT_EQ(tensor[i], 1.0);
    }
}

TEST_F(TensorConstructionTest, Eye_CreatesIdentityMatrix)
{
    auto tensor = DoubleTensor::eye(3);

    EXPECT_EQ(tensor.sizes()[0], 3);
    EXPECT_EQ(tensor.sizes()[1], 3);

    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            if (i == j) {
                EXPECT_EQ(tensor.at({i, j}), 1.0);
            } else {
                EXPECT_EQ(tensor.at({i, j}), 0.0);
            }
        }
    }
}

TEST_F(TensorConstructionTest, Arange_CreatesSequence)
{
    auto tensor = DoubleTensor::arange(0.0, 5.0, 1.0);

    EXPECT_EQ(tensor.size(), 5);
    EXPECT_EQ(tensor[0], 0.0);
    EXPECT_EQ(tensor[4], 4.0);
}

TEST_F(TensorConstructionTest, Arange_WithStep)
{
    auto tensor = DoubleTensor::arange(0.0, 10.0, 2.0);

    EXPECT_EQ(tensor.size(), 5);
    EXPECT_EQ(tensor[0], 0.0);
    EXPECT_EQ(tensor[2], 4.0);
    EXPECT_EQ(tensor[4], 8.0);
}

TEST_F(TensorConstructionTest, Linspace_CreatesEvenlySpaced)
{
    auto tensor = DoubleTensor::linspace(0.0, 1.0, 5);

    EXPECT_EQ(tensor.size(), 5);
    EXPECT_DOUBLE_EQ(tensor[0], 0.0);
    EXPECT_DOUBLE_EQ(tensor[4], 1.0);
    EXPECT_DOUBLE_EQ(tensor[2], 0.5);
}

// ============================================================================
// Tensor Properties Tests
// ============================================================================

class TensorPropertiesTest : public ::testing::Test {};

TEST_F(TensorPropertiesTest, Size_ReturnsTotal)
{
    DoubleTensor tensor({2, 3, 4});
    EXPECT_EQ(tensor.size(), 24);
}

TEST_F(TensorPropertiesTest, SizeWithDim_ReturnsDimSize)
{
    DoubleTensor tensor({2, 3, 4});

    EXPECT_EQ(tensor.size(0), 2);
    EXPECT_EQ(tensor.size(1), 3);
    EXPECT_EQ(tensor.size(2), 4);
}

TEST_F(TensorPropertiesTest, Ndim_ReturnsNumberOfDimensions)
{
    DoubleTensor t1({5});
    DoubleTensor t2({2, 3});
    DoubleTensor t3({2, 3, 4});

    EXPECT_EQ(t1.ndim(), 1);
    EXPECT_EQ(t2.ndim(), 2);
    EXPECT_EQ(t3.ndim(), 3);
}

TEST_F(TensorPropertiesTest, Empty_DetectsEmptyTensor)
{
    DoubleTensor empty;
    DoubleTensor nonEmpty({1.0});

    EXPECT_TRUE(empty.empty());
    EXPECT_FALSE(nonEmpty.empty());
}

TEST_F(TensorPropertiesTest, Shape_ReturnsCorrectShape)
{
    DoubleTensor tensor({2, 3, 4});
    auto shape = tensor.shape();

    EXPECT_EQ(shape.size(), 3);
    EXPECT_EQ(shape[0], 2);
    EXPECT_EQ(shape[1], 3);
    EXPECT_EQ(shape[2], 4);
}

TEST_F(TensorPropertiesTest, Sizes_IsAliasForShape)
{
    DoubleTensor tensor({2, 3, 4});

    EXPECT_EQ(tensor.shape(), tensor.sizes());
}

// ============================================================================
// Element Access Tests
// ============================================================================

class TensorAccessTest : public ::testing::Test {};

TEST_F(TensorAccessTest, At_WithFlatIndex)
{
    DoubleTensor tensor({1.0, 2.0, 3.0, 4.0});

    EXPECT_EQ(tensor.at(0), 1.0);
    EXPECT_EQ(tensor.at(3), 4.0);
}

TEST_F(TensorAccessTest, At_WithMultiIndex)
{
    DoubleTensor tensor({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    EXPECT_EQ(tensor.at({0, 0}), 1.0);
    EXPECT_EQ(tensor.at({0, 2}), 3.0);
    EXPECT_EQ(tensor.at({1, 0}), 4.0);
    EXPECT_EQ(tensor.at({1, 2}), 6.0);
}

TEST_F(TensorAccessTest, OperatorBracket_AccessesByFlatIndex)
{
    DoubleTensor tensor({10.0, 20.0, 30.0});

    EXPECT_EQ(tensor[0], 10.0);
    EXPECT_EQ(tensor[2], 30.0);
}

TEST_F(TensorAccessTest, At_ThrowsOnOutOfRange)
{
    DoubleTensor tensor({1.0, 2.0, 3.0});

    EXPECT_THROW(tensor.at(10), std::out_of_range);
}

TEST_F(TensorAccessTest, Modification_ThroughAccess)
{
    DoubleTensor tensor({1.0, 2.0, 3.0});

    tensor[1] = 100.0;

    EXPECT_EQ(tensor[1], 100.0);
}

TEST_F(TensorAccessTest, Modification_ThroughAt)
{
    DoubleTensor tensor({2, 2}, {1.0, 2.0, 3.0, 4.0});

    tensor.at({1, 1}) = 99.0;

    EXPECT_EQ(tensor.at({1, 1}), 99.0);
}

// ============================================================================
// Shape Operations Tests
// ============================================================================

class TensorShapeOpsTest : public ::testing::Test {};

TEST_F(TensorShapeOpsTest, Reshape_ChangesShape)
{
    DoubleTensor tensor({1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    auto reshaped = tensor.reshape({2, 3});

    EXPECT_EQ(reshaped.sizes()[0], 2);
    EXPECT_EQ(reshaped.sizes()[1], 3);
    EXPECT_EQ(reshaped.at({0, 0}), 1.0);
    EXPECT_EQ(reshaped.at({1, 2}), 6.0);
}

TEST_F(TensorShapeOpsTest, Reshape_PreservesData)
{
    DoubleTensor tensor({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    auto reshaped = tensor.reshape({3, 2});

    for (size_t i = 0; i < tensor.size(); ++i) {
        EXPECT_EQ(tensor[i], reshaped[i]);
    }
}

TEST_F(TensorShapeOpsTest, Flatten_Creates1D)
{
    DoubleTensor tensor({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    auto flat = tensor.flatten();

    EXPECT_EQ(flat.ndim(), 1);
    EXPECT_EQ(flat.size(), 6);
}

TEST_F(TensorShapeOpsTest, Transpose_2D)
{
    DoubleTensor tensor({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    auto t = tensor.transpose();

    EXPECT_EQ(t.sizes()[0], 3);
    EXPECT_EQ(t.sizes()[1], 2);
    EXPECT_EQ(t.at({0, 0}), 1.0);
    EXPECT_EQ(t.at({0, 1}), 4.0);
    EXPECT_EQ(t.at({2, 0}), 3.0);
}

// ============================================================================
// Arithmetic Operations Tests
// ============================================================================

class TensorArithmeticTest : public ::testing::Test {};

TEST_F(TensorArithmeticTest, Addition_ElementWise)
{
    DoubleTensor a({1.0, 2.0, 3.0});
    DoubleTensor b({4.0, 5.0, 6.0});

    auto result = a + b;

    EXPECT_EQ(result[0], 5.0);
    EXPECT_EQ(result[1], 7.0);
    EXPECT_EQ(result[2], 9.0);
}

TEST_F(TensorArithmeticTest, Subtraction_ElementWise)
{
    DoubleTensor a({5.0, 7.0, 9.0});
    DoubleTensor b({1.0, 2.0, 3.0});

    auto result = a - b;

    EXPECT_EQ(result[0], 4.0);
    EXPECT_EQ(result[1], 5.0);
    EXPECT_EQ(result[2], 6.0);
}

TEST_F(TensorArithmeticTest, Multiplication_ElementWise)
{
    DoubleTensor a({2.0, 3.0, 4.0});
    DoubleTensor b({3.0, 4.0, 5.0});

    auto result = a * b;

    EXPECT_EQ(result[0], 6.0);
    EXPECT_EQ(result[1], 12.0);
    EXPECT_EQ(result[2], 20.0);
}

TEST_F(TensorArithmeticTest, Division_ElementWise)
{
    DoubleTensor a({10.0, 20.0, 30.0});
    DoubleTensor b({2.0, 4.0, 5.0});

    auto result = a / b;

    EXPECT_EQ(result[0], 5.0);
    EXPECT_EQ(result[1], 5.0);
    EXPECT_EQ(result[2], 6.0);
}

TEST_F(TensorArithmeticTest, ScalarMultiplication)
{
    DoubleTensor a({1.0, 2.0, 3.0});

    auto result = a * 2.0;

    EXPECT_EQ(result[0], 2.0);
    EXPECT_EQ(result[1], 4.0);
    EXPECT_EQ(result[2], 6.0);
}

TEST_F(TensorArithmeticTest, ScalarDivision)
{
    DoubleTensor a({4.0, 8.0, 12.0});

    auto result = a / 2.0;

    EXPECT_EQ(result[0], 2.0);
    EXPECT_EQ(result[1], 4.0);
    EXPECT_EQ(result[2], 6.0);
}

TEST_F(TensorArithmeticTest, UnaryNegation)
{
    DoubleTensor a({1.0, -2.0, 3.0});

    auto result = -a;

    EXPECT_EQ(result[0], -1.0);
    EXPECT_EQ(result[1], 2.0);
    EXPECT_EQ(result[2], -3.0);
}

// ============================================================================
// Aggregation Tests
// ============================================================================

class TensorAggregationTest : public ::testing::Test {};

TEST_F(TensorAggregationTest, Sum_ReturnsTotal)
{
    DoubleTensor tensor({1.0, 2.0, 3.0, 4.0});

    EXPECT_DOUBLE_EQ(tensor.sum(), 10.0);
}

TEST_F(TensorAggregationTest, Mean_ReturnsAverage)
{
    DoubleTensor tensor({1.0, 2.0, 3.0, 4.0});

    EXPECT_DOUBLE_EQ(tensor.mean(), 2.5);
}

TEST_F(TensorAggregationTest, Min_ReturnsMinimum)
{
    DoubleTensor tensor({3.0, 1.0, 4.0, 1.0, 5.0});

    EXPECT_DOUBLE_EQ(tensor.min(), 1.0);
}

TEST_F(TensorAggregationTest, Max_ReturnsMaximum)
{
    DoubleTensor tensor({3.0, 1.0, 4.0, 1.0, 5.0});

    EXPECT_DOUBLE_EQ(tensor.max(), 5.0);
}

TEST_F(TensorAggregationTest, Variance_CalculatesCorrectly)
{
    DoubleTensor tensor({2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0});

    // Mean = 5, Variance = 4
    double var = tensor.var();
    EXPECT_NEAR(var, 4.0, 0.1);
}

TEST_F(TensorAggregationTest, Std_CalculatesCorrectly)
{
    DoubleTensor tensor({2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0});

    double stddev = tensor.std();
    EXPECT_NEAR(stddev, 2.0, 0.1);
}

// ============================================================================
// Matrix Operations Tests
// ============================================================================

class TensorMatrixOpsTest : public ::testing::Test {};

TEST_F(TensorMatrixOpsTest, MatMul_2x3_times_3x2)
{
    auto a = DoubleTensor({1.0, 2.0, 3.0, 4.0, 5.0, 6.0}).reshape({2, 3});
    auto b = DoubleTensor({1.0, 2.0, 3.0, 4.0, 5.0, 6.0}).reshape({3, 2});

    auto result = a.matmul(b);

    EXPECT_EQ(result.sizes()[0], 2);
    EXPECT_EQ(result.sizes()[1], 2);

    // [1,2,3]   [1,2]   [22, 28]
    // [4,5,6] x [3,4] = [49, 64]
    //           [5,6]
    EXPECT_DOUBLE_EQ(result.at({0, 0}), 22.0);
    EXPECT_DOUBLE_EQ(result.at({0, 1}), 28.0);
    EXPECT_DOUBLE_EQ(result.at({1, 0}), 49.0);
    EXPECT_DOUBLE_EQ(result.at({1, 1}), 64.0);
}

TEST_F(TensorMatrixOpsTest, Dot_VectorProduct)
{
    DoubleTensor a({1.0, 2.0, 3.0});
    DoubleTensor b({4.0, 5.0, 6.0});

    double result = a.dot(b);

    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    EXPECT_DOUBLE_EQ(result, 32.0);
}

// ============================================================================
// tensor_ops Functions Tests
// ============================================================================

class TensorOpsTest : public ::testing::Test {};

TEST_F(TensorOpsTest, Softmax_SumsToOne)
{
    DoubleTensor tensor({1.0, 2.0, 3.0});

    auto result = tensor_ops::softmax(tensor);

    EXPECT_NEAR(result.sum(), 1.0, 1e-6);
}

TEST_F(TensorOpsTest, Softmax_AllPositive)
{
    DoubleTensor tensor({1.0, 2.0, 3.0});

    auto result = tensor_ops::softmax(tensor);

    for (size_t i = 0; i < result.size(); ++i) {
        EXPECT_GT(result[i], 0.0);
    }
}

TEST_F(TensorOpsTest, Softmax_LargerInputsGetHigherProbabilities)
{
    DoubleTensor tensor({1.0, 2.0, 3.0});

    auto result = tensor_ops::softmax(tensor);

    EXPECT_LT(result[0], result[1]);
    EXPECT_LT(result[1], result[2]);
}

TEST_F(TensorOpsTest, Normalize_UnitLength)
{
    DoubleTensor tensor({3.0, 4.0});  // 3-4-5 triangle

    auto result = tensor_ops::normalize(tensor);

    double length = std::sqrt(result[0] * result[0] + result[1] * result[1]);
    EXPECT_NEAR(length, 1.0, 1e-6);
}

TEST_F(TensorOpsTest, MovingAverage_CalculatesCorrectly)
{
    DoubleTensor tensor({1.0, 2.0, 3.0, 4.0, 5.0});

    auto result = ops::moving_average(tensor, 3);

    // Window of 3: (1+2+3)/3=2, (2+3+4)/3=3, (3+4+5)/3=4
    EXPECT_GE(result.size(), 3);
    EXPECT_NEAR(result[0], 2.0, 1e-6);
    EXPECT_NEAR(result[1], 3.0, 1e-6);
    EXPECT_NEAR(result[2], 4.0, 1e-6);
}

TEST_F(TensorOpsTest, EMA_CalculatesCorrectly)
{
    DoubleTensor tensor({1.0, 2.0, 3.0, 4.0, 5.0});

    auto result = ops::ema(tensor, 0.5);

    // EMA: y[0]=1, y[1]=0.5*2+0.5*1=1.5, y[2]=0.5*3+0.5*1.5=2.25, etc.
    EXPECT_EQ(result.size(), tensor.size());
    EXPECT_DOUBLE_EQ(result[0], 1.0);
    EXPECT_NEAR(result[1], 1.5, 1e-6);
}

// ============================================================================
// Edge Cases and Special Values
// ============================================================================

class TensorEdgeCases : public ::testing::Test {};

TEST_F(TensorEdgeCases, EmptyTensor_AggregationsHandled)
{
    DoubleTensor empty;

    // These should not crash
    EXPECT_EQ(empty.size(), 0);
}

TEST_F(TensorEdgeCases, SingleElement_AggregationsCorrect)
{
    DoubleTensor single({42.0});

    EXPECT_DOUBLE_EQ(single.sum(), 42.0);
    EXPECT_DOUBLE_EQ(single.mean(), 42.0);
    EXPECT_DOUBLE_EQ(single.min(), 42.0);
    EXPECT_DOUBLE_EQ(single.max(), 42.0);
}

TEST_F(TensorEdgeCases, NegativeValues_HandledCorrectly)
{
    DoubleTensor tensor({-5.0, -2.0, 0.0, 3.0, 7.0});

    EXPECT_DOUBLE_EQ(tensor.min(), -5.0);
    EXPECT_DOUBLE_EQ(tensor.max(), 7.0);
    EXPECT_DOUBLE_EQ(tensor.sum(), 3.0);
}

TEST_F(TensorEdgeCases, LargeValues_NoOverflow)
{
    DoubleTensor tensor({1e100, 1e100});

    double sum = tensor.sum();
    EXPECT_GT(sum, 1e100);  // Should not overflow
}

// ============================================================================
// Type-Specific Tests
// ============================================================================

TEST(TensorTypeTest, IntTensor_BasicOperations)
{
    IntTensor tensor({1, 2, 3, 4, 5});

    EXPECT_EQ(tensor.size(), 5);
    EXPECT_EQ(tensor.sum(), 15);
    EXPECT_EQ(tensor.min(), 1);
    EXPECT_EQ(tensor.max(), 5);
}

TEST(TensorTypeTest, FloatTensor_BasicOperations)
{
    FloatTensor tensor({1.0f, 2.0f, 3.0f});

    EXPECT_EQ(tensor.size(), 3);
    EXPECT_FLOAT_EQ(tensor.sum(), 6.0f);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
