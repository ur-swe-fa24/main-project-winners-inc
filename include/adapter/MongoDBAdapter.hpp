#ifndef MONGODB_ADAPTER_HPP
#define MONGODB_ADAPTER_HPP

#include "alert/Alert.h"
#include "Robot/Robot.h"
#include <mongocxx/client.hpp>
#include <string>
#include <vector>
#include <memory>

class MongoDBAdapter {
public:
    MongoDBAdapter(const std::string& uri, const std::string& dbName);
    ~MongoDBAdapter();

    // Alert methods
    void saveAlert(const Alert& alert);
    std::vector<Alert> retrieveAlerts();
    void deleteAllAlerts();
    void dropAlertCollection();

    // Robot status methods
    void saveRobotStatus(std::shared_ptr<Robot> robot);

    void deleteRobotStatus(const std::string& robotName);

    std::vector<std::shared_ptr<Robot>> retrieveRobotStatuses();
    void deleteAllRobotStatuses();
    void dropRobotStatusCollection();


private:
    std::string dbName_;
};

#endif // MONGODB_ADAPTER_HPP
