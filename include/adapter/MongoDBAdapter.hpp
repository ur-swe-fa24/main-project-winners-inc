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

class MongoDBAdapter {
  public:
    MongoDBAdapter(const std::string &uri, const std::string &dbName);
    ~MongoDBAdapter();

    // Alert methods
    void saveAlert(const Alert &alert);
    std::vector<Alert> retrieveAlerts();
    void deleteAllAlerts();
    void dropAlertCollection();

    // Robot status methods
    void saveRobotStatus(std::shared_ptr<Robot> robot);
    std::vector<std::shared_ptr<Robot>> retrieveRobotStatuses();
    void deleteAllRobotStatuses();
    void dropRobotStatusCollection();

    // Asynchronous robot status methods
    void saveRobotStatusAsync(std::shared_ptr<Robot> robot);
    void stopRobotStatusThread();

    // Thread management
    void stop();

  private:
    // Alert processing
    void processSaveQueue();

    // Robot status processing
    void processRobotStatusQueue();

    // Remove the client_ member variable
    // mongocxx::client client_; // Removed
    std::string dbName_;

    // Alert threading members
    std::queue<Alert> saveQueue_;
    std::thread dbThread_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_;

    // Robot status threading members
    std::queue<std::shared_ptr<Robot>> robotStatusQueue_;
    std::thread robotStatusThread_;
    std::mutex robotStatusMutex_;
    std::condition_variable robotStatusCV_;
    std::atomic<bool> robotStatusRunning_;
};

#endif // MONGODB_ADAPTER_HPP
