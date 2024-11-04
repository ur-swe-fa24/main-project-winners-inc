#include "RobotMetrics/robot_metrics.h"

RobotMetrics::RobotMetrics(float utilization, float errorRate, float costEfficiency,
                           float timeEfficiency, float batteryUsage, float waterUsage)
    : utilization(utilization), errorRate(errorRate), costEfficiency(costEfficiency),
      timeEfficiency(timeEfficiency), batteryUsage(batteryUsage), waterUsage(waterUsage) {}
