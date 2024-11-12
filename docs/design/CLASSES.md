# **Class Documentation**

## **Introduction**

This document outlines the class structure for the robot fleet management system.

## **Core Classes**

### **1. User Management Classes**

#### **User Class**
- **Purpose:** Represents a system user with associated role and permissions
- **Attributes:**
  - `id`: int - Unique identifier for the user
  - `name`: string - User's name
  - `role`: Role - User's assigned role
- **Methods:**
  - `login()`: Authenticates user into the system
  - `logout()`: Logs user out of the system
  - `receiveNotification(alert: Alert)`: Handles incoming alert notifications
  - Getters and setters for all attributes

#### **Role Class**
- **Purpose:** Defines user roles and associated permissions
- **Attributes:**
  - `name`: string - Name of the role
  - `permissions`: vector<Permission> - List of associated permissions
- **Methods:**
  - `addPermission(permission: Permission)`: Adds a permission to the role
  - `hasPermission(permissionName: string): bool`: Checks if role has specific permission
  - Getters and setters for name

#### **Permission Class**
- **Purpose:** Represents individual system permissions
- **Attributes:**
  - `name`: string - Name of the permission
- **Methods:**
  - Getters and setters for name

### **2. Alert System Classes**

#### **Alert Class**
- **Purpose:** Represents system alerts and notifications
- **Attributes:**
  - `type`: string - Alert type/title
  - `message`: string - Alert message/description
  - `robot`: shared_ptr<Robot> - Associated robot
  - `room`: shared_ptr<Room> - Associated room
  - `timestamp`: time_t - Time of alert creation
  - `severity`: Severity - Alert severity level (LOW, MEDIUM, HIGH)
- **Methods:**
  - `displayAlertInfo()`: Displays complete alert information
  - Getters and setters for all attributes

#### **AlertSystem Class**
- **Purpose:** Manages alert distribution and processing
- **Attributes:**
  - `alertQueue_`: queue<pair<User*, shared_ptr<Alert>>> - Queue of pending alerts
  - `workerThread_`: thread - Thread for processing alerts
  - `queueMutex_`: mutex - Mutex for thread safety
  - `cv_`: condition_variable - For thread synchronization
  - `running_`: atomic<bool> - Thread control flag
- **Methods:**
  - `sendAlert(user: User*, alert: shared_ptr<Alert>)`: Queues an alert for delivery
  - `stop()`: Stops the alert processing thread
  - `processAlerts()`: Internal method for processing queued alerts

### **3. Robot and Room Classes**

#### **Robot Class**
- **Purpose:** Represents a cleaning robot
- **Attributes:**
  - `name`: string - Robot identifier
  - `batteryLevel`: int - Current battery percentage
- **Methods:**
  - `sendStatusUpdate()`: Reports current robot status
  - `recharge()`: Recharges robot battery
  - `needsMaintenance()`: Checks if maintenance is required
  - `depleteBattery(amount: int)`: Reduces battery level
  - Getters for attributes

#### **Room Class**
- **Purpose:** Represents a cleanable room
- **Attributes:**
  - `roomName`: string - Room identifier
  - `roomId`: int - Unique room number
  - `occupied`: bool - Room occupancy status
- **Methods:**
  - `getRoomInfo()`: Displays room information
  - `isOccupied()`: Checks room occupancy
  - Getters and setters for attributes

### **4. Database Adapter Class**

#### **MongoDBAdapter Class**
- **Purpose:** Manages database operations for alerts and robot status
- **Attributes:**
  - `dbName_`: string - Database name
  - `saveQueue_`: queue<Alert> - Alert save queue
  - `robotStatusQueue_`: queue<shared_ptr<Robot>> - Robot status queue
  - Thread management members for both alert and robot status processing
- **Methods:**
  - Alert Operations:
    - `saveAlert(alert: Alert)`: Saves alert to database
    - `retrieveAlerts()`: Retrieves all alerts
    - `deleteAllAlerts()`: Clears alert collection
  - Robot Status Operations:
    - `saveRobotStatusAsync(robot: shared_ptr<Robot>)`: Saves robot status
    - `retrieveRobotStatuses()`: Retrieves all robot statuses
    - `deleteAllRobotStatuses()`: Clears robot status collection
  - Collection Management:
    - `dropAlertCollection()`: Removes alert collection
    - `dropRobotStatusCollection()`: Removes robot status collection
  - Thread Management:
    - `stop()`: Stops alert processing thread
    - `stopRobotStatusThread()`: Stops robot status processing thread

## **Class Relationships**

1. **User-Role-Permission Hierarchy:**
   - User has one Role
   - Role contains multiple Permissions
   - Permissions define access rights

2. **Alert System Integration:**
   - AlertSystem sends Alerts to Users
   - Alerts reference both Robot and Room
   - AlertSystem uses MongoDBAdapter for persistence

3. **Database Integration:**
   - MongoDBAdapter manages persistence for both Alerts and Robot status
   - Uses asynchronous processing for both alert and robot status updates

## **Design Patterns Used**

1. **Observer Pattern:**
   - AlertSystem acts as subject
   - Users are observers receiving notifications

2. **Singleton Pattern (implicit):**
   - AlertSystem and MongoDBAdapter are typically instantiated once

3. **Smart Pointer Usage:**
   - shared_ptr used for Robot and Room references in Alert class
   - Ensures proper memory management

## **Threading and Concurrency**

The system implements thread-safe operations through:
- Mutex locks for queue access
- Condition variables for thread synchronization
- Atomic flags for thread control
- Separate threads for alert and robot status processing

## **Future Enhancements**

Potential areas for expansion:
1. Implementation of a scheduling system
2. Addition of mapping and navigation capabilities
3. Enhanced analytics and reporting features

## **Class Diagram Visualized**

![class Diagram](../design/ClassDiagrams/classDiagram.png "ClassDiagram")
