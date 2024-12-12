#ifndef ALERT_SYSTEM_H
#define ALERT_SYSTEM_H

#include <string>
#include <vector>
#include <mutex>

class AlertSystem {
public:
    void sendAlert(const std::string& message, const std::string& type);
    std::vector<std::string> getAlerts();

private:
    std::mutex mtx_;
    std::vector<std::string> alerts_;
};

#endif // ALERT_SYSTEM_H
