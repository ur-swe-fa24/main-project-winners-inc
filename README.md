# Project Background

## Project Context
Our team was tasked with developing a prototype management system with the power to manage a fleet of robots for Cobotiq, a start-up company based in Henrico, VA dedicated to revolutionizing the autonomous robot cleaning industry. After meeting with the Cobotiq team to discuss customer needs and likely users of this software product, we developed comprehensive documentation to ensure that all user and customer needs would be met by the end of our product development. The main goals of our team, as well as key features of our product, are outlined briefly below; more information regarding our product can be found in our documentation under the *docs* folder in this repository.
---

## Product Goals
Throughout the development of our software product, we have maintained and upheld a set of goals. These goals are listed below:
+ Provide the user with a comprehensive overview of information about their robot, including, but not limited to, efficiency metrics, current robot status, a log of errors and alerts, etc.
+ Create an intuitive, user-friendly UI that enables users to easily schedule, manage, and view the status of cleans.
+ Generate an illustration of the input map to provide the user with information about the locations of the robots, whether a given room has been cleaned, and which rooms are accessible to the robot.
+ Provide users with a clear and consise message in situations where an alert or error message are warranted.
+ Supply the robot with the functionality to find the most efficient path and use different cleaning modes depending on floor type.
---

## Product Features
Our software product includes many important features that make our robots unique. These features are outlined below:
+ Alerts and Errors - Our Cleanbots are equipped with the ability to periodically check their real-time battery and water levels, along with their current operational status. If, at any point, the robot detects some threat to its functionality, like low battery or water levels or a technical issue, the robot is able to send a notification to the user, alerting them of the error.
+ Managing Tasks - These robots are furnished with the capacity to be assigned cleaning tasks from user. Through our carefully designed UI, the user is able to make a series of selections from drop-down menus to indicate the parameters of the cleaning task they want to run. Through the UI, they are able to specify which robot should complete the task, where and when the clean should occur, and the cleaning mode.
+ Robot Analytics - Users are also provided with the capability to access certain metrics that the robot tracks as it executes its cleans. These analytics include utilization, error rate, cost efficiency, time efficiency, battery usage, and water usage.
---

## Navigating This Repository
+ The **docs** directory in this repository contains all of the documentation that we generated to help guide our development. Within this directory, you will find the following subdirectories: **design**, **notes**, and **user_guide**. In the **design** subdirectory, you will find our class diagram, which is updated with each sprint, sequence diagrams, proposed use case diagrams, wire frame diagrams, and architecture diagrams. Under the **notes** subdirectory, you will find a contribution statement from each team member for each sprint along with our *requirements.md* file which documents the likely users of the product in addition to our proposed needs and requirements. Within the **user_guide** subdirectory is our *USER_GUIDE.md* file which documents how users should interact with our software product. Additionally, the **docs** directory contains our *README.md* file.
+ The **include** directory within this repository holds the header files containing the declarations of all libraries generated in the development of our product.
+ The **src** directory contains the implementation files for those libraries that require them.
+ The **app** directory in this repository maintains the main applications of the project. This includes files like *main.cpp*, *CMakeLists.txt*, *testing.cpp*, and more.
+ The **tests** directory is comprised of all test files used to guide our development and test our libraries and app.
---

## Updates Since Sprint 3
+ Continued work on simulator implementation
+ More progress on UI implementation
+ Developing unit tests for Robot, Simulation, MappingSystem, and SchedulingSystem
+ Updated design documents, mainly making changes to our *class_diagram* files
+ Creating README.md and USER_GUIDE.md files
+ Implementation for SchedulingSystem

