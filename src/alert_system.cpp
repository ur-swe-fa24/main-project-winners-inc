#include "AlertSystem/alert_system.h"
#include "iostream"

void AlertSystem::sendAlert(const std::string& message, const std::string& type) {
    if (message.empty() || type.empty()) {
        std::cout << "Warning: Null user or alert provided to sendAlert." << std::endl;
        return;  // Early exit to avoid adding invalid entries
    }
    std::lock_guard<std::mutex> lock(mtx_);
    alerts_.push_back("[" + type + "] " + message);
}

std::vector<std::string> AlertSystem::getAlerts() {
    std::lock_guard<std::mutex> lock(mtx_);
    return alerts_;
}
