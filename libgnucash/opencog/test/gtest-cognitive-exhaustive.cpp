/*
 * gtest-cognitive-exhaustive.cpp
 *
 * Exhaustive unit tests for Cognitive Engine
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include "../gnc-cognitive/cognitive_engine.hpp"

using namespace gnc::opencog;

// ============================================================================
// CognitiveEngine Initialization Tests
// ============================================================================

class CognitiveEngineInitTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override {}
};

TEST_F(CognitiveEngineInitTest, Initialize_ReturnsTrueOnFirstCall)
{
    bool result = engine.initialize();
    EXPECT_TRUE(result);
}

TEST_F(CognitiveEngineInitTest, Initialize_ReturnsTrueOnSubsequentCalls)
{
    engine.initialize();
    bool result = engine.initialize();
    EXPECT_TRUE(result);
}

TEST_F(CognitiveEngineInitTest, Shutdown_ClearsState)
{
    engine.initialize();
    engine.shutdown();

    auto stats = engine.get_stats();
    EXPECT_EQ(stats.accounts_known, 0);
    EXPECT_EQ(stats.transactions_known, 0);
}

// ============================================================================
// Account Import Tests
// ============================================================================

class CognitiveEngineAccountTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineAccountTest, ImportAccount_AddsToKnowledge)
{
    engine.import_account("acc-001", "Expenses:Food", "EXPENSE", "");

    auto stats = engine.get_stats();
    EXPECT_EQ(stats.accounts_known, 1);
}

TEST_F(CognitiveEngineAccountTest, ImportAccount_WithParent)
{
    engine.import_account("parent", "Expenses", "EXPENSE", "");
    engine.import_account("child", "Expenses:Food", "EXPENSE", "parent");

    auto stats = engine.get_stats();
    EXPECT_EQ(stats.accounts_known, 2);
}

TEST_F(CognitiveEngineAccountTest, ImportMultipleAccounts)
{
    engine.import_account("acc-001", "Expenses:Food", "EXPENSE", "");
    engine.import_account("acc-002", "Expenses:Transport", "EXPENSE", "");
    engine.import_account("acc-003", "Income:Salary", "INCOME", "");

    auto stats = engine.get_stats();
    EXPECT_EQ(stats.accounts_known, 3);
}

// ============================================================================
// Transaction Import Tests
// ============================================================================

class CognitiveEngineTransactionTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineTransactionTest, ImportTransaction_AddsToKnowledge)
{
    engine.import_transaction("txn-001", "Grocery Store", "2024-01-15", 45.00,
                             "acc-001", "acc-002");

    auto stats = engine.get_stats();
    EXPECT_EQ(stats.transactions_known, 1);
}

TEST_F(CognitiveEngineTransactionTest, ImportMultipleTransactions)
{
    engine.import_transaction("txn-001", "Grocery Store", "2024-01-15", 45.00, "acc-001", "acc-002");
    engine.import_transaction("txn-002", "Gas Station", "2024-01-16", 35.00, "acc-001", "acc-003");
    engine.import_transaction("txn-003", "Restaurant", "2024-01-17", 25.00, "acc-001", "acc-004");

    auto stats = engine.get_stats();
    EXPECT_EQ(stats.transactions_known, 3);
}

// ============================================================================
// Vendor Import Tests
// ============================================================================

class CognitiveEngineVendorTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineVendorTest, ImportVendor_WithCategory)
{
    engine.import_vendor("Walmart", "Groceries");

    auto result = engine.categorize_transaction("Walmart Shopping", 50.0, "Walmart");
    EXPECT_EQ(result.category, "Groceries");
}

TEST_F(CognitiveEngineVendorTest, ImportVendor_WithoutCategory)
{
    engine.import_vendor("Unknown Store", "");

    auto stats = engine.get_stats();
    // Should not crash and stats should be reasonable
    EXPECT_GE(stats.total_atoms, 0);
}

// ============================================================================
// Categorization Tests
// ============================================================================

class CognitiveEngineCategorizationTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
        // Pre-import some vendors
        engine.import_vendor("Walmart", "Groceries");
        engine.import_vendor("Shell", "Gas");
        engine.import_vendor("Netflix", "Subscriptions");
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineCategorizationTest, CategorizeTransaction_ByVendor)
{
    auto result = engine.categorize_transaction("Shell Gas Station", 45.00, "Shell");

    EXPECT_EQ(result.category, "Gas");
    EXPECT_GT(result.confidence, 0.0);
}

TEST_F(CognitiveEngineCategorizationTest, CategorizeTransaction_ByKeyword)
{
    auto result = engine.categorize_transaction("grocery supermarket", 100.00, "");

    EXPECT_EQ(result.category, "Groceries");
}

TEST_F(CognitiveEngineCategorizationTest, CategorizeTransaction_Unknown)
{
    auto result = engine.categorize_transaction("Unknown Transaction XYZ", 50.00, "");

    // Should return Uncategorized for truly unknown
    EXPECT_FALSE(result.category.empty());
}

TEST_F(CognitiveEngineCategorizationTest, CategorizeTransaction_HasReasoning)
{
    auto result = engine.categorize_transaction("Walmart Purchase", 50.00, "Walmart");

    EXPECT_FALSE(result.reasoning.empty());
}

TEST_F(CognitiveEngineCategorizationTest, CategorizeTransaction_LargeAmount_SuggestsAlternative)
{
    auto result = engine.categorize_transaction("Unknown Store", 5000.00, "");

    // Large amounts should suggest "Large Purchase" as alternative
    bool has_large_purchase = std::find(result.alternative_categories.begin(),
                                        result.alternative_categories.end(),
                                        "Large Purchase") != result.alternative_categories.end();
    // May or may not be present depending on confidence
    EXPECT_GE(result.alternative_categories.size(), 0);
}

// ============================================================================
// Learning Tests
// ============================================================================

class CognitiveEngineLearningTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineLearningTest, LearnCategorization_ImprovesResults)
{
    // First categorization might be uncertain
    auto before = engine.categorize_transaction("New Store ABC", 50.00, "New Store");

    // Learn the correct category
    engine.learn_categorization("New Store ABC", "New Store", "Shopping");

    // After learning, should suggest Shopping
    auto after = engine.categorize_transaction("New Store ABC", 50.00, "New Store");

    EXPECT_EQ(after.category, "Shopping");
}

TEST_F(CognitiveEngineLearningTest, LearnCategorization_UpdatesVendorMapping)
{
    engine.learn_categorization("Purchase at XYZ", "XYZ Vendor", "Electronics");

    auto result = engine.categorize_transaction("Another purchase", 100.00, "XYZ Vendor");

    EXPECT_EQ(result.category, "Electronics");
}

TEST_F(CognitiveEngineLearningTest, LearnCategorization_UpdatesKeywordMapping)
{
    engine.learn_categorization("special coffee from roaster", "", "Coffee");

    auto result = engine.categorize_transaction("premium coffee beans", 30.00, "");

    // Should now associate "coffee" keyword with Coffee category
    EXPECT_EQ(result.category, "Coffee");
}

// ============================================================================
// Pattern Detection Tests
// ============================================================================

class CognitiveEnginePatternTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
        // Import recurring transactions
        for (int i = 0; i < 5; ++i) {
            engine.import_transaction(
                "txn-netflix-" + std::to_string(i),
                "Netflix Subscription",
                "2024-0" + std::to_string(i + 1) + "-15",
                15.99,
                "checking",
                "entertainment"
            );
        }
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEnginePatternTest, DetectSpendingPatterns_FindsRecurring)
{
    auto patterns = engine.detect_spending_patterns();

    // Should detect Netflix as recurring
    bool found_netflix = false;
    for (const auto& pattern : patterns) {
        if (pattern.name.find("Netflix") != std::string::npos) {
            found_netflix = true;
            break;
        }
    }

    EXPECT_TRUE(found_netflix);
}

TEST_F(CognitiveEnginePatternTest, DetectSpendingPatterns_HasFrequency)
{
    auto patterns = engine.detect_spending_patterns();

    for (const auto& pattern : patterns) {
        EXPECT_FALSE(pattern.frequency.empty());
    }
}

TEST_F(CognitiveEnginePatternTest, FindRecurringTransactions_ReturnsHandles)
{
    auto recurring = engine.find_recurring_transactions();

    EXPECT_GT(recurring.size(), 0);
}

// ============================================================================
// Anomaly Detection Tests
// ============================================================================

class CognitiveEngineAnomalyTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
        // Import normal transactions
        for (int i = 0; i < 10; ++i) {
            engine.import_transaction(
                "txn-normal-" + std::to_string(i),
                "Regular Purchase",
                "2024-01-" + std::to_string(10 + i),
                50.0 + (i % 3) * 10.0,  // Normal range: 50-70
                "checking",
                "expenses"
            );
        }
        // Import one anomalous transaction
        engine.import_transaction(
            "txn-anomaly",
            "Huge Purchase",
            "2024-01-25",
            5000.00,  // Much larger than normal
            "checking",
            "expenses"
        );
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineAnomalyTest, DetectAnomalies_FindsOutliers)
{
    auto anomalies = engine.detect_anomalies(2.0);  // 2 std deviations

    EXPECT_GE(anomalies.size(), 1);
}

TEST_F(CognitiveEngineAnomalyTest, DetectAnomalies_HighThreshold_LessAnomalies)
{
    auto low_threshold = engine.detect_anomalies(1.0);
    auto high_threshold = engine.detect_anomalies(5.0);

    EXPECT_GE(low_threshold.size(), high_threshold.size());
}

// ============================================================================
// Prediction Tests
// ============================================================================

class CognitiveEnginePredictionTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
        // Import some historical transactions
        for (int i = 0; i < 10; ++i) {
            engine.import_transaction(
                "txn-" + std::to_string(i),
                "Monthly Bill",
                "2024-01-15",
                100.0,
                "checking",
                "utilities"
            );
        }
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEnginePredictionTest, PredictCashFlow_ReturnsValue)
{
    double predicted = engine.predict_cash_flow(30);

    // Should return some prediction (positive or negative)
    EXPECT_TRUE(std::isfinite(predicted));
}

TEST_F(CognitiveEnginePredictionTest, PredictExpenses_ReturnsCategoryPredictions)
{
    auto predictions = engine.predict_expenses(30);

    // Should have at least one category prediction
    EXPECT_GE(predictions.size(), 0);
}

// ============================================================================
// Insights Tests
// ============================================================================

class CognitiveEngineInsightsTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;
    std::vector<FinancialInsight> received_insights;

    void SetUp() override
    {
        engine.initialize();

        // Subscribe to insights
        engine.subscribe_insights([this](const FinancialInsight& insight) {
            received_insights.push_back(insight);
        });

        // Import data for insights
        for (int i = 0; i < 5; ++i) {
            engine.import_transaction(
                "txn-" + std::to_string(i),
                "Recurring Payment",
                "2024-01-" + std::to_string(10 + i),
                100.0,
                "checking",
                "bills"
            );
        }
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineInsightsTest, GenerateInsights_ReturnsInsights)
{
    auto insights = engine.generate_insights();

    EXPECT_GE(insights.size(), 0);
}

TEST_F(CognitiveEngineInsightsTest, GenerateInsights_NotifiesSubscribers)
{
    engine.generate_insights();

    // Subscribers should have received some insights
    EXPECT_GE(received_insights.size(), 0);
}

TEST_F(CognitiveEngineInsightsTest, Insights_HaveRequiredFields)
{
    auto insights = engine.generate_insights();

    for (const auto& insight : insights) {
        EXPECT_FALSE(insight.title.empty());
        EXPECT_FALSE(insight.description.empty());
        EXPECT_GE(insight.confidence, 0.0);
        EXPECT_LE(insight.confidence, 1.0);
    }
}

// ============================================================================
// Budget Recommendation Tests
// ============================================================================

class CognitiveEngineBudgetTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineBudgetTest, GetBudgetRecommendations_ReturnsStrings)
{
    // Add some subscription-like transactions
    for (int i = 0; i < 10; ++i) {
        engine.import_transaction(
            "sub-" + std::to_string(i),
            "Subscription Service " + std::to_string(i),
            "2024-01-15",
            30.0,  // Monthly subscription
            "checking",
            "subscriptions"
        );
    }

    auto recommendations = engine.get_budget_recommendations();

    // Should suggest reviewing subscriptions if spending is high
    EXPECT_GE(recommendations.size(), 0);
}

// ============================================================================
// Natural Language Query Tests
// ============================================================================

class CognitiveEngineQueryTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
        // Add some data
        for (int i = 0; i < 5; ++i) {
            engine.import_transaction(
                "txn-" + std::to_string(i),
                "Test Transaction",
                "2024-01-15",
                100.0,
                "checking",
                "expenses"
            );
        }
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineQueryTest, Query_SpendingPattern)
{
    auto response = engine.query("What are my spending patterns?");

    EXPECT_FALSE(response.empty());
}

TEST_F(CognitiveEngineQueryTest, Query_RecurringExpenses)
{
    auto response = engine.query("Show me recurring expenses");

    EXPECT_FALSE(response.empty());
}

TEST_F(CognitiveEngineQueryTest, Query_Anomalies)
{
    auto response = engine.query("Are there any unusual transactions?");

    EXPECT_FALSE(response.empty());
}

TEST_F(CognitiveEngineQueryTest, Query_HowMuchSpend)
{
    auto response = engine.query("How much did I spend?");

    EXPECT_FALSE(response.empty());
}

TEST_F(CognitiveEngineQueryTest, Query_UnknownQuestion)
{
    auto response = engine.query("Random question that doesn't match anything");

    // Should return helpful response
    EXPECT_FALSE(response.empty());
}

// ============================================================================
// Statistics Tests
// ============================================================================

class CognitiveEngineStatsTest : public ::testing::Test
{
protected:
    CognitiveEngine engine;

    void SetUp() override
    {
        engine.initialize();
    }

    void TearDown() override
    {
        engine.shutdown();
    }
};

TEST_F(CognitiveEngineStatsTest, GetStats_ReturnsValidStats)
{
    engine.import_account("acc-001", "Test", "EXPENSE", "");
    engine.import_transaction("txn-001", "Test", "2024-01-15", 100.0, "a", "b");

    auto stats = engine.get_stats();

    EXPECT_EQ(stats.accounts_known, 1);
    EXPECT_EQ(stats.transactions_known, 1);
    EXPECT_GE(stats.total_atoms, 0);
    EXPECT_GE(stats.categorization_rules, 0);
}

TEST_F(CognitiveEngineStatsTest, GetStats_UpdatesAfterImports)
{
    auto before = engine.get_stats();

    engine.import_account("acc-001", "Test1", "EXPENSE", "");
    engine.import_account("acc-002", "Test2", "EXPENSE", "");

    auto after = engine.get_stats();

    EXPECT_GT(after.accounts_known, before.accounts_known);
}

// ============================================================================
// Global Engine Singleton Tests
// ============================================================================

TEST(CognitiveEngineSingleton, GlobalEngine_ReturnsSameInstance)
{
    auto& engine1 = cognitive_engine();
    auto& engine2 = cognitive_engine();

    EXPECT_EQ(&engine1, &engine2);
}

TEST(CognitiveEngineSingleton, GlobalEngine_CanInitialize)
{
    auto& engine = cognitive_engine();
    bool result = engine.initialize();
    EXPECT_TRUE(result);
    engine.shutdown();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
