#include "AlertSystem/alert_system.h"

void AlertSystem::sendAlert(const std::string& message, const std::string& type) {
    std::lock_guard<std::mutex> lock(mtx_);
    alerts_.push_back("[" + type + "] " + message);
}

std::vector<std::string> AlertSystem::getAlerts() {
    std::lock_guard<std::mutex> lock(mtx_);
    return alerts_;
}
