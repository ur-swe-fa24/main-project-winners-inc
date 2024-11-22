#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <vector>
#include <memory>
#include <string>
#include "Robot/Robot.h"
#include "map/map.h"

class Scheduler {
public:
    Scheduler(Map* map = nullptr, std::vector<std::shared_ptr<Robot>>* robots = nullptr);

    void assignCleaningTask(const std::string& robotName, int targetRoomId, const std::string& cleaningStrategy);
    void update(); // Regularly called to update tasks
    void executeCleaning(std::shared_ptr<Robot> robot, Room* targetRoom, const std::string& strategy);
    
    void setMap(Map* map) { map_ = map; }
    void setRobots(std::vector<std::shared_ptr<Robot>>* robots) { robots_ = robots; }

private:
    Map* map_;
    std::vector<std::shared_ptr<Robot>>* robots_;
    int getCleaningTime(const Room& room) const;

    // Helper to find robot by name
    std::shared_ptr<Robot> findRobotByName(const std::string& name);
};

#endif // SCHEDULER_HPP
