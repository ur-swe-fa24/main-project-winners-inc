# Robot Management System Staffing Document

## Team Member Profiles and Task Allocation Rationale

1. Ayush: 
   - Strengths: Backend development, database management, API design
   - Gaps: Limited experience with robotics and IoT systems
   - Primary Responsibilities: Building management and scheduling systems
   - Rationale: Ayush's backend skills make him ideal for handling the core scheduling and management systems. 

2. Arianna: 
   - Strengths: Frontend development, user interface design
   - Gaps: Limited experience with real-time systems and notifications
   - Primary Responsibilities: Field engineer support and alert system implementation
   - Rationale: Arianna's frontend expertise will ensure a user-friendly interface for field engineers. 

3. Steven: 
   - Strengths: Experience with data in AI, willingness to learn real-time data processing
   - Gaps: Needs to develop skills in real-time data processing
   - Primary Responsibilities: Senior management analytics features, real-time data processing
   - Rationale: Steven's background in AI and data makes him suitable for analytics tasks.

4. Charlotte: 
   - Strengths: Data visualization, general development skills
   - Gaps: No specific experience with robotics or IoT
   - Primary Responsibilities: Robot functionality and mapping features
   - Rationale: Charlotte's data visualization skills will be valuable for mapping and pathfinding. 


## Epic 1: Senior Management Analytics
Primary Owner: Steven (with support from Charlotte for visualization)

| ID | Story | Category | Priority | Size | Assigned To |
|----|-------|----------|----------|------|-------------|
| SM1 | As a senior manager, I want to be able to access the error rate of each robot so that I am able to make deductions about robot efficiency and optimize performance. | Robot Functions | P1 | M | Steven |
| SM2 | As a senior manager, I want to be able to access the battery life of each robot so that I am able to make deductions about robot efficiency and optimize performance. | Robot Functions | P1 | M | Steven |
| SM3 | As a senior manager, I want to be able to access the cost efficiency of each robot so that I am able to make deductions about robot efficiency and optimize performance. | Robot Functions | P1 | L | Steven |
| SM4 | As a senior manager, I want to be able to access the time efficiency of each robot so that I am able to make deductions about robot efficiency and optimize performance. | Robot Functions | P1 | L | Steven |
| SM5 | As a senior manager, I want to be able to access the rate of water usage of each robot so that I am able to make deductions about robot efficiency and optimize performance. | Robot Functions | P1 | M | Steven |

### Related Operational PBIs:
- As a robot, I want to check my own battery, water, and functionality statuses between tasks so that I can determine whether I may continue, or if I must signal for maintenance. (Checking Status + Managing, P0, L) - Assigned to Steven
- As a robot, I want to have the capacity to send and receive information from the main operating system so that I can update my status and receive new tasks effectively. (Communication w/ Other Robots, P0, XL) - Assigned to Ayush

### Knowledge Gaps and Mitigation:
- Steven needs to develop skills in real-time data processing. Mitigation: Pair programming sessions with Ayush, who has more backend experience.


## Epic 2: Building Management and Scheduling
Primary Owner: Ayush (with support from Arianna for frontend)

| ID | Story | Category | Priority | Size | Assigned To |
|----|-------|----------|----------|------|-------------|
| BM1 | As a building manager, I want to have the capacity to create a daily cleaning schedule so that routine tasks are automatically executed without manual intervention. | Scheduling | P0 | XL | Ayush |
| BM2 | As a building manager, I want to have the capacity to edit an existing daily cleaning schedule so that routine tasks may be adjusted according to building needs. | Scheduling | P1 | L | Ayush |
| BM3 | As a building manager, I want to be able to access the real-time status (battery level, error rate, water level, cleaning progress) of each robot so that I can ensure that cleaning tasks are being completed appropriately and efficiently. | Robot Functions | P0 | XL | Ayush & Arianna |
| BS1 | As a building staff member, I want to assign specific zones to robots so that I can ensure that designated areas are cleaned in a timely manner. | Scheduling | P0 | L | Ayush |
| BS2 | As a building staff member, I want to assign a robot to spot clean so that a specific spot that requires additional treatment may be addressed and appropriately cleaned. | Scheduling | P1 | M | Ayush |

### Related Operational PBIs:
- As a scheduling system, I want to enqueue and dequeue tasks based on priority so that the robots complete the highest priority tasks first. (Scheduling, P0, L) - Assigned to Ayush
- As a scheduling system, I want to take robot battery, water, and functionality statuses as input so that I can appropriately assign tasks to robots that are equipped to take on new tasks. (Scheduling, P0, XL) - Assigned to Ayush
- As a scheduling system, I want to create a virtual wall so that I may prevent robots from entering restricted areas during their cleaning tasks. (Mapping/Planning, P1, M) - Assigned to Charlotte

### Knowledge Gaps and Mitigation:
- The team lacks experience with large-scale scheduling systems. Mitigation: Research existing solutions and possibly do training from online youtube videos.
- Ayush has limited IoT experience. Mitigation: Collaborate closely with Steven, who will be working on real-time data aspects.

## Epic 3: Robot Functionality and Mapping
Primary Owner: Charlotte

| ID | Story | Category | Priority | Size | Assigned To |
|----|-------|----------|----------|------|-------------|
| R1 | As a robot, I need to be able to use different cleaning functions so that I can tailor my cleaning to each room. | Assigned Functions | P0 | XL | Charlotte |
| R2 | As a robot, I want to clean different room types so that I am able to address cleanings most effectively. | Assigned Functions | P0 | XL | Charlotte |

### Related Operational PBIs:
- As a robot, I need to be able to take a map as an input so that I am able to develop cleaning routes and navigate the room. (Mapping/Planning, P0, XL) - Assigned to Charlotte
- As a robot, I want to take flooring type as an input so that I am able to effectively choose which cleaning function to implement in that room. (Mapping/Planning, P1, L) - Assigned to Charlotte
- As a robot, I need to be able to access elements in the map so that I am able to mark rooms as visited (clean) or unvisited (dirty). (Mapping/Planning, P0, L) - Assigned to Charlotte
- As a mapping system, I want to output a working path into memory so that the robot may follow the most efficient route during cleaning. (Mapping/Planning, P0, XL) - Assigned to Charlotte

### Knowledge Gaps and Mitigation:
- The team lacks specific robotics experience. Mitigation: Allocate time for research, and online courses, and possibly ask prof for help.

## Epic 4: Field Engineer Support and Alert System
Primary Owner: Arianna (with support from Steven for backend)

| ID | Story | Category | Priority | Size | Assigned To |
|----|-------|----------|----------|------|-------------|
| FE1 | As a field engineer, I want to receive notifications when a robot's battery or water level becomes low so that I am able to respond appropriately to the robot's needs. | Self Management | P0 | M | Arianna |
| FE2 | As a field manager, I want to access information about errors that occur so that I am able to respond effectively. | Self Management | P0 | L | Arianna |

### Related Operational PBIs:
- As an alert system, I want to output an alert to users when robot battery or water level is low so that robot needs may be properly addressed. (Notification System, P0, M) - Assigned to Arianna
- As an alert system, I want to output an alert to users when a task has been completed by the robot so that they may be updated on cleaning progress. (Notification System, P1, S) - Assigned to Arianna
- As an alert system, I want to output the location of the robot when an error arises so that it may be rescued and taken for maintenance. (Notification System, P0, M) - Assigned to Arianna
- As an alert system, I want to output a notification when a robot fails to complete a task so that the task may be reassigned or rescheduled. (Notification System, P0, M) - Assigned to Arianna

### Knowledge Gaps and Mitigation:
- Arianna has limited experience with real-time alert systems. Mitigation: Collaborate closely with Steven on the backend implementation.

- The team lacks experience in designing systems for field engineers. Mitigation: Conduct user research and possibly shadow field engineers to understand their needs better.


## Overall Project Challenges and Mitigation Strategies:

1. Lack of Specific Robotics Experience:
   - Mitigation: Allocate time for team-wide learning about robotics principles. 

2. Real-time System Challenges:
   - Mitigation: Focus on upskilling Steven and Arianna in real-time processing. 

3. Integration Complexity:
   - Mitigation: Regular integration meetings led by Ayush to ensure all components work together smoothly. Implement continuous integration early in the development process.



