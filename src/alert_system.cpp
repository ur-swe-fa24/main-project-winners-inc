// In src/alert_system.cpp
#include "alert_system/alert_system.h"
#include <iostream>

AlertSystem::AlertSystem() : running_(true) {
    workerThread_ = std::thread(&AlertSystem::processAlerts, this);
}

AlertSystem::~AlertSystem() {
    stop();
}

void AlertSystem::sendAlert(User* user, std::shared_ptr<Alert> alert) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        alertQueue_.emplace(user, alert);
    }
    cv_.notify_one();
}

void AlertSystem::processAlerts() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        cv_.wait(lock, [this]() { return !alertQueue_.empty() || !running_; });

        while (!alertQueue_.empty()) {
            auto [user, alert] = alertQueue_.front();
            alertQueue_.pop();
            lock.unlock();

            if (user && alert) {
                user->receiveNotification(*alert);
                std::cout << "Alert sent to user: " << user->getName() << std::endl;
            } else {
                std::cout << "Failed to send alert. User or alert is null." << std::endl;
            }

            lock.lock();
        }
    }
}

void AlertSystem::stop() {
    running_ = false;
    cv_.notify_all();
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}
