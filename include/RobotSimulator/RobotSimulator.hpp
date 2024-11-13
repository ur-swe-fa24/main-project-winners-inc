// RobotSimulator.hpp

#ifndef ROBOT_SIMULATOR_HPP
#define ROBOT_SIMULATOR_HPP

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "Robot/Robot.h"
#include "adapter/MongoDBAdapter.hpp"
#include "map/map.h"

class RobotSimulator {
public:
    RobotSimulator(std::shared_ptr<MongoDBAdapter> dbAdapter, const std::string& mapFile);
    ~RobotSimulator();

    void start();
    void stop();

    // Methods to interact with the simulator
    std::shared_ptr<Robot> getRobotByName(const std::string& name);
    void startCleaning(const std::string& robotName);
    void stopCleaning(const std::string& robotName);
    void returnToCharger(const std::string& robotName);
    void addRobot(const std::string& robotName);
    void deleteRobot(const std::string& robotName);


    // Nested struct for robot status
    struct RobotStatus {
        std::string name;
        int batteryLevel;
        bool isCleaning;
        std::string currentRoomName;
    };

    std::vector<RobotStatus> getRobotStatuses();
    const std::vector<std::shared_ptr<Robot>>& getRobots() const;
    const Map& getMap() const;

private:
    void simulationLoop();
    void simulateRobotMovement();
    Room* getNextRoomToClean(Room* currentRoom);

    std::shared_ptr<MongoDBAdapter> dbAdapter_;
    std::vector<std::shared_ptr<Robot>> robots_;
    Map map_;

    std::thread simulationThread_;
    std::atomic<bool> running_;

    // Concurrency control
    std::mutex robotsMutex_;
    std::condition_variable cv_;
};

#endif // ROBOT_SIMULATOR_HPP
