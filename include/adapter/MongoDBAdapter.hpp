#ifndef MONGODB_ADAPTER_HPP
#define MONGODB_ADAPTER_HPP

#include "alert/Alert.h"
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <string>
#include <vector>

class MongoDBAdapter {
public:
    MongoDBAdapter(const std::string& uri, const std::string& dbName, const std::string& collectionName);

    // Methods to save and retrieve alerts
    void saveAlert(const Alert& alert);
    std::vector<Alert> retrieveAlerts();

private:
    mongocxx::client client_;
    mongocxx::collection collection_;
};

#endif // MONGODB_ADAPTER_HPP
