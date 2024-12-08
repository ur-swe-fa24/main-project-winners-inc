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
class MongoDBAdapter;

class RobotSimulator {
public:
    RobotSimulator(std::shared_ptr<Map> map,
                   std::shared_ptr<Scheduler> scheduler,
                   std::shared_ptr<AlertSystem> alertSystem,
                   std::shared_ptr<MongoDBAdapter> dbAdapter);

    void update(double deltaTime);
    void moveRobotToRoom(const std::string& robotName, int roomId);
    void startRobotCleaning(const std::string& robotName);
    void stopRobotCleaning(const std::string& robotName);
    void manuallyPickUpRobot(const std::string& robotName);
    void requestReturnToCharger(const std::string& robotName);
    void addRobot(const std::string& robotName);

    // Now return a non-const reference so we can modify the vector
    std::vector<std::shared_ptr<Robot>>& getRobots();

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
    std::shared_ptr<AlertSystem> getAlertSystem() const;

    void assignTaskToRobot(std::shared_ptr<CleaningTask> task);
    std::shared_ptr<MongoDBAdapter> getDbAdapter() const {
        return dbAdapter_;
    }

private:
    std::vector<std::shared_ptr<Robot>> robots_;
    std::shared_ptr<Map> map_;
    std::shared_ptr<Scheduler> scheduler_;
    std::shared_ptr<AlertSystem> alertSystem_;
    std::shared_ptr<MongoDBAdapter> dbAdapter_;

    void checkRobotStatesAndSendAlerts();
    void handleNoTaskAndReturnToChargerIfNeeded(std::shared_ptr<Robot> robot);
    std::shared_ptr<Robot> getRobotByName(const std::string& name);
};

#endif // ROBOT_SIMULATOR_HPP
