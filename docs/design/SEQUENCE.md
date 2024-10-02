# Sequence Diagrams

This document describes the key interactions among system participants as represented in our sequence diagrams. These diagrams illustrate how the use cases are implemented in terms of object interactions over time.

## 1. Scheduling Tasks

**Related Use Case:** Manage Tasks (Building Manager)

**Diagram:** SequenceDiagram_SchedulingTasks.puml

This sequence diagram shows the process of a Building Manager scheduling a new cleaning task:

1. The Building Manager submits user input for time, date, and clean type through the UI.
2. The System Manager checks robot availability in the Database.
3. Available time slots are returned to the UI for the Building Manager to select.
4. The Building Manager submits their selection.
5. The System Manager adds the selection to the robot schedule in the Database.
6. The System Manager sends the request to the robot through the Simulation.
7. A confirmation message is sent back to the UI.

This diagram demonstrates how the system handles task creation and assignment, which is a key part of the "Manage Tasks" use case.

## 2. Monitoring Status

**Related Use Case:** Robot Analytics (Senior Manager)

**Diagram:** SequenceDiagram_MonitoringStatus.puml

This sequence diagram illustrates two main processes:

1. Continuous monitoring of task progress:
   - The Database regularly checks the status of cleaning tasks with the Simulation.
   - The Simulation logs the status in the Database.

2. User viewing clean status:
   - The user views the clean status page in the UI.
   - The System Manager retrieves clean status data from the Database.
   - The data is displayed in the UI.

This diagram shows how the system continuously monitors robot status and how users can access this information, which is crucial for the "Robot Analytics" use case.

## 3. Error Handling

**Related Use Case:** Alerts and Errors (Field Engineer)

**Diagram:** SequenceDiagram_ErrorHandling.puml

This sequence diagram shows the process of error detection and handling:

1. The Simulation continuously checks for errors during cleaning.
2. When an error occurs, the Simulation sends an error message to the System Manager.
3. The System Manager alerts the user through the UI.
4. The user acknowledges the issue.
5. The Simulation logs the error and robot statuses in the Database.

This diagram illustrates how the system detects and responds to errors, which is a key part of the "Alerts and Errors" use case for Field Engineers.

## 4. Creating Schedule

**Related Use Case:** Manage Tasks (Building Manager)

**Diagram:** SequenceDiagram_CreatingSchedule.puml

This sequence diagram shows the process of a Building Manager creating a new cleaning schedule:

1. The Building Manager opens the Scheduling Interface.
2. They select "Create New Schedule" and input cleaning tasks and robot assignments.
3. The Scheduling UI submits the new schedule data to the Scheduling Backend.
4. The Backend saves the new schedule in the Database.
5. A confirmation is sent back to the UI and displayed to the Building Manager.

This diagram provides a more detailed view of schedule creation, which is part of the "Manage Tasks" use case.

## 5. Editing Schedule

**Related Use Case:** Manage Tasks (Building Manager)

**Diagram:** SequenceDiagram_EditScheduling.puml

This sequence diagram illustrates the process of editing an existing schedule:

1. The Building Manager selects an existing schedule.
2. The Scheduling UI requests the existing schedule data from the Backend.
3. The Backend fetches the schedule data from the Database.
4. The schedule is displayed for editing.
5. The Building Manager modifies the schedule.
6. The edited schedule is submitted and updated in the Database.
7. A confirmation is displayed to the Building Manager.

This diagram shows how the system handles schedule modifications, another aspect of the "Manage Tasks" use case.

## 6. Accessing Data

**Related Use Case:** Robot Analytics (Senior Manager)

**Diagram:** SequenceDiagram_AccessingData.puml

This sequence diagram shows how a Senior Manager accesses various types of robot performance data:

1. The Senior Manager opens the Dashboard.
2. They select different types of robot data (Error Rate, Battery Life, Cost Efficiency, Time Efficiency, Water Usage).
3. For each selection, the Dashboard UI requests the specific data from the Analytics Backend.
4. The Backend sends the requested data.
5. The UI displays the data to the Senior Manager.

This diagram illustrates how the system provides detailed analytics to Senior Managers, which is central to the "Robot Analytics" use case.

## 7. Viewing Status

**Related Use Case:** Robot Analytics (Building Manager)

**Diagram:** SequenceDiagram_ViewStatus.puml

This sequence diagram shows how a Building Manager views real-time robot status:

1. The Building Manager opens the Real-Time Status Page.
2. The Status UI requests real-time robot data from the Robot Management Backend.
3. The Backend sends the current data (battery, water, error status).
4. The UI displays the real-time robot status to the Building Manager.

This diagram demonstrates how the system provides up-to-date robot status information, which is part of both the "Robot Analytics" and "Manage Tasks" use cases.

## 8. Triggering Alerts

**Related Use Case:** Alerts and Errors (Field Engineer)

**Diagram:** SequenceDiagram_Alerts.puml

This sequence diagram illustrates how alerts are triggered and handled:

1. The Notification Backend triggers a Low Battery Alert.
2. The Alert System UI displays the notification to the Field Engineer.
3. The Field Engineer acknowledges the alert.
4. Similarly, a Robot Error Alert is triggered and displayed.
5. The Field Engineer acknowledges the Robot Error Alert.

This diagram shows the alert system in action, which is crucial for the "Alerts and Errors" use case.

These sequence diagrams collectively illustrate how the system implements the main use cases, showing the interactions between users, the user interface, backend systems, and the robots themselves. They provide a detailed view of the system's behavior over time, complementing the static structure shown in the class diagrams and the high-level functionality described in the use case diagrams.
