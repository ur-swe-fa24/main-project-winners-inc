#ifndef ROBOTMETRICS_H
#define ROBOTMETRICS_H

class RobotMetrics {
public:
    float utilization;
    float errorRate;
    float costEfficiency;
    float timeEfficiency;
    float batteryUsage;
    float waterUsage;

    RobotMetrics(float utilization, float errorRate, float costEfficiency,
                 float timeEfficiency, float batteryUsage, float waterUsage);
};

#endif // ROBOTMETRICS_H
