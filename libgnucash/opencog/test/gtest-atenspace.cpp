/*
 * gtest-atenspace.cpp
 *
 * Unit tests for ATenSpace hybrid symbolic-neural knowledge representation
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include "../atenspace/atenspace.hpp"
#include "../atenspace/tensor_atom.hpp"

using namespace gnc::atenspace;
using namespace gnc::opencog;
using namespace gnc::aten;

class ATenSpaceTest : public ::testing::Test
{
protected:
    ATenSpace space;
    
    void SetUp() override {}
};

TEST_F(ATenSpaceTest, CreateTensorNode)
{
    auto node = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->is_node());
    EXPECT_EQ(node->name(), "Expenses:Food");
    EXPECT_EQ(node->type(), AtomTypes::ACCOUNT_NODE);
}

TEST_F(ATenSpaceTest, CreateTensorLink)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    
    auto link = space.add_tensor_link(AtomTypes::ACCOUNT_HIERARCHY_LINK, node2, node1);
    
    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_link());
    EXPECT_EQ(link->arity(), 2);
}

TEST_F(ATenSpaceTest, SetEmbedding)
{
    auto node = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    
    // Create embedding
    auto embedding = DoubleTensor::ones({64});
    space.set_embedding(node, EmbeddingType::SEMANTIC, embedding);
    
    // Retrieve embedding
    auto retrieved = space.get_embedding(node, EmbeddingType::SEMANTIC);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->size(), 64);
}

TEST_F(ATenSpaceTest, GenerateSemanticEmbedding)
{
    auto embedding = space.generate_semantic_embedding("Expenses:Food");
    
    EXPECT_GT(embedding.size(), 0);
    
    // Embedding should have reasonable values
    double sum = embedding.sum();
    EXPECT_GT(sum, 0.0);
}

TEST_F(ATenSpaceTest, GenerateTemporalEmbedding)
{
    auto node = space.add_tensor_node(AtomTypes::TRANSACTION_NODE, "txn-001");
    
    auto embedding = space.generate_temporal_embedding(node, 0, 12);
    
    EXPECT_GT(embedding.size(), 0);
}

TEST_F(ATenSpaceTest, GenerateStructuralEmbedding)
{
    auto node = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    
    auto embedding = space.generate_structural_embedding(node);
    
    EXPECT_GT(embedding.size(), 0);
}

TEST_F(ATenSpaceTest, CosineSimilarity)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Groceries");
    
    // Generate embeddings
    auto emb1 = space.generate_semantic_embedding("Expenses:Food");
    auto emb2 = space.generate_semantic_embedding("Expenses:Groceries");
    
    space.set_embedding(node1, EmbeddingType::SEMANTIC, emb1);
    space.set_embedding(node2, EmbeddingType::SEMANTIC, emb2);
    
    double similarity = space.cosine_similarity(node1, node2, EmbeddingType::SEMANTIC);
    
    // Similar accounts should have high similarity
    EXPECT_GT(similarity, 0.5);
    EXPECT_LE(similarity, 1.0);
}

TEST_F(ATenSpaceTest, SemanticSearch)
{
    // Add multiple accounts
    auto food = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    auto groceries = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Groceries");
    auto transport = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Transportation");
    
    // Generate embeddings
    space.set_embedding(food, EmbeddingType::SEMANTIC, 
                       space.generate_semantic_embedding("Expenses:Food"));
    space.set_embedding(groceries, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Groceries"));
    space.set_embedding(transport, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Transportation"));
    
    // Search
    auto results = space.semantic_search("food expenses", 2);
    
    EXPECT_LE(results.size(), 2);
}

TEST_F(ATenSpaceTest, FindSimilar)
{
    auto food = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    auto groceries = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Groceries");
    auto transport = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Transportation");
    
    // Generate embeddings
    space.set_embedding(food, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Food"));
    space.set_embedding(groceries, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Groceries"));
    space.set_embedding(transport, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Transportation"));
    
    auto similar = space.find_similar(food, 2, EmbeddingType::SEMANTIC);
    
    EXPECT_LE(similar.size(), 2);
}

TEST_F(ATenSpaceTest, ComputeAttention)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account1");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account2");
    auto node3 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account3");
    
    // Set embeddings
    space.set_embedding(node1, EmbeddingType::SEMANTIC, DoubleTensor({1.0, 0.0, 0.0}));
    space.set_embedding(node2, EmbeddingType::SEMANTIC, DoubleTensor({0.9, 0.1, 0.0}));
    space.set_embedding(node3, EmbeddingType::SEMANTIC, DoubleTensor({0.0, 1.0, 0.0}));
    
    std::vector<std::shared_ptr<TensorAtom>> atoms = {node1, node2, node3};
    auto query = DoubleTensor({1.0, 0.0, 0.0});
    
    auto attention = space.compute_attention(atoms, query, EmbeddingType::SEMANTIC);
    
    EXPECT_EQ(attention.size(), 3);
    
    // Attention should sum to 1
    double sum = attention.sum();
    EXPECT_NEAR(sum, 1.0, 1e-6);
    
    // node1 should have highest attention (most similar to query)
    EXPECT_GT(attention.at(0), attention.at(2));
}

TEST_F(ATenSpaceTest, AttentionAggregate)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account1");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account2");
    
    space.set_embedding(node1, EmbeddingType::SEMANTIC, DoubleTensor({1.0, 0.0}));
    space.set_embedding(node2, EmbeddingType::SEMANTIC, DoubleTensor({0.0, 1.0}));
    
    std::vector<std::shared_ptr<TensorAtom>> atoms = {node1, node2};
    auto query = DoubleTensor({1.0, 0.0});
    
    auto aggregated = space.attention_aggregate(atoms, query, EmbeddingType::SEMANTIC);
    
    EXPECT_EQ(aggregated.size(), 2);
}

TEST_F(ATenSpaceTest, CombineEmbeddings)
{
    auto semantic = DoubleTensor::ones({64});
    auto temporal = DoubleTensor::ones({32}) * 0.5;
    auto structural = DoubleTensor::ones({16}) * 0.3;
    
    auto combined = space.combine_embeddings(semantic, temporal, structural);
    
    // Combined should have sum of dimensions
    EXPECT_EQ(combined.size(), 64 + 32 + 16);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
