# CMSC322 – Software Engineering
# Sprint0 – Customer Interview Document
# Fall 2024

# Likely Users
+ Senior Management – Interested in metrics like robot utilization, error rates, cost efficiency (cost per square foot), time efficiency (seconds per square foot), battery usage and water usage per square foot.

+ Building Manager – Interested in assigning tasks to groups robots, viewing the status of robot tasks, viewing current robot status (like water level, battery, location), updating tasks mid-run, notifications when a job is completed.

+ Building Staff (AKA Operator) -  Interested in assigning cleaning tasks, viewing the status of robot tasks, viewing current robot status (like water level, battery, location), updating tasks mid-run, notifications when a job is completed.

+ Field Engineer – Same interests as Building Manager, but want more detail about errors and robot status, ability to fix robots, removing or adding robots to the system.

# Needs and Requirements
## User Needs and Requirements
### Senior Management
+ As a senior manager, I want to be able to access the **error rate** of each robot so that I am able to make deductions about robot efficiency and optimize performance. 
+ As a senior manager, I want to be able to access the **battery life** of each robot so that I am able to make deductions about robot efficiency and optimize performance. 
+ As a senior manager, I want to be able to access the **cost efficiency** of each robot so that I am able to make deductions about robot efficiency and optimize performance. 
+ As a senior manager, I want to be able to access the **time efficiency** of each robot so that I am able to make deductions about robot efficiency and optimize performance. 
+ As a senior manager, I want to be able to access the **rate of water usage** of each robot so that I am able to make deductions about robot efficiency and optimize performance. 

### Building Manager
+ As a building manager, I want to have the capacity to **create a daily cleaning schedule** so that routine tasks are automatically executed without manual intervention.
+ As a building manager, I want to have the capacity to **edit an existing daily cleaning schedule** so that routine tasks may be adjusted according to building needs.
+ As a building manager, I want to be able to **access the real-time status** (battery level, error rate, water level, cleaning progress) of each robot so that I can ensure that cleaning tasks are being completed appropriately and efficiently. 

### Building Staff
+ As a building staff member, I want to **assign specific zones to robots** so that I can ensure that designated areas are cleaned in a timely manner.
+ As a building staff member, I want to **assign a robot to spot clean** so that a specific spot that requires additional treatment may be addressed and appropriately cleaned.

### Field Engineer
+ As a field engineer, I want to **receive notifications when a robot's battery or water level becomes low** so that I am able to respond appropriately to the robot's needs.
+ As a field manager, I want to **access information about errors that occur** so that I am able to respond effectively.

### Robot
+ As a robot, I need to be able to **use different cleaning functions** so that I can tailor my cleaning to each room. 
+ As a robot, I want to **clean different room types** so that I am able to address cleanings most effectively. 

## Operational Needs and Requirements
+ As a robot, I need to be able to **take a map as an input** so that I am able to develop cleaning routes and navigate the room.
+ As a robot, I want to **take flooring type as an input** so that I am able to effectively choose which cleaning function to implement in that room. 
+ As a robot, I need to be able to **access elements in the map** so that I am able to mark rooms as visited (clean) or unvisited (dirty).
+ As a robot, I want to have the capacity to **send and receive information from the main operating system** so that I can update my status and receive new tasks effectively.
+ As a robot, I want to **check my own battery, water, and functionality statuses between tasks** so that I can determine whether I may continue, or if I must signal for maintenance.
___
+ As an alert system, I want to output an **alert to users when robot battery or water level is low** so that robot needs may be properly addressed.
+ As an alert system, I want to output an **alert to users when a task has been completed by the robot** so that they may be updated on cleaning progress.
+ As an alert system, I want to output **the location of the robot when an error arises** so that it may be rescued and taken for maintenance.
+ As an alert system, I want to output **a notification when a robot fails to complete a task** so that the task may be reassigned or rescheduled.
___
+ As a mapping system, I want to **output a working path into memory** so that the robot may follow the most efficient route during cleaning.
___
+ As a scheduling system, I want to **enqueue and dequeue tasks based on priority** so that the robots complete the highest priority tasks first.
+ As a scheduling system, I want to **take robot battery, water, and functionality statuses as input** so that I can appropriately assign tasks to robots that are equipped to take on new tasks.
+ As a scheduling system, I want to **create a virtual wall** so that I may prevent robots from entering restricted areas during their cleaning tasks. 
