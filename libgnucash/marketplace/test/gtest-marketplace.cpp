/*
 * libgnucash/marketplace/test/gtest-marketplace.cpp
 *
 * Unit tests for GnuCash Marketplace Module
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include "../marketplace_engine.hpp"

using namespace gnc::marketplace;

class MarketplaceTest : public ::testing::Test
{
protected:
    void SetUp() override {
        engine = &marketplace_engine();
        engine->initialize();
    }

    void TearDown() override {
        engine->shutdown();
    }

    MarketplaceEngine* engine;
};

TEST_F(MarketplaceTest, Initialization)
{
    EXPECT_TRUE(engine->is_initialized());
}

TEST_F(MarketplaceTest, CreateProduct)
{
    ProductInfo product;
    product.seller_id = "SELLER-001";
    product.name = "Test Product";
    product.description = "A test product";
    product.base_price = 99.99;
    product.currency = "USD";
    product.status = ProductStatus::ACTIVE;
    product.stock_quantity = 100;

    std::string product_id = engine->create_product(product);
    
    EXPECT_FALSE(product_id.empty());
    
    auto retrieved = engine->get_product(product_id);
    EXPECT_EQ(retrieved.name, "Test Product");
    EXPECT_EQ(retrieved.base_price, 99.99);
}

TEST_F(MarketplaceTest, CreateOrder)
{
    // First create a product
    ProductInfo product;
    product.seller_id = "SELLER-001";
    product.name = "Order Test Product";
    product.base_price = 50.00;
    product.currency = "USD";
    product.status = ProductStatus::ACTIVE;
    product.stock_quantity = 10;
    
    std::string product_id = engine->create_product(product);
    
    // Create an order
    OrderInfo order;
    order.customer_id = "CUST-001";
    order.seller_id = "SELLER-001";
    order.status = OrderStatus::PENDING;
    
    OrderItem item;
    item.product_id = product_id;
    item.product_name = "Order Test Product";
    item.quantity = 2;
    item.unit_price = 50.00;
    item.total_price = 100.00;
    
    order.items.push_back(item);
    order.subtotal = 100.00;
    order.total_amount = 100.00;
    order.currency = "USD";
    
    std::string order_id = engine->create_order(order);
    
    EXPECT_FALSE(order_id.empty());
    
    auto retrieved = engine->get_order(order_id);
    EXPECT_EQ(retrieved.customer_id, "CUST-001");
    EXPECT_EQ(retrieved.total_amount, 100.00);
    EXPECT_EQ(retrieved.items.size(), 1);
}

TEST_F(MarketplaceTest, InventoryManagement)
{
    // Create a product
    ProductInfo product;
    product.seller_id = "SELLER-001";
    product.name = "Inventory Test Product";
    product.base_price = 25.00;
    product.currency = "USD";
    product.status = ProductStatus::ACTIVE;
    product.stock_quantity = 50;
    
    std::string product_id = engine->create_product(product);
    
    // Check initial stock
    auto inventory = engine->get_inventory(product_id);
    EXPECT_EQ(inventory.quantity_available, 50);
    
    // Update stock
    engine->update_inventory(product_id, "", -10);
    
    inventory = engine->get_inventory(product_id);
    EXPECT_EQ(inventory.quantity_available, 40);
    
    // Check availability
    EXPECT_TRUE(engine->check_stock_available(product_id, "", 40));
    EXPECT_FALSE(engine->check_stock_available(product_id, "", 50));
}

TEST_F(MarketplaceTest, CustomerManagement)
{
    CustomerInfo customer;
    customer.name = "John Doe";
    customer.email = "john@example.com";
    customer.phone = "+1234567890";
    
    std::string customer_id = engine->save_customer(customer);
    
    EXPECT_FALSE(customer_id.empty());
    
    auto retrieved = engine->get_customer(customer_id);
    EXPECT_EQ(retrieved.name, "John Doe");
    EXPECT_EQ(retrieved.email, "john@example.com");
}

TEST_F(MarketplaceTest, StorefrontManagement)
{
    StorefrontInfo storefront;
    storefront.seller_id = "SELLER-001";
    storefront.name = "Test Store";
    storefront.description = "A test storefront";
    storefront.email = "store@example.com";
    storefront.business_name = "Test Business Inc.";
    
    std::string storefront_id = engine->save_storefront(storefront);
    
    EXPECT_FALSE(storefront_id.empty());
    
    auto retrieved = engine->get_storefront(storefront_id);
    EXPECT_EQ(retrieved.name, "Test Store");
    EXPECT_EQ(retrieved.business_name, "Test Business Inc.");
}

TEST_F(MarketplaceTest, SearchProducts)
{
    // Create multiple products
    for (int i = 0; i < 5; i++) {
        ProductInfo product;
        product.seller_id = "SELLER-001";
        product.name = "Product " + std::to_string(i);
        product.description = "Test product number " + std::to_string(i);
        product.base_price = 10.0 * (i + 1);
        product.currency = "USD";
        product.status = ProductStatus::ACTIVE;
        product.stock_quantity = 100;
        
        engine->create_product(product);
    }
    
    // Search for products
    auto results = engine->search_products("Product");
    EXPECT_GE(results.size(), 5);
}

TEST_F(MarketplaceTest, OrderStatusUpdate)
{
    // Create product
    ProductInfo product;
    product.seller_id = "SELLER-001";
    product.name = "Status Test Product";
    product.base_price = 30.00;
    product.currency = "USD";
    product.status = ProductStatus::ACTIVE;
    product.stock_quantity = 10;
    
    std::string product_id = engine->create_product(product);
    
    // Create order
    OrderInfo order;
    order.customer_id = "CUST-001";
    order.seller_id = "SELLER-001";
    order.status = OrderStatus::PENDING;
    
    OrderItem item;
    item.product_id = product_id;
    item.quantity = 1;
    item.unit_price = 30.00;
    item.total_price = 30.00;
    order.items.push_back(item);
    order.total_amount = 30.00;
    
    std::string order_id = engine->create_order(order);
    
    // Update status
    EXPECT_TRUE(engine->update_order_status(order_id, OrderStatus::PAID));
    EXPECT_TRUE(engine->update_order_status(order_id, OrderStatus::SHIPPED));
    EXPECT_TRUE(engine->update_order_status(order_id, OrderStatus::DELIVERED));
    
    auto retrieved = engine->get_order(order_id);
    EXPECT_EQ(retrieved.status, OrderStatus::DELIVERED);
}

TEST_F(MarketplaceTest, GetStats)
{
    auto stats = engine->get_stats();
    
    // Stats should be valid even if empty
    EXPECT_GE(stats.total_products, 0);
    EXPECT_GE(stats.total_orders, 0);
    EXPECT_GE(stats.total_revenue, 0.0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
