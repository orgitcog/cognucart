/*
 * libgnucash/marketplace/marketplace_engine.hpp
 *
 * GnuCash Marketplace Engine - Main Interface
 * Provides marketplace functionality integrated with GnuCash accounting
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GNC_MARKETPLACE_ENGINE_HPP
#define GNC_MARKETPLACE_ENGINE_HPP

#include "marketplace_types.hpp"
#include "product.hpp"
#include "order.hpp"
#include "storefront.hpp"
#include "inventory.hpp"
#include "customer.hpp"

#include <memory>
#include <functional>
#include <vector>

namespace gnc {
namespace marketplace {

/**
 * Callback for marketplace events.
 */
using MarketplaceEventCallback = std::function<void(const std::string& event_type, 
                                                     const std::string& event_data)>;

/**
 * MarketplaceEngine - Main interface for marketplace operations.
 *
 * This engine provides:
 * - Product catalog management
 * - Order processing and tracking
 * - Inventory management
 * - Customer management
 * - Storefront configuration
 * - Integration with GnuCash accounting
 * - AI-powered recommendations and analytics
 */
class MarketplaceEngine
{
public:
    MarketplaceEngine();
    ~MarketplaceEngine();

    /**
     * Initialize the marketplace engine.
     */
    bool initialize();

    /**
     * Shutdown the marketplace engine.
     */
    void shutdown();

    /**
     * Check if engine is initialized.
     */
    bool is_initialized() const { return m_initialized; }

    // =========================================
    // Product Management
    // =========================================

    /**
     * Create a new product listing.
     */
    std::string create_product(const ProductInfo& product);

    /**
     * Update an existing product.
     */
    bool update_product(const std::string& product_id, const ProductInfo& product);

    /**
     * Delete a product (soft delete - marks as archived).
     */
    bool delete_product(const std::string& product_id);

    /**
     * Get product details.
     */
    ProductInfo get_product(const std::string& product_id) const;

    /**
     * List products with filters.
     */
    std::vector<ProductInfo> list_products(const std::string& seller_id = "",
                                          const std::string& category_id = "",
                                          ProductStatus status = ProductStatus::ACTIVE,
                                          int limit = 50, int offset = 0) const;

    /**
     * Search products by query.
     */
    std::vector<ProductInfo> search_products(const std::string& query,
                                            const std::string& category_id = "",
                                            double min_price = 0.0,
                                            double max_price = 0.0) const;

    // =========================================
    // Order Management
    // =========================================

    /**
     * Create a new order.
     */
    std::string create_order(const OrderInfo& order);

    /**
     * Update order status.
     */
    bool update_order_status(const std::string& order_id, OrderStatus status);

    /**
     * Get order details.
     */
    OrderInfo get_order(const std::string& order_id) const;

    /**
     * List orders for a customer or seller.
     */
    std::vector<OrderInfo> list_orders(const std::string& customer_id = "",
                                      const std::string& seller_id = "",
                                      OrderStatus status = OrderStatus::PENDING) const;

    /**
     * Process order payment.
     */
    bool process_payment(const std::string& order_id,
                        PaymentMethod method,
                        const std::string& transaction_id);

    /**
     * Cancel an order.
     */
    bool cancel_order(const std::string& order_id, const std::string& reason);

    /**
     * Issue refund for an order.
     */
    bool refund_order(const std::string& order_id, double amount);

    // =========================================
    // Inventory Management
    // =========================================

    /**
     * Update inventory levels.
     */
    bool update_inventory(const std::string& product_id,
                         const std::string& variant_id,
                         int quantity_change);

    /**
     * Get inventory status.
     */
    InventoryItem get_inventory(const std::string& product_id,
                                const std::string& variant_id = "") const;

    /**
     * Check stock availability.
     */
    bool check_stock_available(const std::string& product_id,
                              const std::string& variant_id,
                              int quantity) const;

    /**
     * Get low stock products.
     */
    std::vector<InventoryItem> get_low_stock_products() const;

    // =========================================
    // Customer Management
    // =========================================

    /**
     * Create or update customer profile.
     */
    std::string save_customer(const CustomerInfo& customer);

    /**
     * Get customer details.
     */
    CustomerInfo get_customer(const std::string& customer_id) const;

    /**
     * Get customer order history.
     */
    std::vector<OrderInfo> get_customer_orders(const std::string& customer_id) const;

    // =========================================
    // Storefront Management
    // =========================================

    /**
     * Create or update storefront.
     */
    std::string save_storefront(const StorefrontInfo& storefront);

    /**
     * Get storefront details.
     */
    StorefrontInfo get_storefront(const std::string& storefront_id) const;

    /**
     * Get storefront by seller ID.
     */
    StorefrontInfo get_storefront_by_seller(const std::string& seller_id) const;

    /**
     * List all active storefronts.
     */
    std::vector<StorefrontInfo> list_storefronts(int limit = 50, int offset = 0) const;

    // =========================================
    // Analytics & Reporting
    // =========================================

    /**
     * Get sales analytics for a period.
     */
    AnalyticsData get_sales_analytics(const std::string& seller_id,
                                     const std::chrono::system_clock::time_point& start_date,
                                     const std::chrono::system_clock::time_point& end_date) const;

    /**
     * Get product performance metrics.
     */
    std::vector<AnalyticsData> get_product_performance(const std::string& seller_id) const;

    /**
     * Get customer analytics.
     */
    AnalyticsData get_customer_analytics(const std::string& customer_id) const;

    // =========================================
    // AI-Powered Features
    // =========================================

    /**
     * Get product recommendations for a customer.
     */
    std::vector<ProductInfo> get_recommendations(const std::string& customer_id,
                                                int limit = 10) const;

    /**
     * Get dynamic pricing suggestion.
     */
    double suggest_price(const std::string& product_id,
                        const std::map<std::string, double>& market_data) const;

    /**
     * Predict demand for a product.
     */
    int predict_demand(const std::string& product_id, int days_ahead) const;

    /**
     * Detect potential fraud in an order.
     */
    bool detect_fraud(const std::string& order_id) const;

    // =========================================
    // GnuCash Integration
    // =========================================

    /**
     * Create accounting transactions for an order.
     */
    std::vector<MarketplaceTransaction> create_accounting_entries(const std::string& order_id);

    /**
     * Sync marketplace data with GnuCash accounts.
     */
    bool sync_with_gnucash();

    // =========================================
    // Events & Notifications
    // =========================================

    /**
     * Subscribe to marketplace events.
     */
    void subscribe_events(MarketplaceEventCallback callback);

    /**
     * Get marketplace statistics.
     */
    struct Stats
    {
        size_t total_products;
        size_t active_products;
        size_t total_orders;
        size_t pending_orders;
        size_t total_customers;
        size_t active_storefronts;
        double total_revenue;
        double average_order_value;
    };

    Stats get_stats() const;

private:
    bool m_initialized;

    // Component managers
    std::unique_ptr<class ProductManager> m_product_manager;
    std::unique_ptr<class OrderManager> m_order_manager;
    std::unique_ptr<class StorefrontManager> m_storefront_manager;
    std::unique_ptr<class InventoryManager> m_inventory_manager;
    std::unique_ptr<class CustomerManager> m_customer_manager;

    // Event subscribers
    std::vector<MarketplaceEventCallback> m_event_callbacks;

    // Internal helpers
    void notify_event(const std::string& event_type, const std::string& event_data);
    std::string generate_id(const std::string& prefix);
};

/**
 * Global marketplace engine instance.
 */
MarketplaceEngine& marketplace_engine();

} // namespace marketplace
} // namespace gnc

#endif // GNC_MARKETPLACE_ENGINE_HPP
