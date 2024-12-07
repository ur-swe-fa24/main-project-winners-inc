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

    void update(double deltaTime);

    // Add these methods to control robots
    void moveRobotToRoom(const std::string& robotName, int roomId);
    void startRobotCleaning(const std::string& robotName);
    void stopRobotCleaning(const std::string& robotName);
    void manuallyPickUpRobot(const std::string& robotName);
    void requestReturnToCharger(const std::string& robotName);
    void addRobot(const std::string& robotName);
    const std::vector<std::shared_ptr<Robot>>& getRobots() const;


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
    const Map& getMap() const;
    
    // Return alertSystem
    std::shared_ptr<AlertSystem> getAlertSystem() const;

private:
    std::vector<std::shared_ptr<Robot>> robots_;
    std::shared_ptr<Map> map_;
    std::shared_ptr<Scheduler> scheduler_;
    std::shared_ptr<AlertSystem> alertSystem_;

    void checkRobotStatesAndSendAlerts();
    std::shared_ptr<Robot> getRobotByName(const std::string& name); // add this helper

};

#endif // ROBOT_SIMULATOR_HPP
