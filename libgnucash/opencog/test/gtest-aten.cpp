/*
 * gtest-aten.cpp
 *
 * Unit tests for ATen tensor library
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include "../aten/tensor.hpp"
#include "../aten/tensor_ops.hpp"

using namespace gnc::aten;

class ATenTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(ATenTest, CreateTensor)
{
    auto tensor = DoubleTensor({1.0, 2.0, 3.0, 4.0});
    
    EXPECT_EQ(tensor.size(), 4);
    EXPECT_EQ(tensor.at(0), 1.0);
    EXPECT_EQ(tensor.at(1), 2.0);
    EXPECT_EQ(tensor.at(2), 3.0);
    EXPECT_EQ(tensor.at(3), 4.0);
}

TEST_F(ATenTest, TensorZeros)
{
    auto tensor = DoubleTensor::zeros({3, 4});
    
    EXPECT_EQ(tensor.sizes().size(), 2);
    EXPECT_EQ(tensor.sizes()[0], 3);
    EXPECT_EQ(tensor.sizes()[1], 4);
    EXPECT_EQ(tensor.size(), 12);
    
    for (size_t i = 0; i < tensor.size(); ++i) {
        EXPECT_EQ(tensor.at(i), 0.0);
    }
}

TEST_F(ATenTest, TensorOnes)
{
    auto tensor = DoubleTensor::ones({2, 3});
    
    EXPECT_EQ(tensor.size(), 6);
    for (size_t i = 0; i < tensor.size(); ++i) {
        EXPECT_EQ(tensor.at(i), 1.0);
    }
}

TEST_F(ATenTest, TensorEye)
{
    auto tensor = DoubleTensor::eye(3);
    
    EXPECT_EQ(tensor.sizes()[0], 3);
    EXPECT_EQ(tensor.sizes()[1], 3);
    
    // Check diagonal
    EXPECT_EQ(tensor.at({0, 0}), 1.0);
    EXPECT_EQ(tensor.at({1, 1}), 1.0);
    EXPECT_EQ(tensor.at({2, 2}), 1.0);
    
    // Check off-diagonal
    EXPECT_EQ(tensor.at({0, 1}), 0.0);
    EXPECT_EQ(tensor.at({1, 0}), 0.0);
}

TEST_F(ATenTest, TensorAddition)
{
    auto a = DoubleTensor({1.0, 2.0, 3.0});
    auto b = DoubleTensor({4.0, 5.0, 6.0});
    
    auto result = a + b;
    
    EXPECT_EQ(result.at(0), 5.0);
    EXPECT_EQ(result.at(1), 7.0);
    EXPECT_EQ(result.at(2), 9.0);
}

TEST_F(ATenTest, TensorSubtraction)
{
    auto a = DoubleTensor({5.0, 7.0, 9.0});
    auto b = DoubleTensor({1.0, 2.0, 3.0});
    
    auto result = a - b;
    
    EXPECT_EQ(result.at(0), 4.0);
    EXPECT_EQ(result.at(1), 5.0);
    EXPECT_EQ(result.at(2), 6.0);
}

TEST_F(ATenTest, TensorMultiplication)
{
    auto a = DoubleTensor({2.0, 3.0, 4.0});
    auto b = DoubleTensor({3.0, 4.0, 5.0});
    
    auto result = a * b;
    
    EXPECT_EQ(result.at(0), 6.0);
    EXPECT_EQ(result.at(1), 12.0);
    EXPECT_EQ(result.at(2), 20.0);
}

TEST_F(ATenTest, ScalarOperations)
{
    auto tensor = DoubleTensor({1.0, 2.0, 3.0});
    
    auto result = tensor * 2.0;
    
    EXPECT_EQ(result.at(0), 2.0);
    EXPECT_EQ(result.at(1), 4.0);
    EXPECT_EQ(result.at(2), 6.0);
}

TEST_F(ATenTest, TensorSum)
{
    auto tensor = DoubleTensor({1.0, 2.0, 3.0, 4.0});
    
    double sum = tensor.sum();
    
    EXPECT_EQ(sum, 10.0);
}

TEST_F(ATenTest, TensorMean)
{
    auto tensor = DoubleTensor({1.0, 2.0, 3.0, 4.0});
    
    double mean = tensor.mean();
    
    EXPECT_EQ(mean, 2.5);
}

TEST_F(ATenTest, TensorMin)
{
    auto tensor = DoubleTensor({3.0, 1.0, 4.0, 2.0});
    
    double min = tensor.min();
    
    EXPECT_EQ(min, 1.0);
}

TEST_F(ATenTest, TensorMax)
{
    auto tensor = DoubleTensor({3.0, 1.0, 4.0, 2.0});
    
    double max = tensor.max();
    
    EXPECT_EQ(max, 4.0);
}

TEST_F(ATenTest, TensorReshape)
{
    auto tensor = DoubleTensor({1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    
    auto reshaped = tensor.reshape({2, 3});
    
    EXPECT_EQ(reshaped.sizes()[0], 2);
    EXPECT_EQ(reshaped.sizes()[1], 3);
    EXPECT_EQ(reshaped.at({0, 0}), 1.0);
    EXPECT_EQ(reshaped.at({1, 2}), 6.0);
}

TEST_F(ATenTest, MatrixMultiplication)
{
    // 2x3 matrix
    auto a = DoubleTensor({1.0, 2.0, 3.0, 4.0, 5.0, 6.0}).reshape({2, 3});
    // 3x2 matrix
    auto b = DoubleTensor({1.0, 2.0, 3.0, 4.0, 5.0, 6.0}).reshape({3, 2});
    
    auto result = a.matmul(b);
    
    EXPECT_EQ(result.sizes()[0], 2);
    EXPECT_EQ(result.sizes()[1], 2);
    
    // [1,2,3]   [1,2]   [22, 28]
    // [4,5,6] x [3,4] = [49, 64]
    //           [5,6]
    EXPECT_EQ(result.at({0, 0}), 22.0);
    EXPECT_EQ(result.at({0, 1}), 28.0);
    EXPECT_EQ(result.at({1, 0}), 49.0);
    EXPECT_EQ(result.at({1, 1}), 64.0);
}

TEST_F(ATenTest, TensorOpsNormalize)
{
    auto tensor = DoubleTensor({1.0, 2.0, 3.0, 4.0});
    
    auto normalized = tensor_ops::normalize(tensor);
    
    double sum_squares = 0.0;
    for (size_t i = 0; i < normalized.size(); ++i) {
        sum_squares += normalized.at(i) * normalized.at(i);
    }
    
    EXPECT_NEAR(sum_squares, 1.0, 1e-6);
}

TEST_F(ATenTest, TensorOpsSoftmax)
{
    auto tensor = DoubleTensor({1.0, 2.0, 3.0});
    
    auto result = tensor_ops::softmax(tensor);
    
    // Softmax should sum to 1
    double sum = result.sum();
    EXPECT_NEAR(sum, 1.0, 1e-6);
    
    // All values should be positive
    for (size_t i = 0; i < result.size(); ++i) {
        EXPECT_GT(result.at(i), 0.0);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
