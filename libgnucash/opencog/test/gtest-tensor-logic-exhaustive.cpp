/*
 * gtest-tensor-logic-exhaustive.cpp
 *
 * Exhaustive unit tests for Tensor Logic Engine, TensorAccount, TensorNetwork
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include "../tensor-logic/tensor_logic_engine.hpp"
#include "../tensor-logic/tensor_account.hpp"
#include "../tensor-logic/tensor_network.hpp"

using namespace gnc::tensor_logic;
using namespace gnc::aten;

// ============================================================================
// TensorAccount Tests
// ============================================================================

class TensorAccountTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(TensorAccountTest, Construction_SetsProperties)
{
    TensorAccount account("guid-001", "Expenses:Food", 2, 12, 1);

    EXPECT_EQ(account.guid(), "guid-001");
    EXPECT_EQ(account.name(), "Expenses:Food");
    EXPECT_EQ(account.num_entities(), 2);
    EXPECT_EQ(account.num_periods(), 12);
    EXPECT_EQ(account.num_currencies(), 1);
}

TEST_F(TensorAccountTest, SetAndGetMetric_Works)
{
    TensorAccount account("test", "Test", 1, 12, 1);

    account.set_metric(0, 5, 0, TensorAccount::Metrics::BALANCE, 1000.0);
    double balance = account.get_metric(0, 5, 0, TensorAccount::Metrics::BALANCE);

    EXPECT_DOUBLE_EQ(balance, 1000.0);
}

TEST_F(TensorAccountTest, GetBalance_ReturnsCorrectValue)
{
    TensorAccount account("test", "Test", 1, 12, 1);

    account.set_metric(0, 3, 0, TensorAccount::Metrics::BALANCE, 500.0);
    double balance = account.get_balance(0, 3, 0);

    EXPECT_DOUBLE_EQ(balance, 500.0);
}

TEST_F(TensorAccountTest, GetBalances_ReturnsTensor)
{
    TensorAccount account("test", "Test", 2, 3, 1);

    account.set_metric(0, 0, 0, TensorAccount::Metrics::BALANCE, 100.0);
    account.set_metric(1, 2, 0, TensorAccount::Metrics::BALANCE, 200.0);

    auto balances = account.get_balances();

    EXPECT_EQ(balances.at({0, 0, 0}), 100.0);
    EXPECT_EQ(balances.at({1, 2, 0}), 200.0);
}

TEST_F(TensorAccountTest, GetTimeSeries_ExtractsCorrectDimension)
{
    TensorAccount account("test", "Test", 1, 4, 1);

    for (size_t p = 0; p < 4; ++p) {
        account.set_metric(0, p, 0, TensorAccount::Metrics::BALANCE, (p + 1) * 100.0);
    }

    auto series = account.get_time_series(0, 0, TensorAccount::Metrics::BALANCE);

    EXPECT_EQ(series.size(), 4);
    EXPECT_DOUBLE_EQ(series[0], 100.0);
    EXPECT_DOUBLE_EQ(series[3], 400.0);
}

TEST_F(TensorAccountTest, GetEntityComparison_WorksCorrectly)
{
    TensorAccount account("test", "Test", 3, 1, 1);

    account.set_metric(0, 0, 0, TensorAccount::Metrics::BALANCE, 100.0);
    account.set_metric(1, 0, 0, TensorAccount::Metrics::BALANCE, 200.0);
    account.set_metric(2, 0, 0, TensorAccount::Metrics::BALANCE, 300.0);

    auto comparison = account.get_entity_comparison(0, 0, TensorAccount::Metrics::BALANCE);

    EXPECT_EQ(comparison.size(), 3);
    EXPECT_DOUBLE_EQ(comparison[0], 100.0);
    EXPECT_DOUBLE_EQ(comparison[2], 300.0);
}

TEST_F(TensorAccountTest, Consolidate_SumsAcrossEntities)
{
    TensorAccount account("test", "Test", 3, 2, 1);

    // Set balances for 3 entities, 2 periods
    account.set_metric(0, 0, 0, TensorAccount::Metrics::BALANCE, 100.0);
    account.set_metric(1, 0, 0, TensorAccount::Metrics::BALANCE, 200.0);
    account.set_metric(2, 0, 0, TensorAccount::Metrics::BALANCE, 300.0);

    auto consolidated = account.consolidate();

    EXPECT_EQ(consolidated.num_entities(), 1);
    EXPECT_DOUBLE_EQ(consolidated.get_balance(0, 0, 0), 600.0);
}

TEST_F(TensorAccountTest, EntityContributionMatrix_ComputesRatios)
{
    TensorAccount account("test", "Test", 3, 1, 1);

    account.set_metric(0, 0, 0, TensorAccount::Metrics::BALANCE, 100.0);
    account.set_metric(1, 0, 0, TensorAccount::Metrics::BALANCE, 100.0);
    account.set_metric(2, 0, 0, TensorAccount::Metrics::BALANCE, 200.0);

    auto contributions = account.entity_contribution_matrix(0, 0);

    // Total = 400, so contributions are 0.25, 0.25, 0.5
    EXPECT_NEAR(contributions[0], 0.25, 1e-6);
    EXPECT_NEAR(contributions[1], 0.25, 1e-6);
    EXPECT_NEAR(contributions[2], 0.5, 1e-6);
}

TEST_F(TensorAccountTest, ComputeVolatility_CalculatesCorrectly)
{
    TensorAccount account("test", "Test", 1, 5, 1);

    // Set varying balances
    account.set_metric(0, 0, 0, TensorAccount::Metrics::BALANCE, 100.0);
    account.set_metric(0, 1, 0, TensorAccount::Metrics::BALANCE, 110.0);
    account.set_metric(0, 2, 0, TensorAccount::Metrics::BALANCE, 105.0);
    account.set_metric(0, 3, 0, TensorAccount::Metrics::BALANCE, 120.0);
    account.set_metric(0, 4, 0, TensorAccount::Metrics::BALANCE, 115.0);

    account.compute_volatility(0, 0);

    double vol = account.get_metric(0, 0, 0, TensorAccount::Metrics::VOLATILITY);
    EXPECT_GT(vol, 0.0);  // Should be positive for varying data
}

TEST_F(TensorAccountTest, ComputeTrend_CalculatesSlope)
{
    TensorAccount account("test", "Test", 1, 5, 1);

    // Set increasing balances
    for (size_t p = 0; p < 5; ++p) {
        account.set_metric(0, p, 0, TensorAccount::Metrics::BALANCE, 100.0 + p * 10.0);
    }

    account.compute_trend(0, 0);

    double trend = account.get_metric(0, 0, 0, TensorAccount::Metrics::TREND);
    EXPECT_GT(trend, 0.0);  // Positive trend for increasing data
}

TEST_F(TensorAccountTest, GetEmbedding_CreatesVector)
{
    TensorAccount account("test", "Test", 1, 4, 1);

    for (size_t p = 0; p < 4; ++p) {
        account.set_metric(0, p, 0, TensorAccount::Metrics::BALANCE, (p + 1) * 100.0);
        account.set_metric(0, p, 0, TensorAccount::Metrics::NET_FLOW, (p + 1) * 10.0);
    }

    auto emb = account.get_embedding();

    EXPECT_GT(emb.size(), 0);
}

TEST_F(TensorAccountTest, MultiScaleDecomposition_CreatesMultipleLevels)
{
    TensorAccount account("test", "Test", 1, 12, 1);

    for (size_t p = 0; p < 12; ++p) {
        account.set_metric(0, p, 0, TensorAccount::Metrics::BALANCE,
                          100.0 + 10.0 * std::sin(p * 0.5));
    }

    auto scales = account.multi_scale_decomposition(0, 0);

    EXPECT_GE(scales.size(), 1);  // At least original scale
}

// ============================================================================
// TensorAccountSet Tests
// ============================================================================

class TensorAccountSetTest : public ::testing::Test
{
protected:
    TensorAccountSet account_set;

    void SetUp() override
    {
        auto acc1 = std::make_shared<TensorAccount>("acc-001", "Expenses:Food", 1, 12, 1);
        auto acc2 = std::make_shared<TensorAccount>("acc-002", "Expenses:Transport", 1, 12, 1);
        auto acc3 = std::make_shared<TensorAccount>("acc-003", "Income:Salary", 1, 12, 1);

        account_set.add_account(acc1);
        account_set.add_account(acc2);
        account_set.add_account(acc3);
    }
};

TEST_F(TensorAccountSetTest, AddAccount_IncreasesSize)
{
    EXPECT_EQ(account_set.size(), 3);
}

TEST_F(TensorAccountSetTest, GetAccount_ReturnsCorrectAccount)
{
    auto account = account_set.get_account("acc-001");

    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->name(), "Expenses:Food");
}

TEST_F(TensorAccountSetTest, GetAccount_ReturnsNullForNonexistent)
{
    auto account = account_set.get_account("nonexistent");

    EXPECT_EQ(account, nullptr);
}

TEST_F(TensorAccountSetTest, AsTensor_CreatesCorrectShape)
{
    auto tensor = account_set.as_tensor(0, 0);

    EXPECT_EQ(tensor.sizes()[0], 3);   // 3 accounts
    EXPECT_EQ(tensor.sizes()[1], 12);  // 12 periods
}

TEST_F(TensorAccountSetTest, ClusterAccounts_ReturnsGroups)
{
    auto clusters = account_set.cluster_accounts(2);

    EXPECT_EQ(clusters.size(), 2);

    // Total accounts across clusters should equal total accounts
    size_t total = 0;
    for (const auto& cluster : clusters) {
        total += cluster.size();
    }
    EXPECT_EQ(total, 3);
}

// ============================================================================
// TensorNetwork Tests
// ============================================================================

class TensorNetworkTest : public ::testing::Test
{
protected:
    TensorNetwork network;

    void SetUp() override {}
};

TEST_F(TensorNetworkTest, AddNode_IncreasesNodeCount)
{
    network.add_node("checking");
    network.add_node("savings");

    auto stats = network.get_stats();
    EXPECT_EQ(stats.num_nodes, 2);
}

TEST_F(TensorNetworkTest, AddEdge_CreatesConnection)
{
    network.add_node("income");
    network.add_node("checking");
    network.add_edge("income", "checking", 1000.0);

    auto stats = network.get_stats();
    EXPECT_EQ(stats.num_edges, 1);
}

TEST_F(TensorNetworkTest, GetAdjacencyMatrix_CorrectShape)
{
    network.add_node("a");
    network.add_node("b");
    network.add_node("c");
    network.add_edge("a", "b", 100.0);
    network.add_edge("b", "c", 50.0);

    auto adj = network.get_adjacency_matrix();

    EXPECT_EQ(adj.sizes()[0], 3);
    EXPECT_EQ(adj.sizes()[1], 3);
}

TEST_F(TensorNetworkTest, FlowCentrality_ComputesValues)
{
    network.add_node("income");
    network.add_node("checking");
    network.add_node("expenses");
    network.add_edge("income", "checking", 1000.0);
    network.add_edge("checking", "expenses", 500.0);

    auto centrality = network.flow_centrality();

    EXPECT_EQ(centrality.size(), 3);
}

TEST_F(TensorNetworkTest, PageRank_ComputesValues)
{
    network.add_node("a");
    network.add_node("b");
    network.add_node("c");
    network.add_edge("a", "b", 1.0);
    network.add_edge("b", "c", 1.0);
    network.add_edge("c", "a", 1.0);

    auto pagerank = network.pagerank();

    EXPECT_EQ(pagerank.size(), 3);
    double sum = pagerank.sum();
    EXPECT_NEAR(sum, 1.0, 0.1);  // PageRank should sum to ~1
}

TEST_F(TensorNetworkTest, ShortestPath_FindsPath)
{
    network.add_node("a");
    network.add_node("b");
    network.add_node("c");
    network.add_edge("a", "b", 1.0);
    network.add_edge("b", "c", 1.0);

    auto path = network.shortest_path("a", "c");

    EXPECT_GE(path.size(), 2);  // At least start and end
}

TEST_F(TensorNetworkTest, DetectCircularFlows_FindsCycles)
{
    network.add_node("a");
    network.add_node("b");
    network.add_node("c");
    network.add_edge("a", "b", 100.0);
    network.add_edge("b", "c", 100.0);
    network.add_edge("c", "a", 100.0);  // Creates cycle

    auto cycles = network.detect_circular_flows();

    EXPECT_GE(cycles.size(), 1);
}

// ============================================================================
// TensorLogicEngine Tests
// ============================================================================

class TensorLogicEngineTest : public ::testing::Test
{
protected:
    TensorLogicEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(TensorLogicEngineTest, Initialize_SetsUp)
{
    auto stats = engine.get_stats();
    // Should have zero accounts initially
    EXPECT_EQ(stats.num_accounts, 0);
}

TEST_F(TensorLogicEngineTest, CreateAccount_AddsAccount)
{
    auto account = engine.create_account("acc-001", "Expenses:Food", 1, 12, 1);

    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->guid(), "acc-001");

    auto stats = engine.get_stats();
    EXPECT_EQ(stats.num_accounts, 1);
}

TEST_F(TensorLogicEngineTest, GetAccount_ReturnsAccount)
{
    engine.create_account("acc-001", "Expenses:Food", 1, 12, 1);

    auto account = engine.get_account("acc-001");

    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->name(), "Expenses:Food");
}

TEST_F(TensorLogicEngineTest, ImportAccountData_SetsMetrics)
{
    engine.create_account("acc-001", "Expenses:Food", 1, 12, 1);

    bool result = engine.import_account_data("acc-001", 0, 0, 0, 1000.0, 500.0, 200.0);

    EXPECT_TRUE(result);

    auto account = engine.get_account("acc-001");
    EXPECT_DOUBLE_EQ(account->get_balance(0, 0, 0), 1000.0);
}

TEST_F(TensorLogicEngineTest, ImportAccountData_ReturnsFalseForNonexistent)
{
    bool result = engine.import_account_data("nonexistent", 0, 0, 0, 1000.0, 500.0, 200.0);

    EXPECT_FALSE(result);
}

TEST_F(TensorLogicEngineTest, RecordTransaction_UpdatesNetwork)
{
    engine.record_transaction("income", "checking", 1000.0, 0);

    auto stats = engine.get_stats();
    EXPECT_GT(stats.num_network_nodes, 0);
    EXPECT_GT(stats.num_network_edges, 0);
}

TEST_F(TensorLogicEngineTest, ConsolidateEntities_CombinesData)
{
    engine.create_account("acc-001", "Expenses", 3, 12, 1);

    engine.import_account_data("acc-001", 0, 0, 0, 1000.0, 500.0, 200.0);
    engine.import_account_data("acc-001", 1, 0, 0, 2000.0, 800.0, 400.0);
    engine.import_account_data("acc-001", 2, 0, 0, 1500.0, 600.0, 300.0);

    auto result = engine.consolidate_entities("acc-001");

    EXPECT_EQ(result.analysis_type, "entity_consolidation");
    EXPECT_GE(result.data.size(), 1);
    EXPECT_DOUBLE_EQ(result.data[0], 4500.0);  // Sum of balances
}

TEST_F(TensorLogicEngineTest, MultiScaleAnalysis_ProducesResult)
{
    engine.create_account("acc-001", "Expenses", 1, 12, 1);

    for (size_t p = 0; p < 12; ++p) {
        engine.import_account_data("acc-001", 0, p, 0, 100.0 * (p + 1), 0.0, 0.0);
    }

    auto result = engine.multi_scale_analysis("acc-001");

    EXPECT_EQ(result.analysis_type, "multi_scale");
}

TEST_F(TensorLogicEngineTest, AggregateToScale_ReturnsAccount)
{
    engine.create_account("acc-001", "Expenses", 1, 12, 1);

    for (size_t p = 0; p < 12; ++p) {
        engine.import_account_data("acc-001", 0, p, 0, 100.0, 0.0, 0.0);
    }

    auto quarterly = engine.aggregate_to_scale("acc-001", TimeScale::QUARTERLY);

    ASSERT_NE(quarterly, nullptr);
    EXPECT_GT(quarterly->num_periods(), 0);
}

TEST_F(TensorLogicEngineTest, CompareEntities_ProducesTensor)
{
    engine.create_account("acc-001", "Expenses", 3, 12, 1);

    auto comparison = engine.compare_entities("acc-001", 0);

    EXPECT_GT(comparison.size(), 0);
}

TEST_F(TensorLogicEngineTest, AnalyzeFlowNetwork_ProducesResult)
{
    engine.record_transaction("income", "checking", 2000.0, 0);
    engine.record_transaction("checking", "expenses", 500.0, 0);
    engine.record_transaction("checking", "savings", 300.0, 0);

    auto result = engine.analyze_flow_network();

    EXPECT_EQ(result.analysis_type, "flow_network");
}

TEST_F(TensorLogicEngineTest, Forecast_ProducesResult)
{
    engine.create_account("acc-001", "Expenses", 1, 12, 1);

    for (size_t p = 0; p < 12; ++p) {
        engine.import_account_data("acc-001", 0, p, 0, 100.0 + p * 10.0, 0.0, 0.0);
    }

    auto result = engine.forecast("acc-001", 3);

    EXPECT_EQ(result.analysis_type, "forecast");
    EXPECT_GE(result.data.size(), 1);
}

TEST_F(TensorLogicEngineTest, FindSimilarAccounts_ReturnsResults)
{
    engine.create_account("acc-001", "Expenses:Food", 1, 4, 1);
    engine.create_account("acc-002", "Expenses:Groceries", 1, 4, 1);
    engine.create_account("acc-003", "Income:Salary", 1, 4, 1);

    // Set similar patterns for food and groceries
    for (size_t p = 0; p < 4; ++p) {
        engine.import_account_data("acc-001", 0, p, 0, 100.0 + p * 10, 0.0, 0.0);
        engine.import_account_data("acc-002", 0, p, 0, 110.0 + p * 10, 0.0, 0.0);
        engine.import_account_data("acc-003", 0, p, 0, 1000.0 - p * 50, 0.0, 0.0);
    }

    auto similar = engine.find_similar_accounts("acc-001", 2);

    EXPECT_LE(similar.size(), 2);
}

TEST_F(TensorLogicEngineTest, ClusterAccounts_ProducesGroups)
{
    engine.create_account("acc-001", "Expenses:Food", 1, 4, 1);
    engine.create_account("acc-002", "Expenses:Groceries", 1, 4, 1);
    engine.create_account("acc-003", "Income:Salary", 1, 4, 1);

    auto clusters = engine.cluster_accounts(2);

    EXPECT_EQ(clusters.size(), 2);
}

TEST_F(TensorLogicEngineTest, GetStats_ReturnsCorrectCounts)
{
    engine.create_account("acc-001", "Test1", 1, 12, 1);
    engine.create_account("acc-002", "Test2", 1, 12, 1);
    engine.record_transaction("a", "b", 100.0, 0);

    auto stats = engine.get_stats();

    EXPECT_EQ(stats.num_accounts, 2);
    EXPECT_GT(stats.num_network_nodes, 0);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

class TensorLogicEdgeCases : public ::testing::Test
{
protected:
    TensorLogicEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(TensorLogicEdgeCases, ConsolidateEntities_NonexistentAccount_ReturnsDefault)
{
    auto result = engine.consolidate_entities("nonexistent");

    EXPECT_EQ(result.analysis_type, "entity_consolidation");
}

TEST_F(TensorLogicEdgeCases, AggregateToScale_NonexistentAccount_ReturnsNull)
{
    auto result = engine.aggregate_to_scale("nonexistent", TimeScale::MONTHLY);

    EXPECT_EQ(result, nullptr);
}

TEST_F(TensorLogicEdgeCases, EmptyNetwork_AnalysisDoesntCrash)
{
    auto result = engine.analyze_flow_network();

    EXPECT_EQ(result.analysis_type, "flow_network");
}

TEST_F(TensorLogicEdgeCases, SingleEntity_ConsolidateWorks)
{
    engine.create_account("acc-001", "Test", 1, 4, 1);
    engine.import_account_data("acc-001", 0, 0, 0, 100.0, 0.0, 0.0);

    auto result = engine.consolidate_entities("acc-001");

    EXPECT_GE(result.data.size(), 1);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
