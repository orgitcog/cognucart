/*
 * libgnucash/marketplace/marketplace_types.hpp
 *
 * GnuCash Marketplace Module - Core Types
 * Defines data structures for marketplace operations
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GNC_MARKETPLACE_TYPES_HPP
#define GNC_MARKETPLACE_TYPES_HPP

#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <memory>

namespace gnc {
namespace marketplace {

/**
 * Product status in the marketplace.
 */
enum class ProductStatus
{
    DRAFT,          // Not yet published
    ACTIVE,         // Available for sale
    OUT_OF_STOCK,   // Temporarily unavailable
    DISCONTINUED,   // No longer available
    ARCHIVED        // Hidden from listings
};

/**
 * Order status in the marketplace.
 */
enum class OrderStatus
{
    PENDING,        // Awaiting payment
    PAID,           // Payment received
    PROCESSING,     // Being prepared
    SHIPPED,        // In transit
    DELIVERED,      // Completed
    CANCELLED,      // Cancelled by buyer/seller
    REFUNDED        // Payment refunded
};

/**
 * Payment method types.
 */
enum class PaymentMethod
{
    CREDIT_CARD,
    DEBIT_CARD,
    BANK_TRANSFER,
    DIGITAL_WALLET,
    CRYPTOCURRENCY,
    CASH_ON_DELIVERY
};

/**
 * Shipping method types.
 */
enum class ShippingMethod
{
    STANDARD,
    EXPRESS,
    OVERNIGHT,
    INTERNATIONAL,
    PICKUP,
    DIGITAL         // For digital products
};

/**
 * Product category types.
 */
struct ProductCategory
{
    std::string id;
    std::string name;
    std::string description;
    std::string parent_id;  // For hierarchical categories
    std::vector<std::string> tags;
};

/**
 * Product variant (size, color, etc.)
 */
struct ProductVariant
{
    std::string id;
    std::string name;
    std::map<std::string, std::string> attributes;  // e.g., {"size": "L", "color": "blue"}
    double price_adjustment;  // Additional cost for this variant
    int stock_quantity;
    std::string sku;  // Stock Keeping Unit
};

/**
 * Product image information.
 */
struct ProductImage
{
    std::string id;
    std::string url;
    std::string alt_text;
    bool is_primary;
    int display_order;
};

/**
 * Product information structure.
 */
struct ProductInfo
{
    std::string id;
    std::string seller_id;
    std::string name;
    std::string description;
    std::string category_id;
    double base_price;
    std::string currency;
    ProductStatus status;
    int stock_quantity;
    std::vector<ProductVariant> variants;
    std::vector<ProductImage> images;
    std::vector<std::string> tags;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    
    // Marketplace-specific
    double average_rating;
    int review_count;
    int total_sales;
    bool featured;
    
    // Shipping
    double weight_kg;
    std::map<std::string, double> dimensions_cm;  // length, width, height
    std::vector<ShippingMethod> available_shipping;
};

/**
 * Order item within an order.
 */
struct OrderItem
{
    std::string id;
    std::string product_id;
    std::string variant_id;
    std::string product_name;
    int quantity;
    double unit_price;
    double total_price;
    double tax_amount;
    double discount_amount;
};

/**
 * Shipping address.
 */
struct Address
{
    std::string street;
    std::string city;
    std::string state;
    std::string postal_code;
    std::string country;
    std::string phone;
};

/**
 * Order information structure.
 */
struct OrderInfo
{
    std::string id;
    std::string customer_id;
    std::string seller_id;
    OrderStatus status;
    std::vector<OrderItem> items;
    
    // Pricing
    double subtotal;
    double tax_amount;
    double shipping_cost;
    double discount_amount;
    double total_amount;
    std::string currency;
    
    // Payment
    PaymentMethod payment_method;
    std::string payment_transaction_id;
    std::chrono::system_clock::time_point payment_date;
    
    // Shipping
    ShippingMethod shipping_method;
    Address shipping_address;
    Address billing_address;
    std::string tracking_number;
    std::chrono::system_clock::time_point estimated_delivery;
    
    // Timestamps
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    
    // Notes
    std::string customer_notes;
    std::string seller_notes;
};

/**
 * Customer information.
 */
struct CustomerInfo
{
    std::string id;
    std::string name;
    std::string email;
    std::string phone;
    std::vector<Address> addresses;
    std::chrono::system_clock::time_point registration_date;
    
    // Stats
    int total_orders;
    double total_spent;
    double average_order_value;
    
    // Preferences
    std::vector<std::string> favorite_categories;
    std::vector<std::string> favorite_sellers;
};

/**
 * Storefront configuration.
 */
struct StorefrontInfo
{
    std::string id;
    std::string seller_id;
    std::string name;
    std::string description;
    std::string logo_url;
    std::string banner_url;
    std::string theme_color;
    
    // Contact
    std::string email;
    std::string phone;
    std::string website;
    
    // Business info
    std::string business_name;
    std::string business_registration;
    Address business_address;
    
    // Stats
    double average_rating;
    int total_sales;
    int product_count;
    std::chrono::system_clock::time_point established_date;
    
    // Settings
    bool accepts_custom_orders;
    std::vector<PaymentMethod> accepted_payment_methods;
    std::vector<ShippingMethod> supported_shipping_methods;
    int min_order_value;
    int max_order_value;
};

/**
 * Inventory item tracking.
 */
struct InventoryItem
{
    std::string id;
    std::string product_id;
    std::string variant_id;
    int quantity_available;
    int quantity_reserved;  // In active orders
    int quantity_incoming;  // On order from suppliers
    int reorder_level;      // Minimum before reorder
    int reorder_quantity;   // How many to reorder
    std::string warehouse_location;
    std::chrono::system_clock::time_point last_stock_check;
    std::vector<std::string> supplier_ids;
};

/**
 * Product review.
 */
struct ProductReview
{
    std::string id;
    std::string product_id;
    std::string customer_id;
    std::string customer_name;
    int rating;  // 1-5 stars
    std::string title;
    std::string comment;
    std::vector<std::string> image_urls;
    bool verified_purchase;
    std::chrono::system_clock::time_point created_at;
    
    // Engagement
    int helpful_count;
    int not_helpful_count;
};

/**
 * Analytics data point.
 */
struct AnalyticsData
{
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, double> metrics;
    std::map<std::string, int> counts;
    std::map<std::string, std::string> dimensions;
};

/**
 * Marketplace transaction (for accounting integration).
 */
struct MarketplaceTransaction
{
    std::string id;
    std::string order_id;
    std::string account_guid;  // GnuCash account
    std::string type;  // "sale", "purchase", "fee", "refund"
    double amount;
    std::string currency;
    std::string description;
    std::chrono::system_clock::time_point transaction_date;
};

} // namespace marketplace
} // namespace gnc

#endif // GNC_MARKETPLACE_TYPES_HPP
