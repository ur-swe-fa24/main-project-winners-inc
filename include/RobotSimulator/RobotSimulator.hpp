#ifndef ROBOT_SIMULATOR_HPP
#define ROBOT_SIMULATOR_HPP

#include <vector>
#include <memory>
#include <string>

class Robot;
class Scheduler;
class AlertSystem;
class Map;
class CleaningTask;

class RobotSimulator {
public:
    RobotSimulator(std::shared_ptr<Map> map,
                   std::shared_ptr<Scheduler> scheduler,
                   std::shared_ptr<AlertSystem> alertSystem);

    void addRobot(const std::shared_ptr<Robot>& robot);
    void update(double deltaTime);

    void assignTaskToRobot(const std::string& robotName, std::shared_ptr<CleaningTask> task);
    void requestReturnToCharger(const std::string& robotName);

    struct RobotStatus {
        std::string name;
        double batteryLevel;
        double waterLevel;
        std::string currentRoomName;
        bool isCleaning;
        bool needsCharging;
        std::string status;
    };

    std::vector<RobotStatus> getRobotStatuses() const;

    const Map& getMap() const; // Added to allow scheduler_panel and others to get rooms

private:
    std::vector<std::shared_ptr<Robot>> robots_;
    std::shared_ptr<Map> map_;
    std::shared_ptr<Scheduler> scheduler_;
    std::shared_ptr<AlertSystem> alertSystem_;

    void checkRobotStatesAndSendAlerts();
};

#endif // ROBOT_SIMULATOR_HPP
