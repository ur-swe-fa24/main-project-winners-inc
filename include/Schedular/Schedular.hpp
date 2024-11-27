#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <vector>
#include <memory>
#include <string>
#include <queue>
#include "Robot/Robot.h"
#include "map/map.h"
#include "CleaningTask/cleaningTask.h"

class Scheduler {
public:
    Scheduler(Map* map = nullptr, std::vector<std::shared_ptr<Robot>>* robots = nullptr);

    void assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& cleaningStrategy);
    void update(); // Regularly called to update tasks
    void executeCleaning(std::shared_ptr<Robot> robot, Room* targetRoom, const std::string& strategy);
    
    void setMap(Map* map) { map_ = map; }
    void setRobots(std::vector<std::shared_ptr<Robot>>* robots) { robots_ = robots; }

    // Helper function to get available strategies for a room type
    static std::vector<std::string> getAvailableStrategies(const std::string& floorType) {
        std::vector<std::string> strategies;
        if (floorType == "Carpet") {
            strategies = {"Vacuum", "Shampoo"};
        } else if (floorType == "Wood" || floorType == "Tile") {
            strategies = {"Vacuum", "Scrub"};
        }
        return strategies;
    }

private:
    Map* map_;
    std::vector<std::shared_ptr<Robot>>* robots_;
    int getCleaningTime(const Room& room) const;

    // Helper to find robot by name
    std::shared_ptr<Robot> findRobotByName(const std::string& name);
    
    // Helper function to convert string to CleanType enum
    static CleaningTask::CleanType cleaningStrategyFromString(const std::string& strategy);
};

#endif // SCHEDULER_HPP
