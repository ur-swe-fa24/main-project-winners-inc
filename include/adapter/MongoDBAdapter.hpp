#ifndef MONGODB_ADAPTER_HPP
#define MONGODB_ADAPTER_HPP

#include "Robot/Robot.h"
#include "alert/Alert.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <memory>
#include "Room/Room.h"

class MongoDBAdapter {
public:
    MongoDBAdapter(const std::string& uri, const std::string& dbName);
    ~MongoDBAdapter();

    // Alert methods
    void saveAlert(const Alert& alert);
    void saveAlertAsync(const Alert& alert);  // Async version
    std::vector<Alert> retrieveAlerts();
    void deleteAllAlerts();
    void dropAlertCollection();
    void dropRoomsCollection();


    // Robot status methods
    void saveRobotStatus(std::shared_ptr<Robot> robot);
    void saveRobotStatusAsync(std::shared_ptr<Robot> robot);  // Async version
    void deleteRobotStatus(const std::string& robotName);
    std::vector<std::shared_ptr<Robot>> retrieveRobotStatuses();
    void deleteAllRobotStatuses();
    void dropRobotStatusCollection();

    // Room methods
    void saveRoomStatus(const Room& room);
    void saveRoomStatusAsync(const Room& room);
    void loadRoomStatuses(std::vector<Room*>& rooms); // Update the rooms vector with status from DB
    void initializeRooms(const std::vector<Room*>& rooms); // Initialize rooms in DB if not present

    // Thread management
    void stop();  // Stop all background threads
    void stopRobotStatusThread();  // Stop robot status monitoring thread
    
    void saveRobotAnalytics(std::shared_ptr<Robot> robot);
    std::vector<std::tuple<std::string,int,double>> retrieveRobotAnalytics(); 

private:
    std::string dbName_;
    mongocxx::client client_;
    mongocxx::database db_;

    // Thread management
    std::atomic<bool> running_;
    std::thread robotStatusThread_;
    std::thread alertThread_;
    std::thread roomThread_; // New thread for room operations
    std::mutex mutex_;
    std::condition_variable cv_;

    // Queues for async operations
    std::queue<std::shared_ptr<Alert>> alertQueue_;
    std::queue<std::shared_ptr<Robot>> robotStatusQueue_;
    std::queue<std::shared_ptr<Room>> roomQueue_; // New queue for room operations

    // Helper methods
    void processAlertQueue();
    void processRobotStatusQueue();
    void processRoomQueue(); // New helper method for processing room queue
};

#endif // MONGODB_ADAPTER_HPP
