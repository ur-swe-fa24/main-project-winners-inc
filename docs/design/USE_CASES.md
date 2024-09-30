# Use Case Documentation

## Use Case 1: Alerts and Errors

### Actor: 
Field Engineer
### Description:
The Field Engineer is notified when a robot encounters an error or when its battery/water level is low, allowing them to take appropriate action.

### Preconditions:
The robot’s error detection system is active.
The Field Engineer is subscribed to receive alerts for errors and low battery/water levels.
### Basic Flow:
The system detects an error or low battery/water level in a robot.
The system sends a notification to the Field Engineer with the relevant information (e.g., robot’s location, type of issue).
The Field Engineer acknowledges the alert and takes corrective action.
### Postconditions:
The Field Engineer is informed of robot status and takes action to resolve the issue (e.g., initiate maintenance).
### Alternative Flows:
Notification Failure: If the alert fails to send, the system logs the failure and retries, or escalates to an admin user.

## Use Case 2: Manage Tasks (Scheduling)

### Actor: 
Building Manager
### Description:
The Building Manager can create, edit, and monitor daily cleaning schedules for the robots to ensure cleaning tasks are performed effectively.

### Preconditions:
The Building Manager is logged into the system.
Robots are available and functional.
### Basic Flow:
The Building Manager opens the scheduling interface.
The Building Manager selects "Create New Schedule" or chooses an existing schedule to edit.
The system presents a form to input/modify cleaning tasks and assign robots to different areas.
The system saves the new or updated schedule.
The Building Manager receives a confirmation.
### Postconditions:
The cleaning schedule is created or updated in the system.
### Alternative Flows:
Task Assignment Failure: If the system cannot save or assign tasks, an error message is displayed, and the Building Manager is prompted to retry or contact support.

## Use Case 3: Robot Analytics

### Actor: 
Senior Manager
### Description:
The Senior Manager wants to access analytics about robot performance, such as error rates, battery life, cost efficiency, and time efficiency.

### Preconditions:
The Senior Manager is logged into the system.
Robots are actively reporting performance data to the system.
### Basic Flow:
The Senior Manager opens the analytics dashboard.
The Senior Manager selects the type of analytics they wish to view (e.g., error rate, battery life, etc.).
The system retrieves the requested data and displays it in the UI.
The Senior Manager can view, download, or analyze the data for decision-making.
### Postconditions:
The Senior Manager has access to robot performance data for analysis.
### Alternative Flows:
Data Unavailable: If the system cannot retrieve the requested analytics, an error message is shown, and the Senior Manager can retry or contact IT.

## Use Case 4: Robot Functions

### Actor: 
Robot (automated)
### Description:
The robot needs to perform various functions such as checking battery life, water levels, and ensuring task completion.

### Preconditions:
The robot is operational and connected to the system.
The robot is assigned tasks via the scheduling system.
### Basic Flow:
The robot receives instructions from the system to perform cleaning tasks.
The robot periodically checks its battery, water level, and operational status.
If the robot detects low resources (battery or water), it sends a signal to the system for maintenance or recharge.
The robot marks tasks as completed or alerts the system in case of a failure.
### Postconditions:
The robot completes its assigned tasks or signals for intervention when necessary.
### Alternative Flows:
Error Detected: If the robot encounters an operational error, it sends an alert to the system for a Field Engineer to resolve the issue.
