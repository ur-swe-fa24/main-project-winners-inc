#ifndef ROBOT_SIMULATOR_HPP
#define ROBOT_SIMULATOR_HPP

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Robot/Robot.h"
#include "map/map.h"
#include "adapter/MongoDBAdapter.hpp"

class RobotSimulator {
public:
    RobotSimulator(std::shared_ptr<MongoDBAdapter> dbAdapter, const std::string& mapFile);
    ~RobotSimulator();

    void start();
    void stop();

    struct RobotStatus {
        std::string name;
        int batteryLevel;
        int waterLevel;
        std::string status;
        std::string currentRoomName;
        
        RobotStatus(const std::string& n, int bl, int wl, const std::string& s, const std::string& crn)
            : name(n), batteryLevel(bl), waterLevel(wl), status(s), currentRoomName(crn) {}
    };

    std::vector<RobotStatus> getRobotStatuses();

    std::shared_ptr<Robot> getRobotByName(const std::string& name);

    void startCleaning(const std::string& robotName);
    void stopCleaning(const std::string& robotName);
    void returnToCharger(const std::string& robotName);

    Map& getMap();  // Non-const getter
    const Map& getMap() const;

    std::vector<std::shared_ptr<Robot>>& getRobots();  // Non-const getter
    const std::vector<std::shared_ptr<Robot>>& getRobots() const;

    void addRobot(const std::string& robotName);
    void deleteRobot(const std::string& robotName);

private:
    void simulationLoop();
    void simulateRobotMovement();
    Room* getNextRoomToClean(Room* currentRoom);

    std::vector<std::shared_ptr<Robot>> robots_;
    Map map_;
    std::shared_ptr<MongoDBAdapter> dbAdapter_;

    std::thread simulationThread_;
    std::mutex robotsMutex_;
    std::condition_variable cv_;
    bool running_;
};

#endif // ROBOT_SIMULATOR_HPP
