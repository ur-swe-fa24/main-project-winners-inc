#ifndef ANALYTICSSERVICE_H
#define ANALYTICSSERVICE_H

#include <string>
#include "analytics/analytics.h"
#include "User.h"

class AnalyticsService {
public:
    void viewAnalytics(const User& user);
};

#endif // ANALYTICSSERVICE_H
