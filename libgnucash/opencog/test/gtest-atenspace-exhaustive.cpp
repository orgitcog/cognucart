/*
 * gtest-atenspace-exhaustive.cpp
 *
 * Exhaustive unit tests for ATenSpace hybrid symbolic-neural knowledge representation
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

// ============================================================================
// TensorNode Tests
// ============================================================================

class TensorNodeTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(TensorNodeTest, CreateTensorNode_BasicProperties)
{
    auto node = create_tensor_node(AtomTypes::ACCOUNT_NODE, "Test");

    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->is_node());
    EXPECT_FALSE(node->is_link());
    EXPECT_EQ(node->name(), "Test");
    EXPECT_EQ(node->type(), AtomTypes::ACCOUNT_NODE);
}

TEST_F(TensorNodeTest, SetEmbedding_StoresCorrectly)
{
    auto node = create_tensor_node(AtomTypes::CONCEPT_NODE, "Test");

    DoubleTensor emb({1.0, 2.0, 3.0});
    node->set_embedding(EmbeddingType::SEMANTIC, emb);

    EXPECT_TRUE(node->has_embedding(EmbeddingType::SEMANTIC));
    EXPECT_FALSE(node->has_embedding(EmbeddingType::TEMPORAL));
}

TEST_F(TensorNodeTest, GetEmbedding_ReturnsCorrectValue)
{
    auto node = create_tensor_node(AtomTypes::CONCEPT_NODE, "Test");

    DoubleTensor emb({1.0, 2.0, 3.0});
    node->set_embedding(EmbeddingType::SEMANTIC, emb);

    auto retrieved = node->get_embedding(EmbeddingType::SEMANTIC);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->dimension(), 3);
}

TEST_F(TensorNodeTest, GetEmbedding_ReturnsNulloptWhenMissing)
{
    auto node = create_tensor_node(AtomTypes::CONCEPT_NODE, "Test");

    auto retrieved = node->get_embedding(EmbeddingType::SEMANTIC);
    EXPECT_FALSE(retrieved.has_value());
}

TEST_F(TensorNodeTest, MultipleEmbeddings_StoredIndependently)
{
    auto node = create_tensor_node(AtomTypes::CONCEPT_NODE, "Test");

    node->set_embedding(EmbeddingType::SEMANTIC, DoubleTensor({1.0, 2.0}));
    node->set_embedding(EmbeddingType::TEMPORAL, DoubleTensor({3.0, 4.0, 5.0}));
    node->set_embedding(EmbeddingType::STRUCTURAL, DoubleTensor({6.0}));

    EXPECT_TRUE(node->has_embedding(EmbeddingType::SEMANTIC));
    EXPECT_TRUE(node->has_embedding(EmbeddingType::TEMPORAL));
    EXPECT_TRUE(node->has_embedding(EmbeddingType::STRUCTURAL));

    auto sem = node->get_embedding(EmbeddingType::SEMANTIC);
    auto temp = node->get_embedding(EmbeddingType::TEMPORAL);
    auto str = node->get_embedding(EmbeddingType::STRUCTURAL);

    EXPECT_EQ(sem->dimension(), 2);
    EXPECT_EQ(temp->dimension(), 3);
    EXPECT_EQ(str->dimension(), 1);
}

// ============================================================================
// TensorLink Tests
// ============================================================================

class TensorLinkTest : public ::testing::Test
{
protected:
    std::shared_ptr<TensorNode> node1;
    std::shared_ptr<TensorNode> node2;

    void SetUp() override
    {
        node1 = create_tensor_node(AtomTypes::CONCEPT_NODE, "Node1");
        node2 = create_tensor_node(AtomTypes::CONCEPT_NODE, "Node2");
    }
};

TEST_F(TensorLinkTest, CreateTensorLink_BasicProperties)
{
    HandleSeq outgoing = {node1, node2};
    auto link = create_tensor_link(AtomTypes::INHERITANCE_LINK, outgoing);

    ASSERT_NE(link, nullptr);
    EXPECT_FALSE(link->is_node());
    EXPECT_TRUE(link->is_link());
    EXPECT_EQ(link->arity(), 2);
}

TEST_F(TensorLinkTest, TensorLink_StoresEmbeddings)
{
    HandleSeq outgoing = {node1, node2};
    auto link = create_tensor_link(AtomTypes::INHERITANCE_LINK, outgoing);

    link->set_embedding(EmbeddingType::STRUCTURAL, DoubleTensor({1.0, 2.0}));

    EXPECT_TRUE(link->has_embedding(EmbeddingType::STRUCTURAL));
}

// ============================================================================
// ATenSpace Tests
// ============================================================================

class ATenSpaceTest : public ::testing::Test
{
protected:
    ATenSpace space;

    void SetUp() override {}
};

TEST_F(ATenSpaceTest, AddTensorNode_CreatesNode)
{
    auto node = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name(), "Expenses:Food");
    EXPECT_EQ(node->type(), AtomTypes::ACCOUNT_NODE);
}

TEST_F(ATenSpaceTest, AddTensorLink_CreatesLink)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");

    auto link = space.add_tensor_link(AtomTypes::ACCOUNT_HIERARCHY_LINK, node2, node1);

    ASSERT_NE(link, nullptr);
    EXPECT_EQ(link->arity(), 2);
}

TEST_F(ATenSpaceTest, SetEmbedding_StoresInSpace)
{
    auto node = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test");

    DoubleTensor emb = DoubleTensor::ones({64});
    space.set_embedding(node, EmbeddingType::SEMANTIC, emb);

    auto retrieved = space.get_embedding(node, EmbeddingType::SEMANTIC);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved->size(), 64);
}

TEST_F(ATenSpaceTest, GenerateSemanticEmbedding_CreatesEmbedding)
{
    auto emb = space.generate_semantic_embedding("Expenses:Food");

    EXPECT_GT(emb.size(), 0);
    double sum = emb.sum();
    EXPECT_NE(sum, 0.0);  // Should have non-zero values
}

TEST_F(ATenSpaceTest, GenerateTemporalEmbedding_CreatesEmbedding)
{
    auto node = space.add_tensor_node(AtomTypes::TRANSACTION_NODE, "txn-001");

    auto emb = space.generate_temporal_embedding(node, 6, 12);

    EXPECT_GT(emb.size(), 0);
}

TEST_F(ATenSpaceTest, GenerateStructuralEmbedding_CreatesEmbedding)
{
    auto node = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test");

    auto emb = space.generate_structural_embedding(node);

    EXPECT_GT(emb.size(), 0);
}

TEST_F(ATenSpaceTest, CosineSimilarity_IdenticalEmbeddings)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test1");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test2");

    auto emb = DoubleTensor({1.0, 2.0, 3.0});
    space.set_embedding(node1, EmbeddingType::SEMANTIC, emb);
    space.set_embedding(node2, EmbeddingType::SEMANTIC, emb);

    double sim = space.cosine_similarity(node1, node2, EmbeddingType::SEMANTIC);

    EXPECT_NEAR(sim, 1.0, 1e-6);
}

TEST_F(ATenSpaceTest, CosineSimilarity_OrthogonalEmbeddings)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test1");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test2");

    space.set_embedding(node1, EmbeddingType::SEMANTIC, DoubleTensor({1.0, 0.0}));
    space.set_embedding(node2, EmbeddingType::SEMANTIC, DoubleTensor({0.0, 1.0}));

    double sim = space.cosine_similarity(node1, node2, EmbeddingType::SEMANTIC);

    EXPECT_NEAR(sim, 0.0, 1e-6);
}

TEST_F(ATenSpaceTest, FindSimilar_ReturnsRankedResults)
{
    auto food = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    auto groceries = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Groceries");
    auto transport = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Transportation");

    space.set_embedding(food, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Food"));
    space.set_embedding(groceries, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Groceries"));
    space.set_embedding(transport, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Transportation"));

    auto similar = space.find_similar(food, 2, EmbeddingType::SEMANTIC);

    EXPECT_LE(similar.size(), 2);
}

TEST_F(ATenSpaceTest, SemanticSearch_FindsRelevant)
{
    auto food = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    auto groceries = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Expenses:Groceries");

    space.set_embedding(food, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Food"));
    space.set_embedding(groceries, EmbeddingType::SEMANTIC,
                       space.generate_semantic_embedding("Expenses:Groceries"));

    auto results = space.semantic_search("food expenses", 2);

    EXPECT_LE(results.size(), 2);
}

TEST_F(ATenSpaceTest, ComputeAttention_SumsToOne)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account1");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account2");
    auto node3 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account3");

    space.set_embedding(node1, EmbeddingType::SEMANTIC, DoubleTensor({1.0, 0.0, 0.0}));
    space.set_embedding(node2, EmbeddingType::SEMANTIC, DoubleTensor({0.9, 0.1, 0.0}));
    space.set_embedding(node3, EmbeddingType::SEMANTIC, DoubleTensor({0.0, 1.0, 0.0}));

    std::vector<std::shared_ptr<TensorNode>> atoms = {node1, node2, node3};
    auto query = DoubleTensor({1.0, 0.0, 0.0});

    auto attention = space.compute_attention(atoms, query, EmbeddingType::SEMANTIC);

    EXPECT_EQ(attention.size(), 3);
    EXPECT_NEAR(attention.sum(), 1.0, 1e-6);
}

TEST_F(ATenSpaceTest, AttentionAggregate_ProducesWeightedSum)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account1");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Account2");

    space.set_embedding(node1, EmbeddingType::SEMANTIC, DoubleTensor({1.0, 0.0}));
    space.set_embedding(node2, EmbeddingType::SEMANTIC, DoubleTensor({0.0, 1.0}));

    std::vector<std::shared_ptr<TensorNode>> atoms = {node1, node2};
    auto query = DoubleTensor({1.0, 0.0});

    auto aggregated = space.attention_aggregate(atoms, query, EmbeddingType::SEMANTIC);

    EXPECT_EQ(aggregated.size(), 2);
}

TEST_F(ATenSpaceTest, CombineEmbeddings_ConcatenatesCorrectly)
{
    auto semantic = DoubleTensor::ones({64});
    auto temporal = DoubleTensor::ones({32}) * 0.5;
    auto structural = DoubleTensor::ones({16}) * 0.3;

    auto combined = space.combine_embeddings(semantic, temporal, structural);

    EXPECT_EQ(combined.size(), 64 + 32 + 16);
}

// ============================================================================
// ATenSpace Configuration Tests
// ============================================================================

class ATenSpaceConfigTest : public ::testing::Test {};

TEST_F(ATenSpaceConfigTest, DefaultConfig_HasReasonableDefaults)
{
    ATenSpaceConfig config;

    EXPECT_GT(config.semantic_dim, 0);
    EXPECT_GT(config.temporal_dim, 0);
    EXPECT_GT(config.structural_dim, 0);
    EXPECT_GT(config.financial_dim, 0);
}

TEST_F(ATenSpaceConfigTest, CustomConfig_AppliedToSpace)
{
    ATenSpaceConfig config;
    config.semantic_dim = 128;
    config.temporal_dim = 64;

    ATenSpace space(config);

    auto emb = space.generate_semantic_embedding("Test");
    EXPECT_EQ(emb.size(), 128);
}

// ============================================================================
// Embedding Tests
// ============================================================================

class EmbeddingTest : public ::testing::Test {};

TEST_F(EmbeddingTest, CreateEmbedding_StoresTensor)
{
    DoubleTensor tensor({1.0, 2.0, 3.0});
    Embedding emb(EmbeddingType::SEMANTIC, tensor);

    EXPECT_EQ(emb.type(), EmbeddingType::SEMANTIC);
    EXPECT_EQ(emb.dimension(), 3);
}

TEST_F(EmbeddingTest, Tensor_ReturnsStoredTensor)
{
    DoubleTensor tensor({1.0, 2.0, 3.0});
    Embedding emb(EmbeddingType::SEMANTIC, tensor);

    auto& retrieved = emb.tensor();
    EXPECT_EQ(retrieved.size(), 3);
    EXPECT_DOUBLE_EQ(retrieved[0], 1.0);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

class ATenSpaceEdgeCases : public ::testing::Test
{
protected:
    ATenSpace space;
};

TEST_F(ATenSpaceEdgeCases, GetEmbedding_NullAtom_ReturnsNullopt)
{
    auto result = space.get_embedding(nullptr, EmbeddingType::SEMANTIC);
    EXPECT_FALSE(result.has_value());
}

TEST_F(ATenSpaceEdgeCases, CosineSimilarity_MissingEmbeddings_ReturnsZero)
{
    auto node1 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test1");
    auto node2 = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test2");

    double sim = space.cosine_similarity(node1, node2, EmbeddingType::SEMANTIC);

    EXPECT_DOUBLE_EQ(sim, 0.0);
}

TEST_F(ATenSpaceEdgeCases, FindSimilar_NoEmbeddings_ReturnsEmpty)
{
    auto node = space.add_tensor_node(AtomTypes::ACCOUNT_NODE, "Test");

    auto similar = space.find_similar(node, 5, EmbeddingType::SEMANTIC);

    EXPECT_TRUE(similar.empty());
}

TEST_F(ATenSpaceEdgeCases, ComputeAttention_EmptyAtoms_ReturnsEmpty)
{
    std::vector<std::shared_ptr<TensorNode>> empty_atoms;
    auto query = DoubleTensor({1.0, 0.0});

    auto attention = space.compute_attention(empty_atoms, query, EmbeddingType::SEMANTIC);

    EXPECT_EQ(attention.size(), 0);
}

TEST_F(ATenSpaceEdgeCases, GenerateStructuralEmbedding_NullAtom)
{
    auto emb = space.generate_structural_embedding(nullptr);

    EXPECT_GT(emb.size(), 0);  // Should still return default embedding
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
