/*
 * gtest-atomspace-exhaustive.cpp
 *
 * Exhaustive unit tests for AtomSpace, Atom, TruthValue, and related classes
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include "../atomspace/atomspace.hpp"
#include "../atomspace/atom.hpp"
#include "../atomspace/truth_value.hpp"
#include "../atomspace/atom_types.hpp"

using namespace gnc::opencog;

// ============================================================================
// TruthValue Tests
// ============================================================================

class TruthValueTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(TruthValueTest, DefaultConstructor_CreatesDefaultValue)
{
    TruthValue tv;
    EXPECT_DOUBLE_EQ(tv.strength(), TruthValue::DEFAULT_STRENGTH);
    EXPECT_DOUBLE_EQ(tv.confidence(), TruthValue::DEFAULT_CONFIDENCE);
    EXPECT_TRUE(tv.is_default());
}

TEST_F(TruthValueTest, ParameterizedConstructor_SetsValues)
{
    TruthValue tv(0.75, 0.9);
    EXPECT_DOUBLE_EQ(tv.strength(), 0.75);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.9);
    EXPECT_FALSE(tv.is_default());
}

TEST_F(TruthValueTest, Constructor_ClampsStrengthBelow0)
{
    TruthValue tv(-0.5, 0.5);
    EXPECT_DOUBLE_EQ(tv.strength(), 0.0);
}

TEST_F(TruthValueTest, Constructor_ClampsStrengthAbove1)
{
    TruthValue tv(1.5, 0.5);
    EXPECT_DOUBLE_EQ(tv.strength(), 1.0);
}

TEST_F(TruthValueTest, Constructor_ClampsConfidenceBelow0)
{
    TruthValue tv(0.5, -0.5);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.0);
}

TEST_F(TruthValueTest, Constructor_ClampsConfidenceAbove1)
{
    TruthValue tv(0.5, 1.5);
    EXPECT_DOUBLE_EQ(tv.confidence(), 1.0);
}

TEST_F(TruthValueTest, FromCount_CalculatesConfidenceCorrectly)
{
    // confidence = count / (count + k)
    // With count=800 and k=800: confidence = 800 / 1600 = 0.5
    TruthValue tv = TruthValue::from_count(0.7, 800.0);
    EXPECT_DOUBLE_EQ(tv.strength(), 0.7);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.5);
}

TEST_F(TruthValueTest, FromCount_WithCustomK)
{
    // confidence = 100 / (100 + 100) = 0.5
    TruthValue tv = TruthValue::from_count(0.8, 100.0, 100.0);
    EXPECT_DOUBLE_EQ(tv.strength(), 0.8);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.5);
}

TEST_F(TruthValueTest, Count_CalculatesFromConfidence)
{
    TruthValue tv(0.7, 0.5);
    // count = k * confidence / (1 - confidence) = 800 * 0.5 / 0.5 = 800
    EXPECT_DOUBLE_EQ(tv.count(), 800.0);
}

TEST_F(TruthValueTest, Count_ReturnsInfinityForConfidence1)
{
    TruthValue tv(0.7, 1.0);
    EXPECT_TRUE(std::isinf(tv.count()));
}

TEST_F(TruthValueTest, SetStrength_ClampsValue)
{
    TruthValue tv;
    tv.set_strength(0.8);
    EXPECT_DOUBLE_EQ(tv.strength(), 0.8);

    tv.set_strength(-0.1);
    EXPECT_DOUBLE_EQ(tv.strength(), 0.0);

    tv.set_strength(1.2);
    EXPECT_DOUBLE_EQ(tv.strength(), 1.0);
}

TEST_F(TruthValueTest, SetConfidence_ClampsValue)
{
    TruthValue tv;
    tv.set_confidence(0.7);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.7);

    tv.set_confidence(-0.2);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.0);

    tv.set_confidence(1.3);
    EXPECT_DOUBLE_EQ(tv.confidence(), 1.0);
}

TEST_F(TruthValueTest, IsTrue_HighStrengthHighConfidence)
{
    TruthValue tv(0.8, 0.9);
    EXPECT_TRUE(tv.is_true());
}

TEST_F(TruthValueTest, IsTrue_LowStrength)
{
    TruthValue tv(0.3, 0.9);
    EXPECT_FALSE(tv.is_true());
}

TEST_F(TruthValueTest, IsTrue_LowConfidence)
{
    TruthValue tv(0.8, 0.3);
    EXPECT_FALSE(tv.is_true());
}

TEST_F(TruthValueTest, IsTrue_CustomThresholds)
{
    TruthValue tv(0.6, 0.4);
    EXPECT_TRUE(tv.is_true(0.5, 0.3));
    EXPECT_FALSE(tv.is_true(0.7, 0.3));
}

TEST_F(TruthValueTest, IsFalse_LowStrengthHighConfidence)
{
    TruthValue tv(0.2, 0.9);
    EXPECT_TRUE(tv.is_false());
}

TEST_F(TruthValueTest, IsFalse_HighStrength)
{
    TruthValue tv(0.8, 0.9);
    EXPECT_FALSE(tv.is_false());
}

TEST_F(TruthValueTest, IsFalse_LowConfidence)
{
    TruthValue tv(0.2, 0.3);
    EXPECT_FALSE(tv.is_false());
}

TEST_F(TruthValueTest, Merge_CombinesWeightedByConfidence)
{
    TruthValue tv1(0.8, 0.6);
    TruthValue tv2(0.4, 0.4);

    TruthValue merged = tv1.merge(tv2);
    // merged_strength = (0.8 * 0.6 + 0.4 * 0.4) / (0.6 + 0.4) = (0.48 + 0.16) / 1.0 = 0.64
    EXPECT_NEAR(merged.strength(), 0.64, 1e-9);
    // merged_confidence = max(0.6, 0.4) = 0.6
    EXPECT_DOUBLE_EQ(merged.confidence(), 0.6);
}

TEST_F(TruthValueTest, Merge_BothZeroConfidence)
{
    TruthValue tv1(0.8, 0.0);
    TruthValue tv2(0.4, 0.0);

    TruthValue merged = tv1.merge(tv2);
    EXPECT_TRUE(merged.is_default());
}

TEST_F(TruthValueTest, Revision_CombinesEvidence)
{
    TruthValue tv1 = TruthValue::from_count(0.8, 400.0);
    TruthValue tv2 = TruthValue::from_count(0.6, 400.0);

    TruthValue revised = tv1.revision(tv2);
    // new_strength = (0.8 * 400 + 0.6 * 400) / 800 = 560 / 800 = 0.7
    EXPECT_NEAR(revised.strength(), 0.7, 1e-9);
    // new_count = 400 + 400 = 800, so confidence = 800 / 1600 = 0.5
    EXPECT_NEAR(revised.confidence(), 0.5, 1e-9);
}

TEST_F(TruthValueTest, Revision_BothZeroCount)
{
    TruthValue tv1(0.5, 0.0);
    TruthValue tv2(0.5, 0.0);

    TruthValue revised = tv1.revision(tv2);
    EXPECT_TRUE(revised.is_default());
}

TEST_F(TruthValueTest, Negation_InvertsStrength)
{
    TruthValue tv(0.8, 0.9);
    TruthValue negated = tv.negation();

    EXPECT_DOUBLE_EQ(negated.strength(), 0.2);
    EXPECT_DOUBLE_EQ(negated.confidence(), 0.9);
}

TEST_F(TruthValueTest, Conjunction_ANDOperation)
{
    TruthValue tv1(0.8, 0.9);
    TruthValue tv2(0.6, 0.7);

    TruthValue result = tv1.conjunction(tv2);
    // strength = 0.8 * 0.6 = 0.48
    EXPECT_NEAR(result.strength(), 0.48, 1e-9);
    // confidence = min(0.9, 0.7) = 0.7
    EXPECT_DOUBLE_EQ(result.confidence(), 0.7);
}

TEST_F(TruthValueTest, Disjunction_OROperation)
{
    TruthValue tv1(0.6, 0.9);
    TruthValue tv2(0.4, 0.7);

    TruthValue result = tv1.disjunction(tv2);
    // strength = 0.6 + 0.4 - 0.6 * 0.4 = 1.0 - 0.24 = 0.76
    EXPECT_NEAR(result.strength(), 0.76, 1e-9);
    // confidence = min(0.9, 0.7) = 0.7
    EXPECT_DOUBLE_EQ(result.confidence(), 0.7);
}

TEST_F(TruthValueTest, ToString_FormatsCorrectly)
{
    TruthValue tv(0.8, 0.9);
    std::string str = tv.to_string();

    EXPECT_NE(str.find("stv"), std::string::npos);
    EXPECT_NE(str.find("0.8"), std::string::npos);
    EXPECT_NE(str.find("0.9"), std::string::npos);
}

TEST_F(TruthValueTest, Equality_SameValues)
{
    TruthValue tv1(0.8, 0.9);
    TruthValue tv2(0.8, 0.9);

    EXPECT_TRUE(tv1 == tv2);
    EXPECT_FALSE(tv1 != tv2);
}

TEST_F(TruthValueTest, Equality_DifferentStrength)
{
    TruthValue tv1(0.8, 0.9);
    TruthValue tv2(0.7, 0.9);

    EXPECT_FALSE(tv1 == tv2);
    EXPECT_TRUE(tv1 != tv2);
}

TEST_F(TruthValueTest, Equality_DifferentConfidence)
{
    TruthValue tv1(0.8, 0.9);
    TruthValue tv2(0.8, 0.8);

    EXPECT_FALSE(tv1 == tv2);
    EXPECT_TRUE(tv1 != tv2);
}

TEST_F(TruthValueTest, HelperFunctions_stv)
{
    TruthValue tv = stv(0.75, 0.85);
    EXPECT_DOUBLE_EQ(tv.strength(), 0.75);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.85);
}

TEST_F(TruthValueTest, HelperFunctions_tv_true)
{
    TruthValue tv = tv_true();
    EXPECT_DOUBLE_EQ(tv.strength(), 1.0);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.9);

    TruthValue tv2 = tv_true(0.8);
    EXPECT_DOUBLE_EQ(tv2.strength(), 1.0);
    EXPECT_DOUBLE_EQ(tv2.confidence(), 0.8);
}

TEST_F(TruthValueTest, HelperFunctions_tv_false)
{
    TruthValue tv = tv_false();
    EXPECT_DOUBLE_EQ(tv.strength(), 0.0);
    EXPECT_DOUBLE_EQ(tv.confidence(), 0.9);

    TruthValue tv2 = tv_false(0.7);
    EXPECT_DOUBLE_EQ(tv2.strength(), 0.0);
    EXPECT_DOUBLE_EQ(tv2.confidence(), 0.7);
}

// ============================================================================
// Node Tests
// ============================================================================

class NodeTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(NodeTest, Constructor_SetsTypeAndName)
{
    Node node(AtomTypes::CONCEPT_NODE, "TestNode");

    EXPECT_EQ(node.type(), AtomTypes::CONCEPT_NODE);
    EXPECT_EQ(node.name(), "TestNode");
}

TEST_F(NodeTest, IsNode_ReturnsTrue)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_TRUE(node.is_node());
}

TEST_F(NodeTest, IsLink_ReturnsFalse)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_FALSE(node.is_link());
}

TEST_F(NodeTest, TypeName_ReturnsCorrectString)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_STREQ(node.type_name(), "ConceptNode");
}

TEST_F(NodeTest, Outgoing_ReturnsEmptySet)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_TRUE(node.outgoing().empty());
}

TEST_F(NodeTest, Arity_ReturnsZero)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_EQ(node.arity(), 0);
}

TEST_F(NodeTest, OutgoingAtom_ReturnsNullptr)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_EQ(node.outgoing_atom(0), nullptr);
    EXPECT_EQ(node.outgoing_atom(100), nullptr);
}

TEST_F(NodeTest, UUID_IsUnique)
{
    Node node1(AtomTypes::CONCEPT_NODE, "Test1");
    Node node2(AtomTypes::CONCEPT_NODE, "Test2");

    EXPECT_NE(node1.uuid(), node2.uuid());
}

TEST_F(NodeTest, DefaultTruthValue_IsDefault)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_TRUE(node.truth_value().is_default());
}

TEST_F(NodeTest, SetTruthValue_ModifiesTruthValue)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    node.set_truth_value(stv(0.8, 0.9));

    EXPECT_DOUBLE_EQ(node.truth_value().strength(), 0.8);
    EXPECT_DOUBLE_EQ(node.truth_value().confidence(), 0.9);
}

TEST_F(NodeTest, AttentionValues_DefaultToZero)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_DOUBLE_EQ(node.sti(), 0.0);
    EXPECT_DOUBLE_EQ(node.lti(), 0.0);
}

TEST_F(NodeTest, SetSTI_ModifiesSTI)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    node.set_sti(50.0);
    EXPECT_DOUBLE_EQ(node.sti(), 50.0);
}

TEST_F(NodeTest, SetLTI_ModifiesLTI)
{
    Node node(AtomTypes::CONCEPT_NODE, "Test");
    node.set_lti(30.0);
    EXPECT_DOUBLE_EQ(node.lti(), 30.0);
}

TEST_F(NodeTest, ToString_ContainsTypeAndName)
{
    Node node(AtomTypes::CONCEPT_NODE, "TestNode");
    std::string str = node.to_string();

    EXPECT_NE(str.find("ConceptNode"), std::string::npos);
    EXPECT_NE(str.find("TestNode"), std::string::npos);
}

TEST_F(NodeTest, ToString_IncludesTruthValueIfNotDefault)
{
    Node node(AtomTypes::CONCEPT_NODE, "TestNode");
    node.set_truth_value(stv(0.8, 0.9));

    std::string str = node.to_string();
    EXPECT_NE(str.find("stv"), std::string::npos);
}

TEST_F(NodeTest, ToShortString_CompactFormat)
{
    Node node(AtomTypes::CONCEPT_NODE, "TestNode");
    std::string str = node.to_short_string();

    EXPECT_NE(str.find("ConceptNode"), std::string::npos);
    EXPECT_NE(str.find("TestNode"), std::string::npos);
}

TEST_F(NodeTest, Hash_ConsistentForSameNode)
{
    Node node(AtomTypes::CONCEPT_NODE, "TestNode");
    EXPECT_EQ(node.hash(), node.hash());
}

TEST_F(NodeTest, Hash_DifferentForDifferentNames)
{
    Node node1(AtomTypes::CONCEPT_NODE, "Node1");
    Node node2(AtomTypes::CONCEPT_NODE, "Node2");

    EXPECT_NE(node1.hash(), node2.hash());
}

TEST_F(NodeTest, Hash_DifferentForDifferentTypes)
{
    Node node1(AtomTypes::CONCEPT_NODE, "TestNode");
    Node node2(AtomTypes::PREDICATE_NODE, "TestNode");

    EXPECT_NE(node1.hash(), node2.hash());
}

TEST_F(NodeTest, Equality_SameTypeAndName)
{
    Node node1(AtomTypes::CONCEPT_NODE, "TestNode");
    Node node2(AtomTypes::CONCEPT_NODE, "TestNode");

    EXPECT_TRUE(node1 == node2);
    EXPECT_FALSE(node1 != node2);
}

TEST_F(NodeTest, Equality_DifferentName)
{
    Node node1(AtomTypes::CONCEPT_NODE, "Node1");
    Node node2(AtomTypes::CONCEPT_NODE, "Node2");

    EXPECT_FALSE(node1 == node2);
    EXPECT_TRUE(node1 != node2);
}

TEST_F(NodeTest, Equality_DifferentType)
{
    Node node1(AtomTypes::CONCEPT_NODE, "TestNode");
    Node node2(AtomTypes::PREDICATE_NODE, "TestNode");

    EXPECT_FALSE(node1 == node2);
}

TEST_F(NodeTest, AllGnuCashNodeTypes)
{
    // Test all GnuCash-specific node types
    Node account(AtomTypes::ACCOUNT_NODE, "Expenses:Food");
    Node transaction(AtomTypes::TRANSACTION_NODE, "txn-001");
    Node split(AtomTypes::SPLIT_NODE, "split-001");
    Node commodity(AtomTypes::COMMODITY_NODE, "USD");
    Node price(AtomTypes::PRICE_NODE, "price-001");
    Node vendor(AtomTypes::VENDOR_NODE, "Grocery Store");
    Node customer(AtomTypes::CUSTOMER_NODE, "John Doe");
    Node category(AtomTypes::CATEGORY_NODE, "Groceries");
    Node date(AtomTypes::DATE_NODE, "2024-01-15");
    Node amount(AtomTypes::AMOUNT_NODE, "100.00");

    EXPECT_EQ(account.type(), AtomTypes::ACCOUNT_NODE);
    EXPECT_EQ(transaction.type(), AtomTypes::TRANSACTION_NODE);
    EXPECT_EQ(split.type(), AtomTypes::SPLIT_NODE);
    EXPECT_EQ(commodity.type(), AtomTypes::COMMODITY_NODE);
    EXPECT_EQ(price.type(), AtomTypes::PRICE_NODE);
    EXPECT_EQ(vendor.type(), AtomTypes::VENDOR_NODE);
    EXPECT_EQ(customer.type(), AtomTypes::CUSTOMER_NODE);
    EXPECT_EQ(category.type(), AtomTypes::CATEGORY_NODE);
    EXPECT_EQ(date.type(), AtomTypes::DATE_NODE);
    EXPECT_EQ(amount.type(), AtomTypes::AMOUNT_NODE);
}

// ============================================================================
// Link Tests
// ============================================================================

class LinkTest : public ::testing::Test
{
protected:
    Handle node1;
    Handle node2;
    Handle node3;

    void SetUp() override
    {
        node1 = create_node(AtomTypes::CONCEPT_NODE, "Node1");
        node2 = create_node(AtomTypes::CONCEPT_NODE, "Node2");
        node3 = create_node(AtomTypes::CONCEPT_NODE, "Node3");
    }
};

TEST_F(LinkTest, Constructor_HandleSeq)
{
    HandleSeq outgoing = {node1, node2};
    Link link(AtomTypes::LIST_LINK, outgoing);

    EXPECT_EQ(link.type(), AtomTypes::LIST_LINK);
    EXPECT_EQ(link.arity(), 2);
}

TEST_F(LinkTest, Constructor_MoveHandleSeq)
{
    HandleSeq outgoing = {node1, node2};
    Link link(AtomTypes::LIST_LINK, std::move(outgoing));

    EXPECT_EQ(link.arity(), 2);
}

TEST_F(LinkTest, Constructor_Variadic)
{
    Link link(AtomTypes::LIST_LINK, node1, node2, node3);

    EXPECT_EQ(link.arity(), 3);
    EXPECT_EQ(link.outgoing_atom(0), node1);
    EXPECT_EQ(link.outgoing_atom(1), node2);
    EXPECT_EQ(link.outgoing_atom(2), node3);
}

TEST_F(LinkTest, IsNode_ReturnsFalse)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2});
    EXPECT_FALSE(link.is_node());
}

TEST_F(LinkTest, IsLink_ReturnsTrue)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2});
    EXPECT_TRUE(link.is_link());
}

TEST_F(LinkTest, Name_ReturnsEmptyString)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2});
    EXPECT_TRUE(link.name().empty());
}

TEST_F(LinkTest, Outgoing_ReturnsAllHandles)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2, node3});
    const HandleSeq& outgoing = link.outgoing();

    EXPECT_EQ(outgoing.size(), 3);
    EXPECT_EQ(outgoing[0], node1);
    EXPECT_EQ(outgoing[1], node2);
    EXPECT_EQ(outgoing[2], node3);
}

TEST_F(LinkTest, Arity_ReturnsCorrectCount)
{
    Link link0(AtomTypes::LIST_LINK, {});
    Link link1(AtomTypes::LIST_LINK, {node1});
    Link link3(AtomTypes::LIST_LINK, {node1, node2, node3});

    EXPECT_EQ(link0.arity(), 0);
    EXPECT_EQ(link1.arity(), 1);
    EXPECT_EQ(link3.arity(), 3);
}

TEST_F(LinkTest, OutgoingAtom_ValidIndex)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2});

    EXPECT_EQ(link.outgoing_atom(0), node1);
    EXPECT_EQ(link.outgoing_atom(1), node2);
}

TEST_F(LinkTest, OutgoingAtom_InvalidIndex)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2});

    EXPECT_EQ(link.outgoing_atom(2), nullptr);
    EXPECT_EQ(link.outgoing_atom(100), nullptr);
}

TEST_F(LinkTest, ToString_ContainsTypeAndOutgoing)
{
    Link link(AtomTypes::INHERITANCE_LINK, {node1, node2});
    std::string str = link.to_string();

    EXPECT_NE(str.find("InheritanceLink"), std::string::npos);
    EXPECT_NE(str.find("Node1"), std::string::npos);
    EXPECT_NE(str.find("Node2"), std::string::npos);
}

TEST_F(LinkTest, ToShortString_CompactFormat)
{
    Link link(AtomTypes::INHERITANCE_LINK, {node1, node2});
    std::string str = link.to_short_string();

    EXPECT_NE(str.find("InheritanceLink"), std::string::npos);
}

TEST_F(LinkTest, Hash_ConsistentForSameLink)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2});
    EXPECT_EQ(link.hash(), link.hash());
}

TEST_F(LinkTest, Hash_DifferentForDifferentOutgoing)
{
    Link link1(AtomTypes::LIST_LINK, {node1, node2});
    Link link2(AtomTypes::LIST_LINK, {node2, node1});

    EXPECT_NE(link1.hash(), link2.hash());
}

TEST_F(LinkTest, Hash_DifferentForDifferentTypes)
{
    Link link1(AtomTypes::LIST_LINK, {node1, node2});
    Link link2(AtomTypes::SET_LINK, {node1, node2});

    EXPECT_NE(link1.hash(), link2.hash());
}

TEST_F(LinkTest, Equality_SameTypeAndOutgoing)
{
    Link link1(AtomTypes::LIST_LINK, {node1, node2});
    Link link2(AtomTypes::LIST_LINK, {node1, node2});

    EXPECT_TRUE(link1 == link2);
    EXPECT_FALSE(link1 != link2);
}

TEST_F(LinkTest, Equality_DifferentOrder)
{
    Link link1(AtomTypes::LIST_LINK, {node1, node2});
    Link link2(AtomTypes::LIST_LINK, {node2, node1});

    EXPECT_FALSE(link1 == link2);
}

TEST_F(LinkTest, Equality_DifferentType)
{
    Link link1(AtomTypes::LIST_LINK, {node1, node2});
    Link link2(AtomTypes::SET_LINK, {node1, node2});

    EXPECT_FALSE(link1 == link2);
}

TEST_F(LinkTest, Equality_DifferentArity)
{
    Link link1(AtomTypes::LIST_LINK, {node1, node2});
    Link link2(AtomTypes::LIST_LINK, {node1, node2, node3});

    EXPECT_FALSE(link1 == link2);
}

TEST_F(LinkTest, Equality_NodeVsLink)
{
    Link link(AtomTypes::LIST_LINK, {node1, node2});
    Node node(AtomTypes::LIST_LINK, "SameName");

    EXPECT_FALSE(link == node);
}

TEST_F(LinkTest, NestedLinks)
{
    auto inner_link = create_link(AtomTypes::LIST_LINK, node1, node2);
    Link outer_link(AtomTypes::EVALUATION_LINK, {inner_link, node3});

    EXPECT_EQ(outer_link.arity(), 2);
    EXPECT_TRUE(outer_link.outgoing_atom(0)->is_link());
    EXPECT_TRUE(outer_link.outgoing_atom(1)->is_node());
}

TEST_F(LinkTest, AllGnuCashLinkTypes)
{
    auto account1 = create_node(AtomTypes::ACCOUNT_NODE, "Income");
    auto account2 = create_node(AtomTypes::ACCOUNT_NODE, "Expenses");
    auto transaction = create_node(AtomTypes::TRANSACTION_NODE, "txn-001");
    auto vendor = create_node(AtomTypes::VENDOR_NODE, "Shop");
    auto category = create_node(AtomTypes::CATEGORY_NODE, "Food");

    Link txn_link(AtomTypes::TRANSACTION_LINK, {transaction, account1, account2});
    Link hierarchy_link(AtomTypes::ACCOUNT_HIERARCHY_LINK, {account1, account2});
    Link categorization_link(AtomTypes::CATEGORIZATION_LINK, {vendor, category});
    Link flow_link(AtomTypes::FLOW_LINK, {account1, account2});
    Link pattern_link(AtomTypes::PATTERN_LINK, {transaction});
    Link prediction_link(AtomTypes::PREDICTION_LINK, {transaction});
    Link anomaly_link(AtomTypes::ANOMALY_LINK, {transaction});

    EXPECT_EQ(txn_link.type(), AtomTypes::TRANSACTION_LINK);
    EXPECT_EQ(hierarchy_link.type(), AtomTypes::ACCOUNT_HIERARCHY_LINK);
    EXPECT_EQ(categorization_link.type(), AtomTypes::CATEGORIZATION_LINK);
    EXPECT_EQ(flow_link.type(), AtomTypes::FLOW_LINK);
    EXPECT_EQ(pattern_link.type(), AtomTypes::PATTERN_LINK);
    EXPECT_EQ(prediction_link.type(), AtomTypes::PREDICTION_LINK);
    EXPECT_EQ(anomaly_link.type(), AtomTypes::ANOMALY_LINK);
}

// ============================================================================
// Factory Function Tests
// ============================================================================

class FactoryFunctionTest : public ::testing::Test {};

TEST_F(FactoryFunctionTest, CreateNode_ReturnsValidHandle)
{
    Handle node = create_node(AtomTypes::CONCEPT_NODE, "TestNode");

    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->is_node());
    EXPECT_EQ(node->type(), AtomTypes::CONCEPT_NODE);
    EXPECT_EQ(node->name(), "TestNode");
}

TEST_F(FactoryFunctionTest, CreateLink_HandleSeq_ReturnsValidHandle)
{
    auto node1 = create_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = create_node(AtomTypes::CONCEPT_NODE, "Node2");

    Handle link = create_link(AtomTypes::LIST_LINK, HandleSeq{node1, node2});

    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_link());
    EXPECT_EQ(link->arity(), 2);
}

TEST_F(FactoryFunctionTest, CreateLink_Variadic_ReturnsValidHandle)
{
    auto node1 = create_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = create_node(AtomTypes::CONCEPT_NODE, "Node2");

    Handle link = create_link(AtomTypes::LIST_LINK, node1, node2);

    ASSERT_NE(link, nullptr);
    EXPECT_TRUE(link->is_link());
    EXPECT_EQ(link->arity(), 2);
}

// ============================================================================
// AtomSpace Tests
// ============================================================================

class AtomSpaceExhaustiveTest : public ::testing::Test
{
protected:
    AtomSpace atomspace;

    void SetUp() override {}
};

TEST_F(AtomSpaceExhaustiveTest, AddNode_ReturnsHandle)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(atomspace.size(), 1);
}

TEST_F(AtomSpaceExhaustiveTest, AddNode_DuplicateReturnsSame)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");

    EXPECT_EQ(node1, node2);
    EXPECT_EQ(atomspace.size(), 1);
}

TEST_F(AtomSpaceExhaustiveTest, AddNode_DifferentNamesAreDifferent)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test2");

    EXPECT_NE(node1, node2);
    EXPECT_EQ(atomspace.size(), 2);
}

TEST_F(AtomSpaceExhaustiveTest, AddNode_DifferentTypesAreDifferent)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    auto node2 = atomspace.add_node(AtomTypes::PREDICATE_NODE, "Test");

    EXPECT_NE(node1, node2);
    EXPECT_EQ(atomspace.size(), 2);
}

TEST_F(AtomSpaceExhaustiveTest, AddLink_HandleSeq)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");

    auto link = atomspace.add_link(AtomTypes::LIST_LINK, HandleSeq{node1, node2});

    ASSERT_NE(link, nullptr);
    EXPECT_EQ(atomspace.size(), 3);  // 2 nodes + 1 link
}

TEST_F(AtomSpaceExhaustiveTest, AddLink_Variadic)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");

    auto link = atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);

    ASSERT_NE(link, nullptr);
    EXPECT_EQ(link->arity(), 2);
}

TEST_F(AtomSpaceExhaustiveTest, AddLink_DuplicateReturnsSame)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");

    auto link1 = atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);
    auto link2 = atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);

    EXPECT_EQ(link1, link2);
    EXPECT_EQ(atomspace.size(), 3);
}

TEST_F(AtomSpaceExhaustiveTest, AddAtom_Node)
{
    auto node = create_node(AtomTypes::CONCEPT_NODE, "Test");
    auto added = atomspace.add_atom(node);

    ASSERT_NE(added, nullptr);
    EXPECT_EQ(atomspace.size(), 1);
}

TEST_F(AtomSpaceExhaustiveTest, AddAtom_Link)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    auto link = create_link(AtomTypes::LIST_LINK, node1, node2);

    auto added = atomspace.add_atom(link);

    ASSERT_NE(added, nullptr);
    EXPECT_EQ(atomspace.size(), 3);
}

TEST_F(AtomSpaceExhaustiveTest, AddAtom_Nullptr)
{
    auto result = atomspace.add_atom(nullptr);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(atomspace.size(), 0);
}

TEST_F(AtomSpaceExhaustiveTest, RemoveAtom_Node)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_EQ(atomspace.size(), 1);

    bool removed = atomspace.remove_atom(node);

    EXPECT_TRUE(removed);
    EXPECT_EQ(atomspace.size(), 0);
}

TEST_F(AtomSpaceExhaustiveTest, RemoveAtom_Link)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    auto link = atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);

    EXPECT_EQ(atomspace.size(), 3);

    bool removed = atomspace.remove_atom(link);

    EXPECT_TRUE(removed);
    EXPECT_EQ(atomspace.size(), 2);  // Nodes remain
}

TEST_F(AtomSpaceExhaustiveTest, RemoveAtom_Nullptr)
{
    bool removed = atomspace.remove_atom(nullptr);
    EXPECT_FALSE(removed);
}

TEST_F(AtomSpaceExhaustiveTest, RemoveAtom_NotInAtomSpace)
{
    auto node = create_node(AtomTypes::CONCEPT_NODE, "NotAdded");
    bool removed = atomspace.remove_atom(node);
    EXPECT_FALSE(removed);
}

TEST_F(AtomSpaceExhaustiveTest, RemoveAtom_ClearsFromTypeIndex)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    atomspace.remove_atom(node);

    auto found = atomspace.get_atoms_by_type(AtomTypes::CONCEPT_NODE);
    EXPECT_TRUE(found.empty());
}

TEST_F(AtomSpaceExhaustiveTest, RemoveAtom_ClearsFromUUIDIndex)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    UUID uuid = node->uuid();
    atomspace.remove_atom(node);

    auto found = atomspace.get_atom(uuid);
    EXPECT_EQ(found, nullptr);
}

TEST_F(AtomSpaceExhaustiveTest, RemoveAtom_ClearsFromIncomingSet)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    auto link = atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);

    atomspace.remove_atom(link);

    auto incoming = atomspace.get_incoming(node1);
    EXPECT_TRUE(incoming.empty());
}

TEST_F(AtomSpaceExhaustiveTest, GetAtom_ByUUID)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    UUID uuid = node->uuid();

    auto found = atomspace.get_atom(uuid);

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found, node);
}

TEST_F(AtomSpaceExhaustiveTest, GetAtom_InvalidUUID)
{
    auto found = atomspace.get_atom(999999);
    EXPECT_EQ(found, nullptr);
}

TEST_F(AtomSpaceExhaustiveTest, GetNode_Exists)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");

    auto found = atomspace.get_node(AtomTypes::CONCEPT_NODE, "Test");

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name(), "Test");
}

TEST_F(AtomSpaceExhaustiveTest, GetNode_NotExists)
{
    auto found = atomspace.get_node(AtomTypes::CONCEPT_NODE, "NotExists");
    EXPECT_EQ(found, nullptr);
}

TEST_F(AtomSpaceExhaustiveTest, GetNode_WrongType)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");

    auto found = atomspace.get_node(AtomTypes::PREDICATE_NODE, "Test");
    EXPECT_EQ(found, nullptr);
}

TEST_F(AtomSpaceExhaustiveTest, GetLink_Exists)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);

    auto found = atomspace.get_link(AtomTypes::LIST_LINK, HandleSeq{node1, node2});

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->arity(), 2);
}

TEST_F(AtomSpaceExhaustiveTest, GetLink_NotExists)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");

    auto found = atomspace.get_link(AtomTypes::LIST_LINK, HandleSeq{node1, node2});
    EXPECT_EQ(found, nullptr);
}

TEST_F(AtomSpaceExhaustiveTest, GetAtomsByType_ReturnsAllOfType)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Concept1");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Concept2");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Concept3");
    atomspace.add_node(AtomTypes::PREDICATE_NODE, "Predicate1");

    auto concepts = atomspace.get_atoms_by_type(AtomTypes::CONCEPT_NODE);

    EXPECT_EQ(concepts.size(), 3);
}

TEST_F(AtomSpaceExhaustiveTest, GetAtomsByType_EmptyForUnusedType)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");

    auto predicates = atomspace.get_atoms_by_type(AtomTypes::PREDICATE_NODE);
    EXPECT_TRUE(predicates.empty());
}

TEST_F(AtomSpaceExhaustiveTest, GetAtomsByType_IncludesLinks)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);
    atomspace.add_link(AtomTypes::LIST_LINK, node2, node1);

    auto links = atomspace.get_atoms_by_type(AtomTypes::LIST_LINK);
    EXPECT_EQ(links.size(), 2);
}

TEST_F(AtomSpaceExhaustiveTest, GetIncoming_SingleLink)
{
    auto animal = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Animal");
    auto dog = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Dog");
    atomspace.add_link(AtomTypes::INHERITANCE_LINK, dog, animal);

    auto incoming = atomspace.get_incoming(animal);

    EXPECT_EQ(incoming.size(), 1);
}

TEST_F(AtomSpaceExhaustiveTest, GetIncoming_MultipleLinks)
{
    auto animal = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Animal");
    auto dog = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Dog");
    auto cat = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Cat");
    auto bird = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Bird");

    atomspace.add_link(AtomTypes::INHERITANCE_LINK, dog, animal);
    atomspace.add_link(AtomTypes::INHERITANCE_LINK, cat, animal);
    atomspace.add_link(AtomTypes::INHERITANCE_LINK, bird, animal);

    auto incoming = atomspace.get_incoming(animal);
    EXPECT_EQ(incoming.size(), 3);
}

TEST_F(AtomSpaceExhaustiveTest, GetIncoming_NoLinks)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Isolated");

    auto incoming = atomspace.get_incoming(node);
    EXPECT_TRUE(incoming.empty());
}

TEST_F(AtomSpaceExhaustiveTest, GetIncoming_Nullptr)
{
    auto incoming = atomspace.get_incoming(nullptr);
    EXPECT_TRUE(incoming.empty());
}

TEST_F(AtomSpaceExhaustiveTest, GetIncomingByType_FiltersByType)
{
    auto animal = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Animal");
    auto dog = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Dog");
    auto similar = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Similar");

    atomspace.add_link(AtomTypes::INHERITANCE_LINK, dog, animal);
    atomspace.add_link(AtomTypes::SIMILARITY_LINK, similar, animal);

    auto inheritance_incoming = atomspace.get_incoming_by_type(animal, AtomTypes::INHERITANCE_LINK);
    auto similarity_incoming = atomspace.get_incoming_by_type(animal, AtomTypes::SIMILARITY_LINK);

    EXPECT_EQ(inheritance_incoming.size(), 1);
    EXPECT_EQ(similarity_incoming.size(), 1);
}

TEST_F(AtomSpaceExhaustiveTest, FindAtoms_ByPredicate)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Apple");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Apricot");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Banana");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Avocado");

    auto found = atomspace.find_atoms([](const Handle& h) {
        return h->is_node() && h->name().front() == 'A';
    });

    EXPECT_EQ(found.size(), 3);  // Apple, Apricot, Avocado
}

TEST_F(AtomSpaceExhaustiveTest, FindAtoms_NoMatches)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");

    auto found = atomspace.find_atoms([](const Handle& h) {
        return false;
    });

    EXPECT_TRUE(found.empty());
}

TEST_F(AtomSpaceExhaustiveTest, Size_Total)
{
    EXPECT_EQ(atomspace.size(), 0);

    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    EXPECT_EQ(atomspace.size(), 1);

    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    EXPECT_EQ(atomspace.size(), 2);
}

TEST_F(AtomSpaceExhaustiveTest, Size_ByType)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Concept1");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Concept2");
    atomspace.add_node(AtomTypes::PREDICATE_NODE, "Predicate1");

    EXPECT_EQ(atomspace.size(AtomTypes::CONCEPT_NODE), 2);
    EXPECT_EQ(atomspace.size(AtomTypes::PREDICATE_NODE), 1);
    EXPECT_EQ(atomspace.size(AtomTypes::SCHEMA_NODE), 0);
}

TEST_F(AtomSpaceExhaustiveTest, Contains_Exists)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    EXPECT_TRUE(atomspace.contains(node));
}

TEST_F(AtomSpaceExhaustiveTest, Contains_NotExists)
{
    auto node = create_node(AtomTypes::CONCEPT_NODE, "NotAdded");
    EXPECT_FALSE(atomspace.contains(node));
}

TEST_F(AtomSpaceExhaustiveTest, Contains_Nullptr)
{
    EXPECT_FALSE(atomspace.contains(nullptr));
}

TEST_F(AtomSpaceExhaustiveTest, Clear_RemovesAllAtoms)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    atomspace.add_node(AtomTypes::PREDICATE_NODE, "Predicate1");

    EXPECT_EQ(atomspace.size(), 3);

    atomspace.clear();

    EXPECT_EQ(atomspace.size(), 0);
}

TEST_F(AtomSpaceExhaustiveTest, Clear_ClearsAllIndexes)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test");
    UUID uuid = node->uuid();

    atomspace.clear();

    EXPECT_EQ(atomspace.get_atom(uuid), nullptr);
    EXPECT_EQ(atomspace.get_node(AtomTypes::CONCEPT_NODE, "Test"), nullptr);
    EXPECT_TRUE(atomspace.get_atoms_by_type(AtomTypes::CONCEPT_NODE).empty());
}

TEST_F(AtomSpaceExhaustiveTest, ForEach_VisitsAllAtoms)
{
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    atomspace.add_node(AtomTypes::PREDICATE_NODE, "Predicate1");

    std::set<std::string> names;
    atomspace.for_each([&names](const Handle& h) {
        if (h->is_node())
            names.insert(h->name());
    });

    EXPECT_EQ(names.size(), 3);
    EXPECT_TRUE(names.count("Node1"));
    EXPECT_TRUE(names.count("Node2"));
    EXPECT_TRUE(names.count("Predicate1"));
}

TEST_F(AtomSpaceExhaustiveTest, GetStats_ReturnsCorrectCounts)
{
    auto node1 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node1");
    auto node2 = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node2");
    auto node3 = atomspace.add_node(AtomTypes::PREDICATE_NODE, "Predicate1");
    atomspace.add_link(AtomTypes::LIST_LINK, node1, node2);
    atomspace.add_link(AtomTypes::INHERITANCE_LINK, node1, node3);

    auto stats = atomspace.get_stats();

    EXPECT_EQ(stats.total_atoms, 5);
    EXPECT_EQ(stats.total_nodes, 3);
    EXPECT_EQ(stats.total_links, 2);
    EXPECT_EQ(stats.by_type[AtomTypes::CONCEPT_NODE], 2);
    EXPECT_EQ(stats.by_type[AtomTypes::PREDICATE_NODE], 1);
    EXPECT_EQ(stats.by_type[AtomTypes::LIST_LINK], 1);
    EXPECT_EQ(stats.by_type[AtomTypes::INHERITANCE_LINK], 1);
}

// ============================================================================
// Concurrency Tests
// ============================================================================

class AtomSpaceConcurrencyTest : public ::testing::Test
{
protected:
    AtomSpace atomspace;
};

TEST_F(AtomSpaceConcurrencyTest, ConcurrentAddNodes)
{
    const int num_threads = 8;
    const int nodes_per_thread = 100;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, nodes_per_thread]() {
            for (int i = 0; i < nodes_per_thread; ++i) {
                std::string name = "Thread" + std::to_string(t) + "_Node" + std::to_string(i);
                atomspace.add_node(AtomTypes::CONCEPT_NODE, name);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(atomspace.size(), num_threads * nodes_per_thread);
}

TEST_F(AtomSpaceConcurrencyTest, ConcurrentAddDuplicates)
{
    const int num_threads = 8;
    const int iterations = 100;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, iterations]() {
            for (int i = 0; i < iterations; ++i) {
                atomspace.add_node(AtomTypes::CONCEPT_NODE, "SharedNode");
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(atomspace.size(), 1);
}

TEST_F(AtomSpaceConcurrencyTest, ConcurrentReadWrite)
{
    // Pre-populate some data
    for (int i = 0; i < 100; ++i) {
        atomspace.add_node(AtomTypes::CONCEPT_NODE, "Initial" + std::to_string(i));
    }

    std::atomic<int> read_count{0};
    std::atomic<int> write_count{0};

    std::vector<std::thread> threads;

    // Reader threads
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([this, &read_count]() {
            for (int i = 0; i < 100; ++i) {
                auto atoms = atomspace.get_atoms_by_type(AtomTypes::CONCEPT_NODE);
                read_count += atoms.size();
            }
        });
    }

    // Writer threads
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([this, t, &write_count]() {
            for (int i = 0; i < 50; ++i) {
                atomspace.add_node(AtomTypes::CONCEPT_NODE, "New" + std::to_string(t) + "_" + std::to_string(i));
                write_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(atomspace.size(), 300);  // 100 initial + 4*50 new
    EXPECT_GT(read_count.load(), 0);
}

TEST_F(AtomSpaceConcurrencyTest, ConcurrentAddRemove)
{
    std::atomic<bool> stop{false};

    // Adder thread
    std::thread adder([this, &stop]() {
        int i = 0;
        while (!stop) {
            atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node" + std::to_string(i++));
        }
    });

    // Remover thread
    std::thread remover([this, &stop]() {
        while (!stop) {
            auto atoms = atomspace.get_atoms_by_type(AtomTypes::CONCEPT_NODE);
            for (const auto& atom : atoms) {
                atomspace.remove_atom(atom);
            }
        }
    });

    // Let it run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop = true;

    adder.join();
    remover.join();

    // Just verify no crashes occurred
    SUCCEED();
}

// ============================================================================
// AtomType Helper Function Tests
// ============================================================================

class AtomTypeTest : public ::testing::Test {};

TEST_F(AtomTypeTest, AtomTypeName_CoreTypes)
{
    EXPECT_STREQ(atom_type_name(AtomTypes::ATOM), "Atom");
    EXPECT_STREQ(atom_type_name(AtomTypes::NODE), "Node");
    EXPECT_STREQ(atom_type_name(AtomTypes::LINK), "Link");
}

TEST_F(AtomTypeTest, AtomTypeName_NodeTypes)
{
    EXPECT_STREQ(atom_type_name(AtomTypes::CONCEPT_NODE), "ConceptNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::PREDICATE_NODE), "PredicateNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::SCHEMA_NODE), "SchemaNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::VARIABLE_NODE), "VariableNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::NUMBER_NODE), "NumberNode");
}

TEST_F(AtomTypeTest, AtomTypeName_GnuCashNodeTypes)
{
    EXPECT_STREQ(atom_type_name(AtomTypes::ACCOUNT_NODE), "AccountNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::TRANSACTION_NODE), "TransactionNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::SPLIT_NODE), "SplitNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::VENDOR_NODE), "VendorNode");
    EXPECT_STREQ(atom_type_name(AtomTypes::CATEGORY_NODE), "CategoryNode");
}

TEST_F(AtomTypeTest, AtomTypeName_LinkTypes)
{
    EXPECT_STREQ(atom_type_name(AtomTypes::LIST_LINK), "ListLink");
    EXPECT_STREQ(atom_type_name(AtomTypes::SET_LINK), "SetLink");
    EXPECT_STREQ(atom_type_name(AtomTypes::INHERITANCE_LINK), "InheritanceLink");
    EXPECT_STREQ(atom_type_name(AtomTypes::EVALUATION_LINK), "EvaluationLink");
}

TEST_F(AtomTypeTest, AtomTypeName_GnuCashLinkTypes)
{
    EXPECT_STREQ(atom_type_name(AtomTypes::TRANSACTION_LINK), "TransactionLink");
    EXPECT_STREQ(atom_type_name(AtomTypes::CATEGORIZATION_LINK), "CategorizationLink");
    EXPECT_STREQ(atom_type_name(AtomTypes::FLOW_LINK), "FlowLink");
    EXPECT_STREQ(atom_type_name(AtomTypes::PATTERN_LINK), "PatternLink");
    EXPECT_STREQ(atom_type_name(AtomTypes::ANOMALY_LINK), "AnomalyLink");
}

TEST_F(AtomTypeTest, AtomTypeName_UnknownType)
{
    EXPECT_STREQ(atom_type_name(999), "UnknownType");
}

TEST_F(AtomTypeTest, IsNodeType_Nodes)
{
    EXPECT_TRUE(is_node_type(AtomTypes::NODE));
    EXPECT_TRUE(is_node_type(AtomTypes::CONCEPT_NODE));
    EXPECT_TRUE(is_node_type(AtomTypes::PREDICATE_NODE));
    EXPECT_TRUE(is_node_type(AtomTypes::ACCOUNT_NODE));
}

TEST_F(AtomTypeTest, IsNodeType_Links)
{
    EXPECT_FALSE(is_node_type(AtomTypes::LIST_LINK));
    EXPECT_FALSE(is_node_type(AtomTypes::INHERITANCE_LINK));
    EXPECT_FALSE(is_node_type(AtomTypes::TRANSACTION_LINK));
}

TEST_F(AtomTypeTest, IsLinkType_Links)
{
    EXPECT_TRUE(is_link_type(AtomTypes::LIST_LINK));
    EXPECT_TRUE(is_link_type(AtomTypes::SET_LINK));
    EXPECT_TRUE(is_link_type(AtomTypes::INHERITANCE_LINK));
    EXPECT_TRUE(is_link_type(AtomTypes::TRANSACTION_LINK));
}

TEST_F(AtomTypeTest, IsLinkType_Nodes)
{
    EXPECT_FALSE(is_link_type(AtomTypes::NODE));
    EXPECT_FALSE(is_link_type(AtomTypes::CONCEPT_NODE));
    EXPECT_FALSE(is_link_type(AtomTypes::ACCOUNT_NODE));
}

// ============================================================================
// Edge Case Tests
// ============================================================================

class AtomSpaceEdgeCaseTest : public ::testing::Test
{
protected:
    AtomSpace atomspace;
};

TEST_F(AtomSpaceEdgeCaseTest, EmptyName)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "");

    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->name().empty());
}

TEST_F(AtomSpaceEdgeCaseTest, VeryLongName)
{
    std::string long_name(10000, 'x');
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, long_name);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name().length(), 10000);
}

TEST_F(AtomSpaceEdgeCaseTest, SpecialCharactersInName)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Test\n\t\"Node'<>&");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name(), "Test\n\t\"Node'<>&");
}

TEST_F(AtomSpaceEdgeCaseTest, UnicodeInName)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Unicode Node");

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name(), "Unicode Node");
}

TEST_F(AtomSpaceEdgeCaseTest, EmptyLink)
{
    auto link = atomspace.add_link(AtomTypes::LIST_LINK, HandleSeq{});

    ASSERT_NE(link, nullptr);
    EXPECT_EQ(link->arity(), 0);
}

TEST_F(AtomSpaceEdgeCaseTest, SelfReferencingLink)
{
    auto node = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Self");
    auto link = atomspace.add_link(AtomTypes::LIST_LINK, node, node);

    ASSERT_NE(link, nullptr);
    EXPECT_EQ(link->arity(), 2);
    EXPECT_EQ(link->outgoing_atom(0), link->outgoing_atom(1));
}

TEST_F(AtomSpaceEdgeCaseTest, DeeplyNestedLinks)
{
    Handle current = atomspace.add_node(AtomTypes::CONCEPT_NODE, "Base");

    for (int i = 0; i < 10; ++i) {
        current = atomspace.add_link(AtomTypes::LIST_LINK, current);
    }

    ASSERT_NE(current, nullptr);
    EXPECT_TRUE(current->is_link());

    // Traverse down
    Handle inner = current;
    int depth = 0;
    while (inner->is_link() && inner->arity() > 0) {
        inner = inner->outgoing_atom(0);
        depth++;
    }
    EXPECT_EQ(depth, 10);
}

TEST_F(AtomSpaceEdgeCaseTest, LargeArityLink)
{
    HandleSeq outgoing;
    for (int i = 0; i < 100; ++i) {
        outgoing.push_back(atomspace.add_node(AtomTypes::CONCEPT_NODE, "Node" + std::to_string(i)));
    }

    auto link = atomspace.add_link(AtomTypes::LIST_LINK, outgoing);

    ASSERT_NE(link, nullptr);
    EXPECT_EQ(link->arity(), 100);
}

TEST_F(AtomSpaceEdgeCaseTest, AddSameAtomTwice)
{
    auto node = create_node(AtomTypes::CONCEPT_NODE, "Test");
    auto added1 = atomspace.add_atom(node);
    auto added2 = atomspace.add_atom(node);

    EXPECT_EQ(added1, added2);
    EXPECT_EQ(atomspace.size(), 1);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
