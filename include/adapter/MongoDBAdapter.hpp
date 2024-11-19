#ifndef MONGODB_ADAPTER_HPP
#define MONGODB_ADAPTER_HPP

#include "Robot/Robot.h"
#include "alert/Alert.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mongocxx/client.hpp>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <memory>

class MongoDBAdapter {
  public:
    MongoDBAdapter(const std::string &uri, const std::string &dbName);
    ~MongoDBAdapter();

    // Alert methods
    void saveAlert(const Alert &alert);
    void saveAlertAsync(const Alert &alert);  // Async version
    std::vector<Alert> retrieveAlerts();
    void deleteAllAlerts();
    void dropAlertCollection();

    // Robot status methods
    void saveRobotStatus(std::shared_ptr<Robot> robot);
    void saveRobotStatusAsync(std::shared_ptr<Robot> robot);  // Async version
    void deleteRobotStatus(const std::string& robotName);
    std::vector<std::shared_ptr<Robot>> retrieveRobotStatuses();
    void deleteAllRobotStatuses();
    void dropRobotStatusCollection();

    // Thread management
    void stop();  // Stop all background threads
    void stopRobotStatusThread();  // Stop robot status monitoring thread

private:
    std::string dbName_;
    mongocxx::client client_;
    mongocxx::database db_;
    
    // Thread management
    std::atomic<bool> running_;
    std::thread robotStatusThread_;
    std::thread alertThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    
    // Queues for async operations
    std::queue<std::shared_ptr<Alert>> alertQueue_;
    std::queue<std::shared_ptr<Robot>> robotStatusQueue_;
    
    // Helper methods
    void processAlertQueue();
    void processRobotStatusQueue();
};

#endif // MONGODB_ADAPTER_HPP
