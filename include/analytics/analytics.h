#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <map>
#include "Robot/Robot.h"
#include "RobotMetrics/robot_metrics.h"

class Analytics {
private:
    std::map<Robot, RobotMetrics> robotData;

public:
    void generateReport();
    float getRobotEfficiency(const Robot& robot);
};

#endif // ANALYTICS_H
