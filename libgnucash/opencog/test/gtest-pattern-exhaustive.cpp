/*
 * gtest-pattern-exhaustive.cpp
 *
 * Exhaustive unit tests for PatternMatcher and QueryBuilder
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include "../pattern/pattern_match.hpp"
#include "../atomspace/atomspace.hpp"

using namespace gnc::opencog;

// ============================================================================
// PatternMatcher Tests
// ============================================================================

class PatternMatcherTest : public ::testing::Test
{
protected:
    AtomSpace atomspace;

    void SetUp() override
    {
        // Create a small graph for testing
        auto expense = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Expense");
        auto food = atomspace.add_node(AtomTypes::CATEGORY_NODE, "Food");
        auto groceries = atomspace.add_node(AtomTypes::CATEGORY_NODE, "Groceries");
        auto transport = atomspace.add_node(AtomTypes::CATEGORY_NODE, "Transportation");

        atomspace.add_link(AtomTypes::INHERITANCE_LINK, food, expense);
        atomspace.add_link(AtomTypes::INHERITANCE_LINK, groceries, expense);
        atomspace.add_link(AtomTypes::INHERITANCE_LINK, transport, expense);

        auto account1 = atomspace.add_node(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
        auto account2 = atomspace.add_node(AtomTypes::ACCOUNT_NODE, "Expenses:Groceries");

        atomspace.add_link(AtomTypes::CATEGORIZATION_LINK, account1, food);
        atomspace.add_link(AtomTypes::CATEGORIZATION_LINK, account2, groceries);
    }
};

TEST_F(PatternMatcherTest, FindByType_FindsAllNodesOfType)
{
    PatternMatcher matcher(atomspace);

    auto categories = matcher.find_by_type(AtomTypes::CATEGORY_NODE);

    EXPECT_EQ(categories.size(), 3);  // Food, Groceries, Transportation
}

TEST_F(PatternMatcherTest, FindByType_ReturnsEmptyForNoMatches)
{
    PatternMatcher matcher(atomspace);

    auto results = matcher.find_by_type(AtomTypes::TRANSACTION_NODE);

    EXPECT_TRUE(results.empty());
}

TEST_F(PatternMatcherTest, FindNodes_FindsNodesByNamePrefix)
{
    PatternMatcher matcher(atomspace);

    auto results = matcher.find_nodes("Expenses:*");

    EXPECT_EQ(results.size(), 2);  // Expenses:Food, Expenses:Groceries
}

TEST_F(PatternMatcherTest, FindNodes_FindsNodesByNameSuffix)
{
    PatternMatcher matcher(atomspace);

    auto results = matcher.find_nodes("*Food");

    EXPECT_GE(results.size(), 1);
}

TEST_F(PatternMatcherTest, FindNodes_FindsNodesByExactName)
{
    PatternMatcher matcher(atomspace);

    auto results = matcher.find_nodes("Food");

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->name(), "Food");
}

TEST_F(PatternMatcherTest, FindLinksTo_FindsLinksPointingToAtom)
{
    PatternMatcher matcher(atomspace);

    auto expense = atomspace.get_node(AtomTypes::CONCEPT_NODE, "Expense");
    auto links = matcher.find_links_to(expense);

    EXPECT_EQ(links.size(), 3);  // 3 inheritance links to Expense
}

TEST_F(PatternMatcherTest, FindLinksContaining_FindsLinksWithAtom)
{
    PatternMatcher matcher(atomspace);

    auto food = atomspace.get_node(AtomTypes::CATEGORY_NODE, "Food");
    auto links = matcher.find_links_containing(food);

    EXPECT_GE(links.size(), 2);  // Inheritance and Categorization links
}

TEST_F(PatternMatcherTest, FindLinksOfType_FindsSpecificLinkType)
{
    PatternMatcher matcher(atomspace);

    auto results = matcher.find_by_type(AtomTypes::INHERITANCE_LINK);

    EXPECT_EQ(results.size(), 3);
}

// ============================================================================
// QueryBuilder Tests
// ============================================================================

class QueryBuilderTest : public ::testing::Test
{
protected:
    AtomSpace atomspace;

    void SetUp() override
    {
        atomspace.add_node(AtomTypes::CONCEPT_NODE, "Income");
        atomspace.add_node(AtomTypes::CONCEPT_NODE, "Expense");
        atomspace.add_node(AtomTypes::CATEGORY_NODE, "Salary");
        atomspace.add_node(AtomTypes::ACCOUNT_NODE, "Bank:Checking");
    }
};

TEST_F(QueryBuilderTest, OfType_FiltersResultsByType)
{
    QueryBuilder builder(atomspace);

    auto results = builder.of_type(AtomTypes::CONCEPT_NODE).execute();

    EXPECT_EQ(results.size(), 2);  // Income, Expense
}

TEST_F(QueryBuilderTest, NameLike_FiltersResultsByPattern)
{
    QueryBuilder builder(atomspace);

    auto results = builder.name_like("*:*").execute();

    EXPECT_EQ(results.size(), 1);  // Bank:Checking
}

TEST_F(QueryBuilderTest, Limit_RestrictsResultCount)
{
    QueryBuilder builder(atomspace);

    auto results = builder.of_type(AtomTypes::CONCEPT_NODE).limit(1).execute();

    EXPECT_EQ(results.size(), 1);
}

TEST_F(QueryBuilderTest, ChainedFilters_CombineCorrectly)
{
    QueryBuilder builder(atomspace);

    auto results = builder
        .of_type(AtomTypes::CONCEPT_NODE)
        .name_like("*ncome")
        .execute();

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->name(), "Income");
}

// ============================================================================
// Pattern Class Tests
// ============================================================================

class PatternTest : public ::testing::Test
{
protected:
    AtomSpace atomspace;
};

TEST_F(PatternTest, CreateVariable_CreatesVariableNode)
{
    Pattern pattern;

    auto var = pattern.variable("X");

    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->type(), AtomTypes::VARIABLE_NODE);
    EXPECT_EQ(var->name(), "X");
}

TEST_F(PatternTest, MultipleVariables_AreDistinct)
{
    Pattern pattern;

    auto var1 = pattern.variable("X");
    auto var2 = pattern.variable("Y");

    EXPECT_NE(var1->name(), var2->name());
}

TEST_F(PatternTest, SameVariableName_ReturnsSameVariable)
{
    Pattern pattern;

    auto var1 = pattern.variable("X");
    auto var2 = pattern.variable("X");

    EXPECT_EQ(var1, var2);
}

TEST_F(PatternTest, AddClause_BuildsPatternBody)
{
    Pattern pattern;

    auto var = pattern.variable("X");
    auto concept = create_node(AtomTypes::CONCEPT_NODE, "Test");
    pattern.clause(AtomTypes::INHERITANCE_LINK, {var, concept});

    auto clauses = pattern.clauses();

    EXPECT_EQ(clauses.size(), 1);
}

// ============================================================================
// Complex Pattern Matching Tests
// ============================================================================

class ComplexPatternMatchTest : public ::testing::Test
{
protected:
    AtomSpace atomspace;

    void SetUp() override
    {
        // Create a transaction graph
        auto txn1 = atomspace.add_node(AtomTypes::TRANSACTION_NODE, "txn-001");
        auto txn2 = atomspace.add_node(AtomTypes::TRANSACTION_NODE, "txn-002");
        auto txn3 = atomspace.add_node(AtomTypes::TRANSACTION_NODE, "txn-003");

        auto checking = atomspace.add_node(AtomTypes::ACCOUNT_NODE, "Checking");
        auto savings = atomspace.add_node(AtomTypes::ACCOUNT_NODE, "Savings");
        auto expenses = atomspace.add_node(AtomTypes::ACCOUNT_NODE, "Expenses");

        auto date1 = atomspace.add_node(AtomTypes::DATE_NODE, "2024-01-15");
        auto date2 = atomspace.add_node(AtomTypes::DATE_NODE, "2024-01-16");

        // Link transactions to accounts
        atomspace.add_link(AtomTypes::FLOW_LINK, txn1, checking);
        atomspace.add_link(AtomTypes::FLOW_LINK, txn2, checking);
        atomspace.add_link(AtomTypes::FLOW_LINK, txn2, savings);
        atomspace.add_link(AtomTypes::FLOW_LINK, txn3, expenses);

        // Link transactions to dates
        atomspace.add_link(AtomTypes::TEMPORAL_LINK, txn1, date1);
        atomspace.add_link(AtomTypes::TEMPORAL_LINK, txn2, date1);
        atomspace.add_link(AtomTypes::TEMPORAL_LINK, txn3, date2);
    }
};

TEST_F(ComplexPatternMatchTest, FindTransactionsOnDate)
{
    PatternMatcher matcher(atomspace);

    auto date = atomspace.get_node(AtomTypes::DATE_NODE, "2024-01-15");
    auto links = matcher.find_links_to(date);

    EXPECT_EQ(links.size(), 2);  // txn1 and txn2

    for (const auto& link : links) {
        EXPECT_EQ(link->type(), AtomTypes::TEMPORAL_LINK);
    }
}

TEST_F(ComplexPatternMatchTest, FindTransactionsForAccount)
{
    PatternMatcher matcher(atomspace);

    auto checking = atomspace.get_node(AtomTypes::ACCOUNT_NODE, "Checking");
    auto links = matcher.find_links_to(checking);

    EXPECT_EQ(links.size(), 2);  // txn1 and txn2 flow to checking
}

TEST_F(ComplexPatternMatchTest, FindAllTransactions)
{
    PatternMatcher matcher(atomspace);

    auto transactions = matcher.find_by_type(AtomTypes::TRANSACTION_NODE);

    EXPECT_EQ(transactions.size(), 3);
}

TEST_F(ComplexPatternMatchTest, FindAllFlowLinks)
{
    PatternMatcher matcher(atomspace);

    auto flows = matcher.find_by_type(AtomTypes::FLOW_LINK);

    EXPECT_EQ(flows.size(), 4);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

class PatternMatcherEdgeCases : public ::testing::Test
{
protected:
    AtomSpace atomspace;
};

TEST_F(PatternMatcherEdgeCases, EmptyAtomSpace_ReturnsEmptyResults)
{
    PatternMatcher matcher(atomspace);

    auto results = matcher.find_by_type(AtomTypes::CONCEPT_NODE);

    EXPECT_TRUE(results.empty());
}

TEST_F(PatternMatcherEdgeCases, NullAtom_HandlesSafely)
{
    PatternMatcher matcher(atomspace);

    auto results = matcher.find_links_to(nullptr);

    EXPECT_TRUE(results.empty());
}

TEST_F(PatternMatcherEdgeCases, WildcardOnlyPattern_MatchesAll)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "A");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "B");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "C");

    PatternMatcher matcher(atomspace);

    auto results = matcher.find_nodes("*");

    EXPECT_GE(results.size(), 3);
}

TEST_F(PatternMatcherEdgeCases, EmptyPattern_ReturnsEmpty)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");

    PatternMatcher matcher(atomspace);

    auto results = matcher.find_nodes("");

    EXPECT_TRUE(results.empty());
}

// ============================================================================
// Performance and Stress Tests
// ============================================================================

class PatternMatcherPerformance : public ::testing::Test
{
protected:
    AtomSpace atomspace;

    void SetUp() override
    {
        // Create many nodes
        for (int i = 0; i < 100; ++i) {
            atomspace.add_node(AtomTypes::CONCEPT_NODE, "Concept_" + std::to_string(i));
        }

        // Create some links
        auto nodes = atomspace.get_atoms_by_type(AtomTypes::CONCEPT_NODE);
        for (size_t i = 1; i < nodes.size() && i < 50; ++i) {
            atomspace.add_link(AtomTypes::INHERITANCE_LINK, nodes[i], nodes[0]);
        }
    }
};

TEST_F(PatternMatcherPerformance, FindManyNodes_CompletesQuickly)
{
    PatternMatcher matcher(atomspace);

    auto start = std::chrono::steady_clock::now();
    auto results = matcher.find_by_type(AtomTypes::CONCEPT_NODE);
    auto end = std::chrono::steady_clock::now();

    EXPECT_EQ(results.size(), 100);

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 1000);  // Should complete in under 1 second
}

TEST_F(PatternMatcherPerformance, FindLinksToPopularNode_CompletesQuickly)
{
    PatternMatcher matcher(atomspace);

    auto nodes = atomspace.get_atoms_by_type(AtomTypes::CONCEPT_NODE);
    auto popular = nodes[0];  // The root node with many incoming links

    auto start = std::chrono::steady_clock::now();
    auto links = matcher.find_links_to(popular);
    auto end = std::chrono::steady_clock::now();

    EXPECT_GE(links.size(), 49);

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 1000);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
