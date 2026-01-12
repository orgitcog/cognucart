# GnuCash Marketplace Module

## Overview

The Marketplace module extends GnuCash with comprehensive e-commerce and marketplace functionality, inspired by:
- **Shopify Marketplaces** - Admin and buyer-facing marketplace applications
- **Virtunomics** - Business simulation game with complex economic modeling
- **OpenCog Cognitive Framework** - AI-powered financial intelligence

## Features

### Merchant/Seller Features
- **Product Catalog Management**: Create, update, and manage product listings
- **Inventory Tracking**: Real-time stock level monitoring
- **Order Management**: Process orders from creation to delivery
- **Storefront Configuration**: Customize your marketplace presence
- **Sales Analytics**: Track performance metrics and revenue

### Buyer/Customer Features
- **Product Browsing**: Search and filter products
- **Shopping Cart**: Add items and manage orders
- **Checkout Process**: Complete purchases with multiple payment methods
- **Order Tracking**: Monitor order status and delivery
- **Customer Reviews**: Rate and review products

### AI-Powered Features (via OpenCog Integration)
- **Product Recommendations**: Personalized suggestions based on purchase history
- **Dynamic Pricing**: AI-suggested optimal pricing
- **Demand Forecasting**: Predict future product demand
- **Fraud Detection**: Identify suspicious orders
- **Customer Behavior Analysis**: Understand buying patterns

### Business Simulation Features (Virtunomics-inspired)
- **Multi-Entity Management**: Handle multiple business entities
- **Supply Chain Tracking**: Monitor product flow from supplier to customer
- **Market Simulation**: Model marketplace dynamics
- **Agent-Based Trading**: Simulate buyer and seller behaviors
- **Economic Modeling**: Analyze market trends

## Architecture

### Core Components

1. **MarketplaceEngine** (`marketplace_engine.hpp/cpp`)
   - Main interface for all marketplace operations
   - Coordinates between different managers
   - Integrates with OpenCog cognitive engine

2. **Product Management** (`product.hpp/cpp`)
   - Product CRUD operations
   - Category management
   - Variant handling

3. **Order Management** (`order.hpp/cpp`)
   - Order creation and tracking
   - Payment processing
   - Order status updates

4. **Inventory Management** (`inventory.hpp/cpp`)
   - Stock level tracking
   - Low stock alerts
   - Automatic reordering

5. **Customer Management** (`customer.hpp/cpp`)
   - Customer profiles
   - Order history
   - Preferences tracking

6. **Storefront Management** (`storefront.hpp/cpp`)
   - Store configuration
   - Business information
   - Theme customization

### Data Types

See `marketplace_types.hpp` for detailed structure definitions:
- `ProductInfo`: Complete product information
- `OrderInfo`: Order details with items and pricing
- `InventoryItem`: Stock tracking data
- `CustomerInfo`: Customer profile and statistics
- `StorefrontInfo`: Store configuration
- `MarketplaceTransaction`: Accounting integration

## Usage Examples

### Creating a Product

```cpp
#include <libgnucash/marketplace/marketplace_engine.hpp>

using namespace gnc::marketplace;

auto& engine = marketplace_engine();
engine.initialize();

ProductInfo product;
product.seller_id = "SELLER-001";
product.name = "Laptop Computer";
product.description = "High-performance laptop";
product.base_price = 999.99;
product.currency = "USD";
product.status = ProductStatus::ACTIVE;
product.stock_quantity = 50;

std::string product_id = engine.create_product(product);
```

### Processing an Order

```cpp
OrderInfo order;
order.customer_id = "CUST-001";
order.seller_id = "SELLER-001";

OrderItem item;
item.product_id = product_id;
item.quantity = 1;
item.unit_price = 999.99;
item.total_price = 999.99;

order.items.push_back(item);
order.total_amount = 999.99;

std::string order_id = engine.create_order(order);

// Process payment
engine.process_payment(order_id, PaymentMethod::CREDIT_CARD, "TXN-12345");

// Update status
engine.update_order_status(order_id, OrderStatus::SHIPPED);
```

### Managing Inventory

```cpp
// Check stock availability
bool in_stock = engine.check_stock_available(product_id, "", 5);

// Update inventory
engine.update_inventory(product_id, "", -5);  // Decrease by 5

// Get low stock alerts
auto low_stock = engine.get_low_stock_products();
for (const auto& item : low_stock) {
    std::cout << "Low stock: " << item.product_id << std::endl;
}
```

### AI-Powered Recommendations

```cpp
// Get personalized product recommendations
auto recommendations = engine.get_recommendations("CUST-001", 10);

// Get pricing suggestion
std::map<std::string, double> market_data;
market_data["demand_factor"] = 1.2;
double suggested_price = engine.suggest_price(product_id, market_data);

// Predict demand
int predicted_demand = engine.predict_demand(product_id, 30);  // 30 days ahead
```

### Analytics

```cpp
auto start = std::chrono::system_clock::now() - std::chrono::hours(24 * 30);
auto end = std::chrono::system_clock::now();

AnalyticsData analytics = engine.get_sales_analytics("SELLER-001", start, end);

std::cout << "Total revenue: " << analytics.metrics["total_revenue"] << std::endl;
std::cout << "Average order: " << analytics.metrics["average_order_value"] << std::endl;
std::cout << "Total orders: " << analytics.counts["total_orders"] << std::endl;
```

## Building

The marketplace module is built as part of the standard GnuCash build:

```bash
mkdir build && cd build
cmake -G Ninja ..
ninja gnc-marketplace
```

### Running Tests

```bash
ninja gtest-marketplace
./libgnucash/marketplace/test/gtest-marketplace
```

## Integration with GnuCash

The marketplace module integrates seamlessly with GnuCash accounting:

1. **Automatic Transaction Creation**: Orders automatically generate accounting entries
2. **Account Synchronization**: Marketplace data syncs with GnuCash accounts
3. **Revenue Tracking**: Sales are tracked in appropriate income accounts
4. **Expense Management**: Costs are tracked in expense accounts

```cpp
// Create accounting entries for an order
auto transactions = engine.create_accounting_entries(order_id);

// Sync with GnuCash
engine.sync_with_gnucash();
```

## Event System

Subscribe to marketplace events for real-time notifications:

```cpp
engine.subscribe_events([](const std::string& event_type, 
                           const std::string& event_data) {
    if (event_type == "order.created") {
        std::cout << "New order: " << event_data << std::endl;
    }
});
```

Available events:
- `marketplace.initialized`
- `product.created`, `product.updated`, `product.deleted`
- `order.created`, `order.status_changed`, `order.payment_processed`
- `order.cancelled`, `order.refunded`
- `storefront.saved`

## Future Enhancements

- [ ] Multi-channel marketplace support (Amazon, eBay, etc.)
- [ ] Advanced shipping integrations
- [ ] Cryptocurrency payment support
- [ ] Enhanced fraud detection algorithms
- [ ] Machine learning for demand prediction
- [ ] Social marketplace features
- [ ] Auction and bidding system
- [ ] Subscription and recurring billing
- [ ] Advanced reporting and dashboards
- [ ] Mobile app API endpoints

## References

- [Shopify Marketplace Kit](https://shopify.dev/marketplaces)
- [Shopify Admin App](https://github.com/Shopify/shopify-marketplaces-admin-app)
- [Shopify Buyer App](https://github.com/Shopify/shopify-marketplaces-buyer-app)
- [Virtunomics](https://github.com/o9nn/virtunomicog)
- [OpenCog Framework](https://opencog.org/)
- [GnuCash OpenCog Integration](../opencog/README.md)

## License

This module is part of GnuCash and is licensed under GPL-2.0-or-later.

## Contributing

Contributions are welcome! Please see the main GnuCash development guide.
