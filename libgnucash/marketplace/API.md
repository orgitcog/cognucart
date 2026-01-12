# Marketplace API Reference

## Table of Contents

1. [MarketplaceEngine](#marketplaceengine)
2. [Product Management](#product-management)
3. [Order Management](#order-management)
4. [Inventory Management](#inventory-management)
5. [Customer Management](#customer-management)
6. [Storefront Management](#storefront-management)
7. [Analytics](#analytics)
8. [AI Features](#ai-features)
9. [Data Structures](#data-structures)

---

## MarketplaceEngine

The main interface for all marketplace operations.

### Singleton Access

```cpp
#include <libgnucash/marketplace/marketplace_engine.hpp>

using namespace gnc::marketplace;

auto& engine = marketplace_engine();
```

### Initialization

```cpp
bool initialize();
void shutdown();
bool is_initialized() const;
```

**Example:**
```cpp
auto& engine = marketplace_engine();
if (!engine.initialize()) {
    std::cerr << "Failed to initialize\n";
    return 1;
}

// Use engine...

engine.shutdown();
```

---

## Product Management

### create_product

Creates a new product listing.

**Signature:**
```cpp
std::string create_product(const ProductInfo& product);
```

**Parameters:**
- `product`: Product information structure

**Returns:** Product ID

**Example:**
```cpp
ProductInfo product;
product.seller_id = "SELLER-001";
product.name = "Laptop";
product.description = "High-performance laptop";
product.base_price = 999.99;
product.currency = "USD";
product.status = ProductStatus::ACTIVE;
product.stock_quantity = 50;

std::string product_id = engine.create_product(product);
```

### update_product

Updates an existing product.

**Signature:**
```cpp
bool update_product(const std::string& product_id, const ProductInfo& product);
```

**Returns:** `true` on success

### delete_product

Soft-deletes a product (marks as ARCHIVED).

**Signature:**
```cpp
bool delete_product(const std::string& product_id);
```

### get_product

Retrieves product details.

**Signature:**
```cpp
ProductInfo get_product(const std::string& product_id) const;
```

### list_products

Lists products with optional filters.

**Signature:**
```cpp
std::vector<ProductInfo> list_products(
    const std::string& seller_id = "",
    const std::string& category_id = "",
    ProductStatus status = ProductStatus::ACTIVE,
    int limit = 50,
    int offset = 0
) const;
```

**Example:**
```cpp
// List all active products
auto products = engine.list_products();

// List products for specific seller
auto seller_products = engine.list_products("SELLER-001");

// Paginated results
auto page1 = engine.list_products("", "", ProductStatus::ACTIVE, 20, 0);
auto page2 = engine.list_products("", "", ProductStatus::ACTIVE, 20, 20);
```

### search_products

Search products by query string.

**Signature:**
```cpp
std::vector<ProductInfo> search_products(
    const std::string& query,
    const std::string& category_id = "",
    double min_price = 0.0,
    double max_price = 0.0
) const;
```

**Example:**
```cpp
// Simple search
auto results = engine.search_products("laptop");

// Search with price range
auto laptops = engine.search_products("laptop", "", 500.0, 2000.0);

// Search in category
auto electronics = engine.search_products("wireless", "electronics");
```

---

## Order Management

### create_order

Creates a new order.

**Signature:**
```cpp
std::string create_order(const OrderInfo& order);
```

**Returns:** Order ID, or empty string if out of stock

**Example:**
```cpp
OrderInfo order;
order.customer_id = "CUST-001";
order.seller_id = "SELLER-001";
order.status = OrderStatus::PENDING;

OrderItem item;
item.product_id = "PROD-123";
item.product_name = "Laptop";
item.quantity = 1;
item.unit_price = 999.99;
item.total_price = 999.99;

order.items.push_back(item);
order.subtotal = 999.99;
order.tax_amount = 80.00;
order.shipping_cost = 20.00;
order.total_amount = 1099.99;
order.currency = "USD";

std::string order_id = engine.create_order(order);
if (order_id.empty()) {
    std::cerr << "Product out of stock\n";
}
```

### update_order_status

Updates order status.

**Signature:**
```cpp
bool update_order_status(const std::string& order_id, OrderStatus status);
```

**Order Status Flow:**
```
PENDING → PAID → PROCESSING → SHIPPED → DELIVERED
           ↓
        CANCELLED
           ↓
        REFUNDED
```

**Example:**
```cpp
engine.update_order_status(order_id, OrderStatus::PAID);
engine.update_order_status(order_id, OrderStatus::SHIPPED);
engine.update_order_status(order_id, OrderStatus::DELIVERED);
```

### get_order

Retrieves order details.

**Signature:**
```cpp
OrderInfo get_order(const std::string& order_id) const;
```

### list_orders

Lists orders with filters.

**Signature:**
```cpp
std::vector<OrderInfo> list_orders(
    const std::string& customer_id = "",
    const std::string& seller_id = "",
    OrderStatus status = OrderStatus::PENDING
) const;
```

**Example:**
```cpp
// All pending orders
auto pending = engine.list_orders("", "", OrderStatus::PENDING);

// Customer's orders
auto customer_orders = engine.list_orders("CUST-001");

// Seller's orders
auto seller_orders = engine.list_orders("", "SELLER-001");
```

### process_payment

Processes payment for an order.

**Signature:**
```cpp
bool process_payment(
    const std::string& order_id,
    PaymentMethod method,
    const std::string& transaction_id
);
```

**Payment Methods:**
- `PaymentMethod::CREDIT_CARD`
- `PaymentMethod::DEBIT_CARD`
- `PaymentMethod::BANK_TRANSFER`
- `PaymentMethod::DIGITAL_WALLET`
- `PaymentMethod::CRYPTOCURRENCY`
- `PaymentMethod::CASH_ON_DELIVERY`

### cancel_order

Cancels an order and restores inventory.

**Signature:**
```cpp
bool cancel_order(const std::string& order_id, const std::string& reason);
```

### refund_order

Issues a refund for an order.

**Signature:**
```cpp
bool refund_order(const std::string& order_id, double amount);
```

---

## Inventory Management

### update_inventory

Updates inventory levels.

**Signature:**
```cpp
bool update_inventory(
    const std::string& product_id,
    const std::string& variant_id,
    int quantity_change
);
```

**Example:**
```cpp
// Add 100 units to stock
engine.update_inventory("PROD-123", "", 100);

// Remove 5 units (sale)
engine.update_inventory("PROD-123", "", -5);

// Update variant inventory
engine.update_inventory("PROD-123", "VARIANT-RED", -1);
```

### get_inventory

Gets inventory status.

**Signature:**
```cpp
InventoryItem get_inventory(
    const std::string& product_id,
    const std::string& variant_id = ""
) const;
```

### check_stock_available

Checks if sufficient stock is available.

**Signature:**
```cpp
bool check_stock_available(
    const std::string& product_id,
    const std::string& variant_id,
    int quantity
) const;
```

**Example:**
```cpp
if (engine.check_stock_available("PROD-123", "", 5)) {
    // Proceed with order
} else {
    // Out of stock
}
```

### get_low_stock_products

Gets products with low inventory.

**Signature:**
```cpp
std::vector<InventoryItem> get_low_stock_products() const;
```

**Example:**
```cpp
auto low_stock = engine.get_low_stock_products();
for (const auto& item : low_stock) {
    std::cout << "Reorder needed: " << item.product_id 
              << " (current: " << item.quantity_available 
              << ", reorder level: " << item.reorder_level << ")\n";
}
```

---

## Customer Management

### save_customer

Creates or updates customer profile.

**Signature:**
```cpp
std::string save_customer(const CustomerInfo& customer);
```

**Returns:** Customer ID

**Example:**
```cpp
CustomerInfo customer;
customer.name = "John Doe";
customer.email = "john@example.com";
customer.phone = "+1-555-0123";

Address address;
address.street = "123 Main St";
address.city = "Springfield";
address.state = "IL";
address.postal_code = "62701";
address.country = "USA";
customer.addresses.push_back(address);

std::string customer_id = engine.save_customer(customer);
```

### get_customer

Retrieves customer details.

**Signature:**
```cpp
CustomerInfo get_customer(const std::string& customer_id) const;
```

### get_customer_orders

Gets customer's order history.

**Signature:**
```cpp
std::vector<OrderInfo> get_customer_orders(const std::string& customer_id) const;
```

---

## Storefront Management

### save_storefront

Creates or updates storefront.

**Signature:**
```cpp
std::string save_storefront(const StorefrontInfo& storefront);
```

**Example:**
```cpp
StorefrontInfo storefront;
storefront.seller_id = "SELLER-001";
storefront.name = "Tech Store";
storefront.description = "Your tech destination";
storefront.email = "store@example.com";
storefront.phone = "+1-555-0123";
storefront.business_name = "Tech Inc.";
storefront.logo_url = "https://example.com/logo.png";
storefront.theme_color = "#0066cc";

std::string storefront_id = engine.save_storefront(storefront);
```

### get_storefront

Retrieves storefront by ID.

**Signature:**
```cpp
StorefrontInfo get_storefront(const std::string& storefront_id) const;
```

### get_storefront_by_seller

Retrieves storefront by seller ID.

**Signature:**
```cpp
StorefrontInfo get_storefront_by_seller(const std::string& seller_id) const;
```

### list_storefronts

Lists all active storefronts.

**Signature:**
```cpp
std::vector<StorefrontInfo> list_storefronts(int limit = 50, int offset = 0) const;
```

---

## Analytics

### get_sales_analytics

Gets sales analytics for a time period.

**Signature:**
```cpp
AnalyticsData get_sales_analytics(
    const std::string& seller_id,
    const std::chrono::system_clock::time_point& start_date,
    const std::chrono::system_clock::time_point& end_date
) const;
```

**Example:**
```cpp
auto start = std::chrono::system_clock::now() - std::chrono::hours(24 * 30);
auto end = std::chrono::system_clock::now();

auto analytics = engine.get_sales_analytics("SELLER-001", start, end);

std::cout << "Total Revenue: $" << analytics.metrics["total_revenue"] << "\n";
std::cout << "Avg Order: $" << analytics.metrics["average_order_value"] << "\n";
std::cout << "Orders: " << analytics.counts["total_orders"] << "\n";
```

### get_product_performance

Gets product performance metrics.

**Signature:**
```cpp
std::vector<AnalyticsData> get_product_performance(
    const std::string& seller_id
) const;
```

### get_customer_analytics

Gets customer analytics.

**Signature:**
```cpp
AnalyticsData get_customer_analytics(const std::string& customer_id) const;
```

### get_stats

Gets overall marketplace statistics.

**Signature:**
```cpp
struct Stats {
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
```

---

## AI Features

Powered by OpenCog cognitive engine integration.

### get_recommendations

Gets AI-powered product recommendations.

**Signature:**
```cpp
std::vector<ProductInfo> get_recommendations(
    const std::string& customer_id,
    int limit = 10
) const;
```

**Example:**
```cpp
auto recommendations = engine.get_recommendations("CUST-001", 5);
for (const auto& product : recommendations) {
    std::cout << "Recommended: " << product.name 
              << " ($" << product.base_price << ")\n";
}
```

### suggest_price

Gets dynamic pricing suggestion.

**Signature:**
```cpp
double suggest_price(
    const std::string& product_id,
    const std::map<std::string, double>& market_data
) const;
```

**Example:**
```cpp
std::map<std::string, double> market_data;
market_data["demand_factor"] = 1.5;  // High demand
market_data["competition_factor"] = 0.9;  // Lower competition
market_data["season_factor"] = 1.2;  // Holiday season

double optimal_price = engine.suggest_price("PROD-123", market_data);
```

### predict_demand

Predicts future demand.

**Signature:**
```cpp
int predict_demand(const std::string& product_id, int days_ahead) const;
```

### detect_fraud

Detects potential fraud in an order.

**Signature:**
```cpp
bool detect_fraud(const std::string& order_id) const;
```

---

## GnuCash Integration

### create_accounting_entries

Creates accounting transactions for an order.

**Signature:**
```cpp
std::vector<MarketplaceTransaction> create_accounting_entries(
    const std::string& order_id
);
```

**Example:**
```cpp
auto transactions = engine.create_accounting_entries(order_id);
for (const auto& txn : transactions) {
    std::cout << "Transaction: " << txn.type 
              << " Amount: $" << txn.amount << "\n";
}
```

### sync_with_gnucash

Syncs marketplace data with GnuCash accounts.

**Signature:**
```cpp
bool sync_with_gnucash();
```

---

## Events

### subscribe_events

Subscribe to marketplace events.

**Signature:**
```cpp
void subscribe_events(MarketplaceEventCallback callback);

// Callback type:
using MarketplaceEventCallback = 
    std::function<void(const std::string& event_type, 
                       const std::string& event_data)>;
```

**Event Types:**
- `marketplace.initialized`
- `marketplace.shutdown`
- `product.created`
- `product.updated`
- `product.deleted`
- `order.created`
- `order.status_changed`
- `order.payment_processed`
- `order.cancelled`
- `order.refunded`
- `storefront.saved`

**Example:**
```cpp
engine.subscribe_events([](const std::string& event_type,
                           const std::string& event_data) {
    std::cout << "Event: " << event_type << " - " << event_data << "\n";
    
    if (event_type == "order.created") {
        // Send confirmation email
    } else if (event_type == "product.created") {
        // Update search index
    }
});
```

---

## Complete Example

```cpp
#include <libgnucash/marketplace/marketplace_engine.hpp>
#include <iostream>

using namespace gnc::marketplace;

int main() {
    // Initialize
    auto& engine = marketplace_engine();
    engine.initialize();
    
    // Subscribe to events
    engine.subscribe_events([](auto type, auto data) {
        std::cout << "Event: " << type << "\n";
    });
    
    // Create storefront
    StorefrontInfo store;
    store.seller_id = "SELLER-001";
    store.name = "My Store";
    auto store_id = engine.save_storefront(store);
    
    // Add product
    ProductInfo product;
    product.seller_id = "SELLER-001";
    product.name = "Widget";
    product.base_price = 29.99;
    product.currency = "USD";
    product.status = ProductStatus::ACTIVE;
    product.stock_quantity = 100;
    auto product_id = engine.create_product(product);
    
    // Create customer
    CustomerInfo customer;
    customer.name = "John Doe";
    customer.email = "john@example.com";
    auto customer_id = engine.save_customer(customer);
    
    // Place order
    OrderInfo order;
    order.customer_id = customer_id;
    order.seller_id = "SELLER-001";
    
    OrderItem item;
    item.product_id = product_id;
    item.quantity = 2;
    item.unit_price = 29.99;
    item.total_price = 59.98;
    order.items.push_back(item);
    order.total_amount = 59.98;
    
    auto order_id = engine.create_order(order);
    
    // Process payment
    engine.process_payment(order_id, 
                          PaymentMethod::CREDIT_CARD, 
                          "TXN-001");
    
    // Get stats
    auto stats = engine.get_stats();
    std::cout << "Total Revenue: $" << stats.total_revenue << "\n";
    
    // Cleanup
    engine.shutdown();
    return 0;
}
```

---

## Error Handling

Most methods return boolean success indicators or empty strings on failure:

```cpp
// Check product exists
auto product = engine.get_product(product_id);
if (product.id.empty()) {
    std::cerr << "Product not found\n";
}

// Check order creation success
auto order_id = engine.create_order(order);
if (order_id.empty()) {
    std::cerr << "Failed to create order (out of stock?)\n";
}

// Check update success
if (!engine.update_order_status(order_id, OrderStatus::SHIPPED)) {
    std::cerr << "Failed to update order status\n";
}
```

---

## Thread Safety

The MarketplaceEngine uses internal locking for thread-safe operations. Multiple threads can safely call engine methods concurrently.

---

## See Also

- [README.md](README.md) - Module overview
- [INTEGRATION.md](INTEGRATION.md) - Integration with Shopify and Virtunomics
- [marketplace_types.hpp](marketplace_types.hpp) - Complete data structure reference
