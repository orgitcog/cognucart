/*
 * gtest-cogutil.cpp
 *
 * Exhaustive unit tests for cogutil components:
 * - Counter
 * - Logger
 * - ConcurrentQueue
 *
 * Copyright (C) 2024 GnuCash Developers
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <chrono>
#include "../cogutil/counter.hpp"
#include "../cogutil/logger.hpp"
#include "../cogutil/concurrent_queue.hpp"

using namespace gnc::opencog;

// ============================================================
// Counter Tests
// ============================================================

class CounterTest : public ::testing::Test
{
protected:
    Counter<std::string> counter;
};

TEST_F(CounterTest, DefaultConstruction)
{
    EXPECT_TRUE(counter.empty());
    EXPECT_EQ(counter.size(), 0);
    EXPECT_EQ(counter.total(), 0);
}

TEST_F(CounterTest, InitializerListConstruction)
{
    Counter<std::string> c{"apple", "banana", "apple", "apple", "cherry"};

    EXPECT_EQ(c.count("apple"), 3);
    EXPECT_EQ(c.count("banana"), 1);
    EXPECT_EQ(c.count("cherry"), 1);
    EXPECT_EQ(c.size(), 3);
    EXPECT_EQ(c.total(), 5);
}

TEST_F(CounterTest, RangeConstruction)
{
    std::vector<int> data = {1, 2, 2, 3, 3, 3};
    Counter<int> c(data.begin(), data.end());

    EXPECT_EQ(c.count(1), 1);
    EXPECT_EQ(c.count(2), 2);
    EXPECT_EQ(c.count(3), 3);
}

TEST_F(CounterTest, Increment)
{
    counter.increment("test");
    EXPECT_EQ(counter.count("test"), 1);

    counter.increment("test");
    EXPECT_EQ(counter.count("test"), 2);

    counter.increment("test", 5);
    EXPECT_EQ(counter.count("test"), 7);
}

TEST_F(CounterTest, Decrement)
{
    counter.increment("test", 10);

    counter.decrement("test");
    EXPECT_EQ(counter.count("test"), 9);

    counter.decrement("test", 5);
    EXPECT_EQ(counter.count("test"), 4);

    // Decrement more than available should remove
    counter.decrement("test", 10);
    EXPECT_EQ(counter.count("test"), 0);
    EXPECT_FALSE(counter.contains("test"));
}

TEST_F(CounterTest, DecrementNonExistent)
{
    // Should not crash or add entry
    counter.decrement("nonexistent");
    EXPECT_FALSE(counter.contains("nonexistent"));
    EXPECT_TRUE(counter.empty());
}

TEST_F(CounterTest, Count)
{
    counter.increment("a", 5);
    counter.increment("b", 3);

    EXPECT_EQ(counter.count("a"), 5);
    EXPECT_EQ(counter.count("b"), 3);
    EXPECT_EQ(counter.count("c"), 0);  // Non-existent returns 0
}

TEST_F(CounterTest, OperatorBracket)
{
    counter["test"] = 10;
    EXPECT_EQ(counter.count("test"), 10);

    counter["test"] += 5;
    EXPECT_EQ(counter.count("test"), 15);

    // Creates entry if not present
    counter["new"] = 1;
    EXPECT_TRUE(counter.contains("new"));
}

TEST_F(CounterTest, Contains)
{
    counter.increment("exists");

    EXPECT_TRUE(counter.contains("exists"));
    EXPECT_FALSE(counter.contains("does_not_exist"));
}

TEST_F(CounterTest, Total)
{
    counter.increment("a", 10);
    counter.increment("b", 20);
    counter.increment("c", 30);

    EXPECT_EQ(counter.total(), 60);
}

TEST_F(CounterTest, Size)
{
    EXPECT_EQ(counter.size(), 0);

    counter.increment("a");
    counter.increment("b");
    counter.increment("c");

    EXPECT_EQ(counter.size(), 3);
}

TEST_F(CounterTest, Empty)
{
    EXPECT_TRUE(counter.empty());

    counter.increment("test");
    EXPECT_FALSE(counter.empty());

    counter.decrement("test");
    EXPECT_TRUE(counter.empty());
}

TEST_F(CounterTest, Clear)
{
    counter.increment("a", 10);
    counter.increment("b", 20);

    counter.clear();

    EXPECT_TRUE(counter.empty());
    EXPECT_EQ(counter.size(), 0);
    EXPECT_EQ(counter.total(), 0);
}

TEST_F(CounterTest, MostCommon)
{
    counter.increment("rare", 1);
    counter.increment("common", 10);
    counter.increment("medium", 5);

    auto top2 = counter.most_common(2);

    EXPECT_EQ(top2.size(), 2);
    EXPECT_EQ(top2[0].first, "common");
    EXPECT_EQ(top2[0].second, 10);
    EXPECT_EQ(top2[1].first, "medium");
    EXPECT_EQ(top2[1].second, 5);
}

TEST_F(CounterTest, MostCommonAll)
{
    counter.increment("a", 3);
    counter.increment("b", 2);
    counter.increment("c", 1);

    auto all = counter.most_common();  // n=0 means all

    EXPECT_EQ(all.size(), 3);
    EXPECT_EQ(all[0].first, "a");
}

TEST_F(CounterTest, LeastCommon)
{
    counter.increment("rare", 1);
    counter.increment("common", 10);
    counter.increment("medium", 5);

    auto bottom2 = counter.least_common(2);

    EXPECT_EQ(bottom2.size(), 2);
    EXPECT_EQ(bottom2[0].first, "rare");
    EXPECT_EQ(bottom2[0].second, 1);
}

TEST_F(CounterTest, OperatorPlusEquals)
{
    Counter<std::string> other;
    counter.increment("a", 5);
    other.increment("a", 3);
    other.increment("b", 2);

    counter += other;

    EXPECT_EQ(counter.count("a"), 8);
    EXPECT_EQ(counter.count("b"), 2);
}

TEST_F(CounterTest, OperatorMinusEquals)
{
    Counter<std::string> other;
    counter.increment("a", 10);
    counter.increment("b", 5);
    other.increment("a", 3);
    other.increment("b", 10);  // More than available

    counter -= other;

    EXPECT_EQ(counter.count("a"), 7);
    EXPECT_FALSE(counter.contains("b"));  // Removed because b became <= 0
}

TEST_F(CounterTest, Iterators)
{
    counter.increment("a", 1);
    counter.increment("b", 2);

    int total = 0;
    for (const auto& [key, count] : counter) {
        total += count;
    }

    EXPECT_EQ(total, 3);
}

TEST_F(CounterTest, ConstIterators)
{
    counter.increment("test", 5);

    const Counter<std::string>& const_counter = counter;

    int total = 0;
    for (auto it = const_counter.cbegin(); it != const_counter.cend(); ++it) {
        total += it->second;
    }

    EXPECT_EQ(total, 5);
}

TEST_F(CounterTest, CustomTypes)
{
    Counter<int> int_counter;
    int_counter.increment(42, 10);
    int_counter.increment(-1, 5);

    EXPECT_EQ(int_counter.count(42), 10);
    EXPECT_EQ(int_counter.count(-1), 5);
}

// ============================================================
// Logger Tests
// ============================================================

class LoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Capture log output
    }
};

TEST_F(LoggerTest, DefaultLogLevel)
{
    Logger logger;
    EXPECT_EQ(logger.get_level(), LogLevel::INFO);
}

TEST_F(LoggerTest, SetLogLevel)
{
    Logger logger;

    logger.set_level(LogLevel::DEBUG);
    EXPECT_EQ(logger.get_level(), LogLevel::DEBUG);

    logger.set_level(LogLevel::ERROR);
    EXPECT_EQ(logger.get_level(), LogLevel::ERROR);
}

TEST_F(LoggerTest, LogLevelFiltering)
{
    Logger logger;
    logger.set_level(LogLevel::WARN);

    // These should be filtered (level too low)
    EXPECT_FALSE(logger.should_log(LogLevel::DEBUG));
    EXPECT_FALSE(logger.should_log(LogLevel::INFO));

    // These should be logged
    EXPECT_TRUE(logger.should_log(LogLevel::WARN));
    EXPECT_TRUE(logger.should_log(LogLevel::ERROR));
}

TEST_F(LoggerTest, GlobalLogger)
{
    Logger& global = Logger::get();
    global.set_level(LogLevel::DEBUG);
    EXPECT_EQ(global.get_level(), LogLevel::DEBUG);
}

// ============================================================
// ConcurrentQueue Tests
// ============================================================

class ConcurrentQueueTest : public ::testing::Test
{
protected:
    ConcurrentQueue<int> queue;
};

TEST_F(ConcurrentQueueTest, DefaultConstruction)
{
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(ConcurrentQueueTest, PushAndPop)
{
    queue.push(42);
    EXPECT_EQ(queue.size(), 1);
    EXPECT_FALSE(queue.empty());

    int value;
    EXPECT_TRUE(queue.pop(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(queue.empty());
}

TEST_F(ConcurrentQueueTest, TryPop)
{
    int value;
    EXPECT_FALSE(queue.try_pop(value));  // Empty queue

    queue.push(10);
    EXPECT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 10);
}

TEST_F(ConcurrentQueueTest, FIFO_Order)
{
    queue.push(1);
    queue.push(2);
    queue.push(3);

    int value;
    queue.pop(value);
    EXPECT_EQ(value, 1);

    queue.pop(value);
    EXPECT_EQ(value, 2);

    queue.pop(value);
    EXPECT_EQ(value, 3);
}

TEST_F(ConcurrentQueueTest, Clear)
{
    queue.push(1);
    queue.push(2);
    queue.push(3);

    queue.clear();

    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(ConcurrentQueueTest, Cancel)
{
    queue.cancel();

    int value;
    EXPECT_FALSE(queue.pop(value));  // Should return immediately
}

TEST_F(ConcurrentQueueTest, ConcurrentPushPop)
{
    const int num_items = 1000;
    std::atomic<int> pop_count{0};

    // Producer thread
    std::thread producer([this, num_items]() {
        for (int i = 0; i < num_items; ++i) {
            queue.push(i);
        }
    });

    // Consumer thread
    std::thread consumer([this, &pop_count, num_items]() {
        int value;
        while (pop_count < num_items) {
            if (queue.try_pop(value)) {
                pop_count++;
            }
            std::this_thread::yield();
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(pop_count, num_items);
    EXPECT_TRUE(queue.empty());
}

TEST_F(ConcurrentQueueTest, MultipleProducersConsumers)
{
    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 100;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Start consumers first
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([this, &consumed]() {
            int value;
            while (true) {
                if (queue.try_pop(value)) {
                    consumed++;
                }
                if (consumed >= num_producers * items_per_producer)
                    break;
                std::this_thread::yield();
            }
        });
    }

    // Start producers
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([this, &produced, items_per_producer]() {
            for (int j = 0; j < items_per_producer; ++j) {
                queue.push(j);
                produced++;
            }
        });
    }

    for (auto& t : producers)
        t.join();

    // Wait for consumers to finish
    for (auto& t : consumers)
        t.join();

    EXPECT_EQ(produced, num_producers * items_per_producer);
    EXPECT_GE(consumed, num_producers * items_per_producer);
}

TEST_F(ConcurrentQueueTest, WaitPop)
{
    std::thread producer([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        queue.push(42);
    });

    int value;
    auto start = std::chrono::steady_clock::now();
    EXPECT_TRUE(queue.pop(value));  // Should block until producer pushes
    auto end = std::chrono::steady_clock::now();

    EXPECT_EQ(value, 42);
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), 40);

    producer.join();
}

TEST_F(ConcurrentQueueTest, CancelUnblocksWaitingThread)
{
    std::atomic<bool> pop_returned{false};

    std::thread waiting([this, &pop_returned]() {
        int value;
        queue.pop(value);  // Will block
        pop_returned = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.cancel();  // Unblock the waiting thread

    waiting.join();

    EXPECT_TRUE(pop_returned);
}

// ============================================================
// Additional Edge Case Tests
// ============================================================

TEST(CounterEdgeCases, EmptyMostCommon)
{
    Counter<std::string> c;
    auto result = c.most_common(5);
    EXPECT_TRUE(result.empty());
}

TEST(CounterEdgeCases, ZeroCount)
{
    Counter<int> c;
    c[42] = 0;
    EXPECT_FALSE(c.contains(42));  // Zero count shouldn't "contain"
}

TEST(ConcurrentQueueEdgeCases, CustomType)
{
    struct CustomData {
        int id;
        std::string name;
    };

    ConcurrentQueue<CustomData> q;
    q.push({1, "test"});

    CustomData data;
    EXPECT_TRUE(q.pop(data));
    EXPECT_EQ(data.id, 1);
    EXPECT_EQ(data.name, "test");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
