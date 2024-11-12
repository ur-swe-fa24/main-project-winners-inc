// In AlertSystem/alert_system.h
#ifndef ALERT_SYSTEM_H
#define ALERT_SYSTEM_H

#include "user/user.h"
#include "alert/Alert.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

class AlertSystem {
public:
    AlertSystem();
    ~AlertSystem();

    void sendAlert(User* user, std::shared_ptr<Alert> alert);
    void stop();

private:
    void processAlerts();

    std::queue<std::pair<User*, std::shared_ptr<Alert>>> alertQueue_;
    std::thread workerThread_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
};

#endif // ALERT_SYSTEM_H
