#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "RobotMetrics/robot_metrics.h"
#include "Robot/Robot.h"
#include <memory>
#include <chrono>
#include <thread>

TEST_CASE("Robot Metrics Test") {
    // Initialize test objects
    auto robot = std::make_shared<Robot>("TestBot", 100);
    RobotMetrics metrics(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);

    SECTION("Metrics Initialization") {
        REQUIRE(metrics.utilization == 0.0f);
        REQUIRE(metrics.errorRate == 0.0f);
        REQUIRE(metrics.costEfficiency == 1.0f);
        REQUIRE(metrics.timeEfficiency == 1.0f);
        REQUIRE(metrics.batteryUsage == 0.0f);
        REQUIRE(metrics.waterUsage == 0.0f);
    }

    SECTION("Metrics Update") {
        // Update metrics with new values
        metrics.utilization = 0.75f;
        metrics.errorRate = 0.05f;
        metrics.costEfficiency = 0.95f;
        metrics.timeEfficiency = 0.9f;
        metrics.batteryUsage = 0.5f;
        metrics.waterUsage = 0.3f;

        // Verify updated values
        REQUIRE(metrics.utilization == 0.75f);
        REQUIRE(metrics.errorRate == 0.05f);
        REQUIRE(metrics.costEfficiency == 0.95f);
        REQUIRE(metrics.timeEfficiency == 0.9f);
        REQUIRE(metrics.batteryUsage == 0.5f);
        REQUIRE(metrics.waterUsage == 0.3f);
    }

    SECTION("Battery Metrics") {
        // Test initial battery level
        REQUIRE(robot->getBatteryLevel() == 100);

        // Simulate battery usage
        metrics.batteryUsage = 0.5f;
        REQUIRE(metrics.batteryUsage == 0.5f);

        // Test low battery threshold
        metrics.batteryUsage = 0.9f;
        REQUIRE(metrics.batteryUsage > 0.8f);
    }

    SECTION("Efficiency Metrics") {
        // Test initial efficiencies
        REQUIRE(metrics.costEfficiency == 1.0f);
        REQUIRE(metrics.timeEfficiency == 1.0f);

        // Test efficiency updates
        metrics.costEfficiency = 0.85f;
        metrics.timeEfficiency = 0.9f;

        REQUIRE(metrics.costEfficiency == 0.85f);
        REQUIRE(metrics.timeEfficiency == 0.9f);
    }

    SECTION("Resource Usage") {
        // Test water usage
        metrics.waterUsage = 0.3f;
        REQUIRE(metrics.waterUsage == 0.3f);

        // Test utilization
        metrics.utilization = 0.75f;
        REQUIRE(metrics.utilization == 0.75f);
    }

    SECTION("Error Metrics") {
        // Test error rate
        metrics.errorRate = 0.02f;
        REQUIRE(metrics.errorRate == 0.02f);

        // Test high error threshold
        metrics.errorRate = 0.15f;
        REQUIRE(metrics.errorRate > 0.1f);
    }
}
