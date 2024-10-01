# User Interface Design 
## Introduction
The following description documents our intended implementation of the user interface (UI) for the system we will be designing for Cobotiq. The UI for the software is designed to uphold appropriate design principles such as simplicity, consistency, feedback, and error prevention and recovery. Users should be able to easily manage robot functions, monitor robot performance, and receive alerts and errors when appropriate. The Main Menu serves as the hub where the user is able to find all essential functions and submit requests efficiently.

## Home Screen/ Dashboard
Key Features:
1. Hourly schedule displaying upcoming cleaning tasks. Each schedule block where there is a task scheduled will also display the type of clean and location where the clean will take place. To modify an existing scheduled task, the user may click on the block in the schedule. 
2. Dashboard displaying each robots' current status (idle/cleaning, battery level, water level).
3. Button for scheduling a new cleaning task. 

## Task Scheduling Interface
### Input Fields
- Drop down menu to **select a date and time** for the task to occur. Here, the user may also schedule a recurring clean. 
- Drop down menu to **select a robot** to execute the clean. There will also exist a default option-- if the user does not have a preference for which robot will be assigned the clean, the computer will select the robot with the lightest task load that will be able to complete the clean the fastest.
- Drop down menu to **select the type of clean** the robot will execute.
- Drop down menu to **select the location of the clean**. Here, the user may select between allowing the robot to roam, picking a specific room, or spot cleaning. 
### Buttons
- **Confirm** to submit the request and assign the clean to a robot.
- **Cancel** to return to the home screen and discard changes. 

## Progress Monitoring
### Robot Activity Status
- Icons to display each **robot's current activity** (idle, cleaning, charging).
- Percentages to display each **robot's current water and battery levels**.
- Block schedule similar to the one in the Main Menu displaying each **robot's next scheduled clean**.
### Robot Performance Data
- Provide user with **additional performance metrics** like robot efficiency, water usage, etc. 

## User Experience Considerations
### Simplicity
Goal: Opt for more simplified designs such that both new and existing users are able to access and implement basic functions with ease.
Implementation:
1. Minimize the number of actions required to set robot tasks in motion. Scheduling a cleaning should take no more than 3-5 steps.
2. Use clear and concise labeling for interaction opportunities, like "Schedule," "Check Status," or "Help/Request Maintenance."
3. Avoid flooding the user with extra information that they don't need. Extra information should be hidden and available by request.
### Consistency
Goal: Data is presented in similar ways throughout the system, making it more easily comprehendable for the user.
Implementation:
1. Data regarding robot performance statistics, statuses, progress, etc. is presented using appropriate scales and measurements.
2. Apply uniform terminology and visuals across the platform so that users are able to easily comprehend similar data from different pages.
### Feedback
Goal: Provide appropriate feedback for each interaction occuring between the user and UI.
Implementation:
1. Present user with progress indications to let them know that their request is being processed. For example, a progress bar indicating how far along a cleaning task is, or some sort of loading indicator while the user's cleaning request is being communicated to the robot.
2. Display confirmation messages to inform the user that their request has been received, accompanied by the details of their request.
3. Use icons and graphics to display robot statistics to the user. 
### Error Prevention and Recovery: 
Goal: Reduce the occurence of errors and provide the user with framework to recover quickly if an error is to occur.
Implementation:
1. Validate user input; for example, ensuring that the user has entered valid time slots, cleaning functions, or room names when trying to schedule a clean.
2. Disable certain buttons under certain conditions; for example, when no robots are available, scheduling a new clean should be limited.
3. Offer appropriate links to help sites or the contact information for Cobotiq help desk so that the user can quickly access support. 
