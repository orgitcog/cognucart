/*
 * libgnucash/marketplace/examples/marketplace_demo.cpp
 *
 * Demonstration of GnuCash Marketplace functionality
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "../marketplace_engine.hpp"
#include <iostream>
#include <iomanip>

using namespace gnc::marketplace;

void print_separator() {
    std::cout << "\n" << std::string(60, '=') << "\n\n";
}

int main()
{
    std::cout << "GnuCash Marketplace Demo\n";
    print_separator();

    // Initialize the marketplace engine
    auto& engine = marketplace_engine();
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize marketplace engine\n";
        return 1;
    }

    std::cout << "Marketplace engine initialized successfully!\n";
    print_separator();

    // Create a storefront
    std::cout << "Creating a storefront...\n";
    StorefrontInfo storefront;
    storefront.seller_id = "SELLER-001";
    storefront.name = "Tech Gadgets Store";
    storefront.description = "Your one-stop shop for the latest tech gadgets";
    storefront.email = "contact@techgadgets.com";
    storefront.phone = "+1-555-0123";
    storefront.business_name = "Tech Gadgets Inc.";

    std::string storefront_id = engine.save_storefront(storefront);
    std::cout << "Storefront created with ID: " << storefront_id << "\n";
    print_separator();

    // Create some products
    std::cout << "Adding products to the storefront...\n\n";

    ProductInfo laptop;
    laptop.seller_id = "SELLER-001";
    laptop.name = "UltraBook Pro 15";
    laptop.description = "High-performance laptop with 15-inch display";
    laptop.base_price = 1299.99;
    laptop.currency = "USD";
    laptop.status = ProductStatus::ACTIVE;
    laptop.stock_quantity = 25;
    laptop.weight_kg = 1.8;
    laptop.category_id = "electronics";

    std::string laptop_id = engine.create_product(laptop);
    std::cout << "  ✓ Laptop created (ID: " << laptop_id << ")\n";

    ProductInfo mouse;
    mouse.seller_id = "SELLER-001";
    mouse.name = "Wireless Mouse Pro";
    mouse.description = "Ergonomic wireless mouse with precision tracking";
    mouse.base_price = 49.99;
    mouse.currency = "USD";
    mouse.status = ProductStatus::ACTIVE;
    mouse.stock_quantity = 100;
    mouse.weight_kg = 0.1;
    mouse.category_id = "electronics";

    std::string mouse_id = engine.create_product(mouse);
    std::cout << "  ✓ Mouse created (ID: " << mouse_id << ")\n";

    ProductInfo keyboard;
    keyboard.seller_id = "SELLER-001";
    keyboard.name = "Mechanical Keyboard RGB";
    keyboard.description = "Premium mechanical keyboard with RGB lighting";
    keyboard.base_price = 129.99;
    keyboard.currency = "USD";
    keyboard.status = ProductStatus::ACTIVE;
    keyboard.stock_quantity = 50;
    keyboard.weight_kg = 0.8;
    keyboard.category_id = "electronics";

    std::string keyboard_id = engine.create_product(keyboard);
    std::cout << "  ✓ Keyboard created (ID: " << keyboard_id << ")\n";
    print_separator();

    // Create a customer
    std::cout << "Registering a customer...\n";
    CustomerInfo customer;
    customer.name = "John Doe";
    customer.email = "john.doe@email.com";
    customer.phone = "+1-555-0456";

    Address address;
    address.street = "123 Main Street";
    address.city = "Springfield";
    address.state = "IL";
    address.postal_code = "62701";
    address.country = "USA";
    address.phone = "+1-555-0456";
    customer.addresses.push_back(address);

    std::string customer_id = engine.save_customer(customer);
    std::cout << "Customer registered with ID: " << customer_id << "\n";
    print_separator();

    // Search for products
    std::cout << "Searching for products with 'keyboard'...\n";
    auto search_results = engine.search_products("keyboard");
    std::cout << "Found " << search_results.size() << " product(s):\n";
    for (const auto& product : search_results) {
        std::cout << "  - " << product.name << " ($" << std::fixed 
                  << std::setprecision(2) << product.base_price << ")\n";
    }
    print_separator();

    // Create an order
    std::cout << "Creating an order...\n";
    OrderInfo order;
    order.customer_id = customer_id;
    order.seller_id = "SELLER-001";
    order.status = OrderStatus::PENDING;

    // Add laptop to order
    OrderItem item1;
    item1.product_id = laptop_id;
    item1.product_name = "UltraBook Pro 15";
    item1.quantity = 1;
    item1.unit_price = 1299.99;
    item1.total_price = 1299.99;
    order.items.push_back(item1);

    // Add mouse to order
    OrderItem item2;
    item2.product_id = mouse_id;
    item2.product_name = "Wireless Mouse Pro";
    item2.quantity = 2;
    item2.unit_price = 49.99;
    item2.total_price = 99.98;
    order.items.push_back(item2);

    order.subtotal = 1399.97;
    order.tax_amount = 140.00;
    order.shipping_cost = 25.00;
    order.total_amount = 1564.97;
    order.currency = "USD";
    order.shipping_address = address;
    order.billing_address = address;

    std::string order_id = engine.create_order(order);
    std::cout << "Order created with ID: " << order_id << "\n";
    std::cout << "  Subtotal: $" << std::fixed << std::setprecision(2) 
              << order.subtotal << "\n";
    std::cout << "  Tax: $" << order.tax_amount << "\n";
    std::cout << "  Shipping: $" << order.shipping_cost << "\n";
    std::cout << "  Total: $" << order.total_amount << "\n";
    print_separator();

    // Process payment
    std::cout << "Processing payment...\n";
    bool payment_success = engine.process_payment(order_id, 
                                                  PaymentMethod::CREDIT_CARD, 
                                                  "TXN-20240112-001");
    if (payment_success) {
        std::cout << "Payment processed successfully!\n";
    }
    print_separator();

    // Update order status
    std::cout << "Updating order status to SHIPPED...\n";
    engine.update_order_status(order_id, OrderStatus::SHIPPED);
    std::cout << "Order status updated.\n";
    print_separator();

    // Check inventory
    std::cout << "Checking inventory levels...\n";
    auto laptop_inventory = engine.get_inventory(laptop_id);
    auto mouse_inventory = engine.get_inventory(mouse_id);
    std::cout << "  Laptop stock: " << laptop_inventory.quantity_available << " units\n";
    std::cout << "  Mouse stock: " << mouse_inventory.quantity_available << " units\n";
    print_separator();

    // Get recommendations
    std::cout << "Getting product recommendations for customer...\n";
    auto recommendations = engine.get_recommendations(customer_id, 3);
    std::cout << "Recommended products:\n";
    for (const auto& product : recommendations) {
        std::cout << "  - " << product.name << " ($" << std::fixed 
                  << std::setprecision(2) << product.base_price << ")\n";
    }
    print_separator();

    // Get marketplace statistics
    std::cout << "Marketplace Statistics:\n";
    auto stats = engine.get_stats();
    std::cout << "  Total Products: " << stats.total_products << "\n";
    std::cout << "  Active Products: " << stats.active_products << "\n";
    std::cout << "  Total Orders: " << stats.total_orders << "\n";
    std::cout << "  Total Revenue: $" << std::fixed << std::setprecision(2) 
              << stats.total_revenue << "\n";
    std::cout << "  Average Order Value: $" << stats.average_order_value << "\n";
    std::cout << "  Active Storefronts: " << stats.active_storefronts << "\n";
    print_separator();

    // Shutdown
    std::cout << "Demo completed successfully!\n";
    engine.shutdown();

    return 0;
}
