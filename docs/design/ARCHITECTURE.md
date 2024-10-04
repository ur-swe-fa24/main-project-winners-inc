# System Architecture

## 1. Introduction

The system architecture of the Cobotiq robot fleet management solution is designed to integrate a user-friendly interface with backend systems to manage, schedule, and monitor the operations of a fleet of robots. This document outlines the structure of the system, including the main components, communication flow, and core functional modules. The architecture aims to ensure high availability, scalability, and efficient management of robots, offering a robust solution for building and field engineers to manage tasks effectively.

---

## 2. Architecture Overview

The architecture for the Cobotiq system follows a modular design pattern, incorporating the following key layers:

- **Presentation Layer (User Interface)**
- **Application Layer (Business Logic)**
- **Data Layer (Databases & Storage)**
- **Robotics Communication Layer (Robot Integration)**

Each layer is designed to handle distinct functionalities while communicating efficiently with other layers. Below is a breakdown of each layer’s responsibilities and components.

---

## 3. Components

### 3.1 Presentation Layer (User Interface)
The **Presentation Layer** serves as the user-facing component of the system, providing an intuitive UI for interaction with the robot fleet. The core functionalities of this layer include:

- **Dashboard**: Displays robot statuses, schedules, and provides quick access to control functions.
- **Scheduling Interface**: Allows users to create, modify, and manage robot task schedules through simple forms with dropdowns for task details.
- **Alerts and Notification Panel**: Alerts users (e.g., Field Engineers, Building Managers) about errors or critical events like low battery, water levels, or task completion failures.

This layer is built using modern web technologies such as **React** or **Vue.js**, ensuring a responsive design that functions on various devices, including tablets and desktops.

---

### 3.2 Application Layer (Business Logic)
The **Application Layer** is the core of the system that handles all the business logic for robot task management and monitoring. It acts as an intermediary between the UI and the data layer. Key modules include:

- **Task Scheduler**: Implements the logic for task assignments, robot selection, and task prioritization based on load, availability, and cleaning type.
- **Notification and Alert System**: Manages real-time notifications for users about robot statuses and errors. It includes retry mechanisms for alert failures.
- **Analytics Engine**: Compiles and displays robot performance metrics like battery usage, water consumption, and task efficiency for senior managers and building administrators.
- **Error Handling and Recovery**: Manages automated systems for detecting and handling errors with validation processes, ensuring system robustness.

Technologies: This layer can be implemented using a server-side framework like **Node.js** or **Django**, ensuring that requests are processed quickly, and tasks are dispatched efficiently to robots.

---

### 3.3 Data Layer (Databases & Storage)
The **Data Layer** manages all the system's persistent data, including:

- **Task Database**: Stores cleaning tasks, schedules, and robot assignments. This database is queried by the UI to present schedules and robot statuses.
- **Robot Status Database**: Maintains real-time information on each robot’s location, battery level, water level, and operational status.
- **Performance Data Storage**: Stores analytics data on robot performance for historical analysis, reporting, and decision-making.

This layer relies on **SQL-based databases** like **PostgreSQL** or **MySQL** for structured task and performance data, while a **NoSQL** solution like **MongoDB** could be used to store real-time robot telemetry data.

---

### 3.4 Robotics Communication Layer (Robot Integration)
The **Robotics Communication Layer** serves as the bridge between the software system and the physical robots. It includes:

- **Command Dispatch Module**: Sends task instructions from the application layer to the robots. This module ensures that commands are executed in the correct order, based on task priority.
- **Telemetry Receiver**: Receives data from robots regarding their operational status (battery level, location, water level) and feeds it back into the application layer for UI updates.
- **Error Reporting and Diagnostics**: Collects diagnostic information from robots and raises alerts when issues like low battery or hardware failures occur.

This layer leverages a communication protocol such as **MQTT** or **WebSockets** to ensure real-time bi-directional communication with the robots, ensuring that task statuses and robot performance are up-to-date.

---

## 4. System Workflow

### 4.1 Task Scheduling Workflow
1. The **Building Manager** logs into the system and opens the **Task Scheduling Interface**.
2. The user inputs task details, such as the cleaning type, location, and preferred robot.
3. The **Task Scheduler Module** in the application layer assigns the task based on robot availability and sends the task to the appropriate robot through the **Command Dispatch Module**.
4. The robot performs the task, and status updates are sent to the system via the **Telemetry Receiver**.
5. The **Application Layer** updates the **Task Database**, and the UI reflects the robot’s progress on the **Dashboard**.
6. The task is marked complete, and the **Building Manager** is notified.

### 4.2 Error Reporting Workflow
1. A robot detects a critical error (e.g., low battery or operational failure).
2. The robot sends an error signal to the **Telemetry Receiver**.
3. The **Notification and Alert System** triggers a notification to the **Field Engineer**.
4. The Field Engineer receives an alert in the UI with details on the error and robot location.
5. If the error persists or the notification fails to send, the system logs the failure and retries, escalating the issue if needed.

---

## 5. Scalability and Future Enhancements

### 5.1 Scalability Considerations
- **Horizontal Scaling**: The system can be scaled horizontally by distributing the task scheduling and analytics workload across multiple server instances.
- **Cloud Integration**: For larger robot fleets or complex deployments, cloud-based services like **AWS** or **Google Cloud** could be integrated to handle increased data storage and real-time processing needs.

### 5.2 Future Enhancements
- **AI-based Task Assignment**: Implement machine learning algorithms for better task load balancing between robots.
- **Advanced Analytics Dashboard**: Add predictive analytics for preemptive maintenance and optimized task scheduling based on robot usage patterns.
- **Mobile Application**: Develop a mobile app for on-the-go monitoring and control of robots.

---

## 6. Conclusion

The system architecture of Cobotiq’s robot fleet management solution is designed to provide efficient, real-time management of robots with a focus on user-friendly interfaces, reliable task scheduling, and detailed performance monitoring. The modular structure allows for easy scalability and future enhancements, ensuring the system remains adaptable to evolving requirements in robot management.

---
