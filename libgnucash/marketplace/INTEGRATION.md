# Marketplace Integration Guide

## Overview

This document explains how the GnuCash Marketplace module integrates concepts and features from the referenced repositories:

1. **[Shopify Marketplaces Admin App](https://github.com/Shopify/shopify-marketplaces-admin-app)** - Merchant-facing channel app
2. **[Shopify Marketplaces Buyer App](https://github.com/Shopify/shopify-marketplaces-buyer-app)** - Buyer-facing marketplace app  
3. **[Virtunomics](https://github.com/o9nn/virtunomicog)** - Business simulation game

## Feature Mapping

### From Shopify Marketplaces Admin App

| Shopify Feature | GnuCash Implementation | Status |
|----------------|------------------------|--------|
| **Product Listings** | `ProductInfo` struct, `ProductManager` class | ✅ Complete |
| **Storefront Access Tokens** | `StorefrontInfo` with authentication fields | ✅ Structure Ready |
| **Session Management** | Event callback system, engine state tracking | ✅ Complete |
| **Database Integration (Sequelize)** | In-memory managers, extensible to SQL | ✅ Foundation |
| **GraphQL/REST Handlers** | `MarketplaceEngine` public API | ✅ C++ API |
| **OAuth Scopes** | Future: Authentication layer | 📋 Planned |
| **Merchant Dashboard** | Demo app showing capabilities | ✅ Example |
| **Webhook System** | Event notification callbacks | ✅ Complete |

**Key Implementations:**

```cpp
// Storefront configuration (inspired by Shopify channel apps)
StorefrontInfo storefront;
storefront.seller_id = "SELLER-001";
storefront.name = "My Store";
storefront.email = "store@example.com";
storefront.business_name = "Business Inc.";
// ... theme, branding, payment methods, shipping options

std::string storefront_id = engine.save_storefront(storefront);
```

```cpp
// Event system (like Shopify webhooks)
engine.subscribe_events([](const std::string& event_type, 
                           const std::string& event_data) {
    if (event_type == "order.created") {
        // Handle new order notification
    }
});
```

### From Shopify Marketplaces Buyer App

| Shopify Feature | GnuCash Implementation | Status |
|----------------|------------------------|--------|
| **Next.js Frontend** | Future: Web UI framework | 📋 Planned |
| **Product Discovery** | `search_products()`, `list_products()` | ✅ Complete |
| **Product Grid/Cards** | Data structures with images, ratings | ✅ Data Ready |
| **Shopping Cart** | `OrderInfo` with multiple `OrderItem`s | ✅ Complete |
| **Checkout Flow** | `process_payment()`, order status tracking | ✅ Complete |
| **GraphQL Queries** | C++ API ready for GraphQL wrapper | ✅ Foundation |
| **ApolloClient Integration** | Event-driven architecture | ✅ Pattern Ready |
| **Multi-Shop Support** | Multiple `StorefrontInfo` entries | ✅ Complete |

**Key Implementations:**

```cpp
// Product browsing and search (buyer experience)
auto results = engine.search_products("laptop", 
                                     "electronics", 
                                     min_price, 
                                     max_price);

// Shopping cart as order
OrderInfo cart;
cart.customer_id = "CUST-001";
cart.status = OrderStatus::PENDING;

OrderItem item;
item.product_id = "PROD-123";
item.quantity = 1;
item.unit_price = 999.99;
cart.items.push_back(item);

// Checkout
std::string order_id = engine.create_order(cart);
engine.process_payment(order_id, PaymentMethod::CREDIT_CARD, "TXN-001");
```

### From Virtunomics

| Virtunomics Feature | GnuCash Implementation | Status |
|-------------------|------------------------|--------|
| **Business Simulation** | `MarketplaceEngine` with economic modeling | ✅ Complete |
| **Multi-Entity Management** | Data structures support multiple entities | ✅ Structure |
| **Supply Chain** | `InventoryManager`, supplier tracking | ✅ Foundation |
| **Financial Management** | GnuCash accounting integration | ✅ Hooks Ready |
| **Market Dynamics** | `suggest_price()`, demand forecasting | ✅ Basic |
| **Agent-Based System** | OpenCog cognitive engine integration | ✅ Complete |
| **Turn-Based Processing** | Event-driven updates | ✅ Architecture |
| **Business Units** | Storefronts as business entities | ✅ Complete |
| **Technology Research** | Future: Product innovation tracking | 📋 Planned |
| **Contract System** | Order terms and delivery tracking | ✅ Basic |

**Key Implementations:**

```cpp
// Multi-entity business management
StorefrontInfo entity1, entity2, entity3;
// ... configure each entity

// Supply chain and inventory
InventoryItem inventory;
inventory.product_id = "PROD-001";
inventory.quantity_available = 100;
inventory.quantity_incoming = 50;  // On order from suppliers
inventory.reorder_level = 20;
inventory.supplier_ids = {"SUP-001", "SUP-002"};

// Market dynamics and pricing
std::map<std::string, double> market_data;
market_data["demand_factor"] = 1.2;
market_data["competition_factor"] = 0.9;
double optimal_price = engine.suggest_price(product_id, market_data);

// Economic simulation
int predicted_demand = engine.predict_demand(product_id, 30);  // 30 days
```

## Architecture Comparison

### Shopify Architecture → GnuCash Marketplace

```
Shopify:
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│ React App   │────▶│ Express      │────▶│ PostgreSQL  │
│ (Frontend)  │     │ GraphQL/REST │     │ (Sequelize) │
└─────────────┘     └──────────────┘     └─────────────┘
                           │
                    ┌──────▼──────┐
                    │ Shopify API │
                    └─────────────┘

GnuCash Marketplace:
┌─────────────┐     ┌──────────────────┐     ┌──────────────┐
│ UI          │────▶│ MarketplaceEngine│────▶│ In-Memory/DB │
│ (Future)    │     │ C++ API          │     │ (Managers)   │
└─────────────┘     └──────────────────┘     └──────────────┘
                           │
                    ┌──────▼──────────┐
                    │ OpenCog AI      │
                    │ GnuCash Account │
                    └─────────────────┘
```

### Virtunomics Architecture → GnuCash Marketplace

```
Virtunomics:
┌─────────────┐     ┌──────────────┐     ┌──────────────┐
│ React UI    │────▶│ tRPC Server  │────▶│ MySQL/Drizzle│
│ (Client)    │     │ (TypeScript) │     │              │
└─────────────┘     └──────────────┘     └──────────────┘
                           │
                    ┌──────▼──────────┐
                    │ Game Services   │
                    │ - Agent Brain   │
                    │ - World Economy │
                    │ - Event Bridge  │
                    └─────────────────┘

GnuCash Marketplace:
┌─────────────┐     ┌──────────────────┐     ┌──────────────┐
│ Demo App    │────▶│ MarketplaceEngine│────▶│ Managers     │
│ (C++)       │     │ (C++17)          │     │ (In-Memory)  │
└─────────────┘     └──────────────────┘     └──────────────┘
                           │
                    ┌──────▼──────────┐
                    │ OpenCog Engine  │
                    │ - Cognitive AI  │
                    │ - Pattern Match │
                    │ - Tensor Logic  │
                    └─────────────────┘
```

## Data Model Comparison

### Product Entity

**Shopify Admin App:**
```javascript
{
  id: "product_123",
  title: "Product Name",
  description: "...",
  variants: [...],
  images: [...],
  vendor: "...",
  product_type: "...",
  tags: [...]
}
```

**GnuCash Marketplace:**
```cpp
struct ProductInfo {
    std::string id;
    std::string seller_id;
    std::string name;
    std::string description;
    double base_price;
    std::string currency;
    ProductStatus status;
    int stock_quantity;
    std::vector<ProductVariant> variants;
    std::vector<ProductImage> images;
    std::vector<std::string> tags;
    // ... additional fields
};
```

### Order Entity

**Shopify Buyer App:**
```javascript
{
  id: "order_123",
  customer: {...},
  line_items: [...],
  subtotal_price: "100.00",
  total_tax: "10.00",
  total_price: "110.00",
  shipping_address: {...},
  financial_status: "paid"
}
```

**GnuCash Marketplace:**
```cpp
struct OrderInfo {
    std::string id;
    std::string customer_id;
    std::string seller_id;
    OrderStatus status;
    std::vector<OrderItem> items;
    double subtotal;
    double tax_amount;
    double shipping_cost;
    double total_amount;
    std::string currency;
    PaymentMethod payment_method;
    Address shipping_address;
    Address billing_address;
    // ... additional fields
};
```

## API Integration Examples

### REST API Wrapper (Future Implementation)

```cpp
// Example REST endpoint handlers using MarketplaceEngine

// GET /api/products
void handleGetProducts(Request& req, Response& res) {
    auto& engine = marketplace_engine();
    auto products = engine.list_products("", "", ProductStatus::ACTIVE);
    
    // Serialize to JSON
    nlohmann::json response = products;
    res.send(response.dump());
}

// POST /api/orders
void handleCreateOrder(Request& req, Response& res) {
    auto& engine = marketplace_engine();
    
    // Parse request body
    OrderInfo order = parseOrderFromJSON(req.body);
    
    std::string order_id = engine.create_order(order);
    
    res.json({{"order_id", order_id}});
}
```

### GraphQL Schema (Future Implementation)

```graphql
type Product {
  id: ID!
  seller_id: String!
  name: String!
  description: String
  base_price: Float!
  currency: String!
  status: ProductStatus!
  stock_quantity: Int!
  variants: [ProductVariant!]!
  images: [ProductImage!]!
}

type Query {
  products(seller_id: String, status: ProductStatus): [Product!]!
  product(id: ID!): Product
  searchProducts(query: String!, category: String): [Product!]!
}

type Mutation {
  createProduct(input: ProductInput!): Product!
  createOrder(input: OrderInput!): Order!
  updateOrderStatus(order_id: ID!, status: OrderStatus!): Order!
}
```

## Integration with Virtunomics Features

### Agent-Based Trading

```cpp
// Virtunomics has AI agents that make business decisions
// GnuCash uses OpenCog cognitive engine for similar functionality

#include <opencog/gnc-cognitive/cognitive_engine.hpp>

auto& cognitive = gnc::opencog::cognitive_engine();
auto& marketplace = gnc::marketplace::marketplace_engine();

// AI-powered decision making
auto recommendations = marketplace.get_recommendations(customer_id, 10);

// Pattern detection (like Virtunomics agent behavior)
auto patterns = cognitive.detect_spending_patterns();

// Predictive analytics
int demand = marketplace.predict_demand(product_id, 30);
double optimal_price = marketplace.suggest_price(product_id, market_data);
```

### Turn-Based Processing

Virtunomics processes game turns to update all entities. GnuCash marketplace can implement similar batch processing:

```cpp
void processTurn() {
    auto& engine = marketplace_engine();
    
    // 1. Update inventory from suppliers
    auto low_stock = engine.get_low_stock_products();
    for (const auto& item : low_stock) {
        // Trigger reorder
        engine.update_inventory(item.product_id, "", item.reorder_quantity);
    }
    
    // 2. Process pending orders
    auto pending_orders = engine.list_orders("", "", OrderStatus::PAID);
    for (const auto& order : pending_orders) {
        // Ship orders
        engine.update_order_status(order.id, OrderStatus::SHIPPED);
    }
    
    // 3. Update analytics
    auto start = std::chrono::system_clock::now() - std::chrono::hours(24);
    auto end = std::chrono::system_clock::now();
    auto analytics = engine.get_sales_analytics("", start, end);
    
    // 4. Generate insights
    auto& cognitive = gnc::opencog::cognitive_engine();
    auto insights = cognitive.generate_insights();
}
```

## Future Enhancements

### Phase 1: Database Integration
- [ ] PostgreSQL/MySQL backend (like Shopify with Sequelize)
- [ ] Migration from in-memory to persistent storage
- [ ] Transaction support and ACID compliance

### Phase 2: API Layer
- [ ] RESTful API endpoints
- [ ] GraphQL schema and resolvers
- [ ] WebSocket for real-time updates
- [ ] OAuth2 authentication

### Phase 3: Frontend
- [ ] React/Next.js admin dashboard (Shopify-inspired)
- [ ] Buyer-facing marketplace UI
- [ ] Real-time order tracking
- [ ] Product catalog with filters

### Phase 4: Advanced Features
- [ ] Multi-channel integration (Amazon, eBay, etc.)
- [ ] Advanced AI recommendations using OpenCog
- [ ] Business simulation mode (Virtunomics-inspired)
- [ ] Supply chain optimization
- [ ] Cryptocurrency payment support

### Phase 5: Mobile & Cloud
- [ ] Mobile apps (iOS/Android)
- [ ] Cloud deployment options
- [ ] Scalability improvements
- [ ] Load balancing and caching

## Running the Demo

To see the marketplace features in action:

```bash
# Build the marketplace module
cd /path/to/gnucash
mkdir build && cd build
cmake -G Ninja ..
ninja gnc-marketplace

# Run the demo
cd build
./libgnucash/marketplace/examples/marketplace-demo
```

Expected output:
```
GnuCash Marketplace Demo
============================================================

Marketplace engine initialized successfully!
...
[Shows complete workflow: storefront → products → orders → analytics]
```

## Contributing

To add features inspired by Shopify or Virtunomics:

1. Study the reference implementations in the GitHub repos
2. Design the C++ equivalent for GnuCash architecture
3. Add to appropriate manager class (Product, Order, etc.)
4. Update `marketplace_engine.hpp` with new public API
5. Add unit tests in `test/gtest-marketplace.cpp`
6. Update this integration guide

## References

- [Shopify Marketplace Kit Documentation](https://shopify.dev/marketplaces)
- [Shopify Admin App Source](https://github.com/Shopify/shopify-marketplaces-admin-app)
- [Shopify Buyer App Source](https://github.com/Shopify/shopify-marketplaces-buyer-app)
- [Virtunomics Source](https://github.com/o9nn/virtunomicog)
- [OpenCog Framework](https://opencog.org/)
- [GnuCash Development Guide](../../CLAUDE.md)
