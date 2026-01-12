/*
 * gtest-tensor-logic.cpp
 *
 * Unit tests for Tensor Logic Engine
 * Multi-Entity, Multi-Scale, Network-Aware Tensor Accounting
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include "../tensor-logic/tensor_logic_engine.hpp"

using namespace gnc::tensor_logic;
using namespace gnc::aten;

class TensorLogicEngineTest : public ::testing::Test
{
protected:
    TensorLogicEngine engine;
    
    void SetUp() override
    {
        engine.initialize();
    }
};

TEST_F(TensorLogicEngineTest, Initialize)
{
    EXPECT_TRUE(engine.is_initialized());
}

TEST_F(TensorLogicEngineTest, CreateAccount)
{
    auto account = engine.create_account("acc-001", "Expenses:Food", 1, 12, 1);
    
    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->guid(), "acc-001");
    EXPECT_EQ(account->name(), "Expenses:Food");
}

TEST_F(TensorLogicEngineTest, GetAccount)
{
    engine.create_account("acc-001", "Expenses:Food", 1, 12, 1);
    
    auto account = engine.get_account("acc-001");
    
    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->guid(), "acc-001");
}

TEST_F(TensorLogicEngineTest, ImportAccountData)
{
    engine.create_account("acc-001", "Expenses:Food", 1, 12, 1);
    
    bool result = engine.import_account_data("acc-001", 0, 0, 0, 1000.0, 500.0, 200.0);
    
    EXPECT_TRUE(result);
    
    auto account = engine.get_account("acc-001");
    ASSERT_NE(account, nullptr);
    
    auto balance = account->get_balance(0, 0, 0);
    EXPECT_EQ(balance, 1000.0);
}

TEST_F(TensorLogicEngineTest, MultiEntityConsolidation)
{
    // Create account with 3 entities
    engine.create_account("acc-001", "Expenses", 3, 12, 1);
    
    // Import data for each entity
    engine.import_account_data("acc-001", 0, 0, 0, 1000.0, 500.0, 200.0);
    engine.import_account_data("acc-001", 1, 0, 0, 2000.0, 800.0, 400.0);
    engine.import_account_data("acc-001", 2, 0, 0, 1500.0, 600.0, 300.0);
    
    auto consolidated = engine.consolidate_entities("acc-001");
    
    EXPECT_EQ(consolidated.size(), 12);  // 12 periods
    
    // First period should be sum: 1000 + 2000 + 1500 = 4500
    EXPECT_EQ(consolidated.at(0), 4500.0);
}

TEST_F(TensorLogicEngineTest, MultiScaleAnalysis)
{
    engine.create_account("acc-001", "Expenses", 1, 12, 1);
    
    // Import monthly data
    for (size_t month = 0; month < 12; ++month) {
        engine.import_account_data("acc-001", 0, month, 0, (month + 1) * 100.0, 0.0, 0.0);
    }
    
    auto analysis = engine.multi_scale_analysis("acc-001");
    
    EXPECT_EQ(analysis.analysis_type, "multi_scale");
}

TEST_F(TensorLogicEngineTest, AggregateToScale)
{
    engine.create_account("acc-001", "Expenses", 1, 12, 1);
    
    // Import monthly data
    for (size_t month = 0; month < 12; ++month) {
        engine.import_account_data("acc-001", 0, month, 0, 100.0, 0.0, 0.0);
    }
    
    auto quarterly = engine.aggregate_to_scale("acc-001", TimeScale::QUARTERLY);
    
    EXPECT_EQ(quarterly.size(), 4);  // 4 quarters
    
    // Each quarter should sum 3 months: 300.0
    EXPECT_EQ(quarterly.at(0), 300.0);
}

TEST_F(TensorLogicEngineTest, RecordTransaction)
{
    engine.record_transaction("checking", "expenses", 100.0, 0);
    
    // Verify transaction recorded (network should have edge)
    auto flow_analysis = engine.analyze_flow_network();
    
    EXPECT_GT(flow_analysis.insights.size(), 0);
}

TEST_F(TensorLogicEngineTest, AnalyzeFlowNetwork)
{
    // Record multiple transactions
    engine.record_transaction("income", "checking", 2000.0, 0);
    engine.record_transaction("checking", "expenses_food", 200.0, 0);
    engine.record_transaction("checking", "expenses_gas", 100.0, 0);
    
    auto analysis = engine.analyze_flow_network();
    
    EXPECT_EQ(analysis.analysis_type, "network_flow");
    EXPECT_GT(analysis.insights.size(), 0);
}

TEST_F(TensorLogicEngineTest, FindFlowPath)
{
    // Record transactions creating a path
    engine.record_transaction("income", "checking", 2000.0, 0);
    engine.record_transaction("checking", "savings", 500.0, 0);
    
    auto path = engine.find_flow_path("income", "savings");
    
    EXPECT_GT(path.size(), 0);
}

TEST_F(TensorLogicEngineTest, DetectCircularFlows)
{
    // Create circular flow: A -> B -> C -> A
    engine.record_transaction("account_a", "account_b", 100.0, 0);
    engine.record_transaction("account_b", "account_c", 100.0, 0);
    engine.record_transaction("account_c", "account_a", 100.0, 0);
    
    auto cycles = engine.detect_circular_flows();
    
    // Should detect at least one cycle
    EXPECT_GT(cycles.size(), 0);
}

TEST_F(TensorLogicEngineTest, DetectFlowAnomalies)
{
    // Record normal transactions
    for (int i = 0; i < 10; ++i) {
        engine.record_transaction("checking", "expenses", 100.0, i);
    }
    
    // Record anomalous transaction
    engine.record_transaction("checking", "expenses", 1000.0, 10);
    
    auto anomalies = engine.detect_flow_anomalies(2.0);
    
    // Should detect anomaly
    EXPECT_GT(anomalies.size(), 0);
}

TEST_F(TensorLogicEngineTest, ForecastBalance)
{
    engine.create_account("acc-001", "Checking", 1, 12, 1);
    
    // Import historical data with trend
    for (size_t month = 0; month < 12; ++month) {
        engine.import_account_data("acc-001", 0, month, 0, 1000.0 + month * 100.0, 0.0, 0.0);
    }
    
    auto forecast = engine.forecast_balance("acc-001", 3);
    
    EXPECT_EQ(forecast.account_guid, "acc-001");
    EXPECT_EQ(forecast.horizon, 3);
    EXPECT_EQ(forecast.predicted_values.size(), 3);
}

TEST_F(TensorLogicEngineTest, ForecastCashFlow)
{
    engine.create_account("acc-001", "Checking", 1, 12, 1);
    
    // Import historical data
    for (size_t month = 0; month < 12; ++month) {
        engine.import_account_data("acc-001", 0, month, 0, 1000.0, 500.0, 300.0);
    }
    
    auto forecast = engine.forecast_cash_flow("acc-001", 3);
    
    EXPECT_EQ(forecast.account_guid, "acc-001");
    EXPECT_EQ(forecast.horizon, 3);
}

TEST_F(TensorLogicEngineTest, FindSimilarAccounts)
{
    // Create accounts with similar patterns
    engine.create_account("acc-001", "Expenses:Food", 1, 12, 1);
    engine.create_account("acc-002", "Expenses:Groceries", 1, 12, 1);
    engine.create_account("acc-003", "Expenses:Transportation", 1, 12, 1);
    
    // Import similar data for food accounts
    for (size_t month = 0; month < 12; ++month) {
        engine.import_account_data("acc-001", 0, month, 0, 100.0, 50.0, 50.0);
        engine.import_account_data("acc-002", 0, month, 0, 110.0, 55.0, 55.0);
        engine.import_account_data("acc-003", 0, month, 0, 500.0, 250.0, 250.0);
    }
    
    auto similar = engine.find_similar_accounts("acc-001", 2);
    
    EXPECT_LE(similar.size(), 2);
}

TEST_F(TensorLogicEngineTest, ClusterAccounts)
{
    // Create multiple accounts
    for (int i = 0; i < 5; ++i) {
        std::string guid = "acc-" + std::to_string(i);
        engine.create_account(guid, "Account" + std::to_string(i), 1, 12, 1);
        
        // Import data
        for (size_t month = 0; month < 12; ++month) {
            engine.import_account_data(guid, 0, month, 0, 
                                      (i + 1) * 100.0, 0.0, 0.0);
        }
    }
    
    auto clusters = engine.cluster_accounts(2);
    
    EXPECT_EQ(clusters.size(), 2);
}

TEST_F(TensorLogicEngineTest, GetNetworkStats)
{
    engine.record_transaction("income", "checking", 2000.0, 0);
    engine.record_transaction("checking", "expenses", 500.0, 0);
    
    auto stats = engine.get_network_stats();
    
    EXPECT_GT(stats["num_nodes"], 0.0);
    EXPECT_GT(stats["num_edges"], 0.0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
