#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "analytics/analytics.h"
#include "RobotMetrics/robot_metrics.h"
#include "Robot/Robot.h"

TEST_CASE("RobotMetrics - Constructor and Attribute Access") {
    RobotMetrics metrics(90.5, 5.0, 0.8, 85.0, 100.0, 20.0);

    REQUIRE(metrics.utilization == Approx(90.5f));
    REQUIRE(metrics.errorRate == Approx(5.0f));
    REQUIRE(metrics.costEfficiency == Approx(0.8f));
    REQUIRE(metrics.timeEfficiency == Approx(85.0f));
    REQUIRE(metrics.batteryUsage == Approx(100.0f));
    REQUIRE(metrics.waterUsage == Approx(20.0f));
}

TEST_CASE("Analytics - Add and Retrieve Data") {
    Analytics analytics;
    Robot robot1("Robot1", 101);
    Robot robot2("Robot2", 102);

    RobotMetrics metrics1(85.0, 2.0, 0.9, 80.0, 90.0, 25.0);
    RobotMetrics metrics2(92.0, 1.0, 1.0, 95.0, 85.0, 20.0);

    // Add data to analytics (Assume addRobotData exists in Analytics)
    analytics.addRobotMetrics(robot1, metrics1);
    analytics.addRobotMetrics(robot2, metrics2);

    // Retrieve efficiency
    REQUIRE(analytics.getRobotEfficiency(robot1) == Approx(85.0f));
    REQUIRE(analytics.getRobotEfficiency(robot2) == Approx(92.0f));
}

TEST_CASE("Analytics - Generate Report") {
    Analytics analytics;
    Robot robot1("Robot1", 101);
    Robot robot2("Robot2", 102);

    RobotMetrics metrics1(85.0, 2.0, 0.9, 80.0, 90.0, 25.0);
    RobotMetrics metrics2(92.0, 1.0, 1.0, 95.0, 85.0, 20.0);

    // Add data to analytics
    analytics.addRobotMetrics(robot1, metrics1);
    analytics.addRobotMetrics(robot2, metrics2);

    // Capture report generation output (mocking std::cout in tests can be tricky)
    REQUIRE_NOTHROW(analytics.generateReport());
}
