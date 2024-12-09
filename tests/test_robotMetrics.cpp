#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "RobotMetrics/robot_metrics.h"
#include "Robot/Robot.h"
#include <memory>
#include <chrono>
#include <thread>

// First Test Case: Initialization and Basic Updates
TEST_CASE("Robot Metrics Initialization and Basic Updates") {
    auto robot = std::make_shared<Robot>("CleaningBot", 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, 50.0);
    RobotMetrics metrics(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);

    SECTION("Default Initialization") {
        REQUIRE(metrics.utilization == 0.0f);
        REQUIRE(metrics.errorRate == 0.0f);
        REQUIRE(metrics.costEfficiency == 1.0f);
        REQUIRE(metrics.timeEfficiency == 1.0f);
        REQUIRE(metrics.batteryUsage == 0.0f);
        REQUIRE(metrics.waterUsage == 0.0f);
    }

    SECTION("Metrics Update Verification") {
        metrics.utilization = 0.85f;
        metrics.errorRate = 0.03f;
        metrics.costEfficiency = 0.92f;
        metrics.timeEfficiency = 0.88f;
        metrics.batteryUsage = 0.45f;
        metrics.waterUsage = 0.2f;

        REQUIRE(metrics.utilization == 0.85f);
        REQUIRE(metrics.errorRate == 0.03f);
        REQUIRE(metrics.costEfficiency == 0.92f);
        REQUIRE(metrics.timeEfficiency == 0.88f);
        REQUIRE(metrics.batteryUsage == 0.45f);
        REQUIRE(metrics.waterUsage == 0.2f);
    }

    SECTION("Edge Case: Maximum and Minimum Values") {
        metrics.utilization = 1.0f;
        metrics.errorRate = 0.0f;
        metrics.batteryUsage = 1.0f;
        metrics.waterUsage = 1.0f;

        REQUIRE(metrics.utilization == 1.0f);
        REQUIRE(metrics.errorRate == 0.0f);
        REQUIRE(metrics.batteryUsage == 1.0f);
        REQUIRE(metrics.waterUsage == 1.0f);
    }
}

// Second Test Case: Advanced Metrics and Resource Usage
TEST_CASE("Robot Metrics Advanced Operations") {
    auto robot = std::make_shared<Robot>("CleaningBot", 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM, 50.0);
    RobotMetrics metrics(0.2f, 0.05f, 0.9f, 0.85f, 0.3f, 0.25f);

    SECTION("Battery Management") {
        REQUIRE(robot->getBatteryLevel() == 100);

        metrics.batteryUsage = 0.6f;
        REQUIRE(metrics.batteryUsage == 0.6f);

        metrics.batteryUsage = 0.95f;
        REQUIRE(metrics.batteryUsage > 0.9f); // Critical level check
    }

    SECTION("Efficiency Adjustments") {
        metrics.costEfficiency = 0.75f;
        metrics.timeEfficiency = 0.8f;

        REQUIRE(metrics.costEfficiency == 0.75f);
        REQUIRE(metrics.timeEfficiency == 0.8f);
    }

    SECTION("Resource Usage Tracking") {
        metrics.waterUsage = 0.4f;
        REQUIRE(metrics.waterUsage == 0.4f);

        metrics.utilization = 0.9f;
        REQUIRE(metrics.utilization == 0.9f);
    }

    SECTION("Error Rate Monitoring") {
        metrics.errorRate = 0.07f;
        REQUIRE(metrics.errorRate == 0.07f);

        metrics.errorRate = 0.2f;
        REQUIRE(metrics.errorRate > 0.15f); // High error threshold alert
    }

    SECTION("Simulation of Full Metrics Update") {
        metrics.utilization = 0.95f;
        metrics.errorRate = 0.01f;
        metrics.costEfficiency = 0.98f;
        metrics.timeEfficiency = 0.97f;
        metrics.batteryUsage = 0.75f;
        metrics.waterUsage = 0.5f;

        REQUIRE(metrics.utilization == 0.95f);
        REQUIRE(metrics.errorRate == 0.01f);
        REQUIRE(metrics.costEfficiency == 0.98f);
        REQUIRE(metrics.timeEfficiency == 0.97f);
        REQUIRE(metrics.batteryUsage == 0.75f);
        REQUIRE(metrics.waterUsage == 0.5f);
    }
}
