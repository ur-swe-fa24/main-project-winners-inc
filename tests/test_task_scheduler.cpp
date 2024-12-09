#include <catch2/catch_test_macros.hpp>
#include "TaskScheduler/TaskScheduler.h"
#include "Robot/Robot.h"
#include "Room/Room.h"
#include "config/ResourceConfig.hpp"
#include <memory>
#include <chrono>
#include <thread>
#include <filesystem>

class TestFixture {
public:
    TestFixture() {
        // Initialize resource config
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path resourcePath = currentPath / "resources";
        if (!std::filesystem::exists(resourcePath)) {
            resourcePath = currentPath / ".." / "resources";
        }
        config::ResourceConfig::initialize(resourcePath.string());
        
        // Create test rooms
        room1 = std::make_shared<Room>("Test Room 1", 1, "hardwood", "medium");
        room2 = std::make_shared<Room>("Test Room 2", 2, "carpet", "medium");
        
        // Create test robot
        robot = std::make_shared<Robot>("TestBot", 100.0, Robot::Size::MEDIUM, Robot::Strategy::VACUUM);
        robot->setCurrentRoom(room1.get());
        
        // Clear any existing tasks in the scheduler
        auto& scheduler = TaskScheduler::getInstance();
        while (scheduler.hasTasks()) {
            scheduler.dequeueTask();
        }
    }

    ~TestFixture() {
        // Clean up tasks in scheduler
        auto& scheduler = TaskScheduler::getInstance();
        while (scheduler.hasTasks()) {
            scheduler.dequeueTask();
        }
    }

    std::shared_ptr<Room> room1;
    std::shared_ptr<Room> room2;
    std::shared_ptr<Robot> robot;
};

TEST_CASE("Test Task Scheduler System", "[task_scheduler]") {
    auto& scheduler = TaskScheduler::getInstance();
    TestFixture fixture;

    SECTION("Priority Queue Ordering") {
        // Create tasks with different priorities
        auto lowTask = std::make_shared<CleaningTask>(1, CleaningTask::Priority::LOW, 
                                                     CleaningTask::CleanType::VACUUM, fixture.room1.get());
        auto mediumTask = std::make_shared<CleaningTask>(2, CleaningTask::Priority::MEDIUM, 
                                                        CleaningTask::CleanType::SCRUB, fixture.room1.get());
        auto highTask = std::make_shared<CleaningTask>(3, CleaningTask::Priority::HIGH, 
                                                      CleaningTask::CleanType::SHAMPOO, fixture.room1.get());

        // Add tasks in reverse priority order
        scheduler.enqueueTask(lowTask);
        scheduler.enqueueTask(mediumTask);
        scheduler.enqueueTask(highTask);

        // Verify tasks are dequeued in priority order
        auto task1 = scheduler.dequeueTask();
        REQUIRE(task1 != nullptr);
        CHECK(task1->getPriority() == CleaningTask::Priority::HIGH);
        CHECK(task1->getID() == 3);

        auto task2 = scheduler.dequeueTask();
        REQUIRE(task2 != nullptr);
        CHECK(task2->getPriority() == CleaningTask::Priority::MEDIUM);
        CHECK(task2->getID() == 2);

        auto task3 = scheduler.dequeueTask();
        REQUIRE(task3 != nullptr);
        CHECK(task3->getPriority() == CleaningTask::Priority::LOW);
        CHECK(task3->getID() == 1);

        // Verify queue is empty
        CHECK(scheduler.dequeueTask() == nullptr);
    }

    SECTION("Robot Task Assignment") {
        // Create and enqueue a high priority task
        auto highTask = std::make_shared<CleaningTask>(1, CleaningTask::Priority::HIGH, 
                                                      CleaningTask::CleanType::VACUUM, fixture.room1.get());
        scheduler.enqueueTask(highTask);

        // Robot should be able to accept task
        CHECK(fixture.robot->canAcceptTask());
        
        // Robot should successfully request and receive the task
        CHECK(fixture.robot->requestNextTask());
        
        // Verify robot state
        CHECK(fixture.robot->isCleaning());
        auto currentTask = fixture.robot->getCurrentTask();
        REQUIRE(currentTask != nullptr);
        CHECK(currentTask->getID() == 1);
        CHECK(currentTask->getPriority() == CleaningTask::Priority::HIGH);

        // Queue should be empty now
        CHECK_FALSE(scheduler.hasTasks());
    }

    SECTION("Robot Resource Check") {
        // Create a low battery robot
        auto lowBatteryRobot = std::make_shared<Robot>("LowBatBot", 15.0, Robot::Size::MEDIUM, 
                                                      Robot::Strategy::VACUUM);
        lowBatteryRobot->setCurrentRoom(fixture.room1.get());

        // Create and enqueue a task
        auto task = std::make_shared<CleaningTask>(1, CleaningTask::Priority::HIGH, 
                                                  CleaningTask::CleanType::VACUUM, fixture.room1.get());
        scheduler.enqueueTask(task);

        // Robot with low battery should not accept task
        CHECK_FALSE(lowBatteryRobot->canAcceptTask());
        CHECK_FALSE(lowBatteryRobot->requestNextTask());

        // Task should still be in queue
        CHECK(scheduler.hasTasks());
        CHECK(scheduler.taskCount() == 1);
    }

    SECTION("Task Queue Persistence") {
        // Verify queue is empty at start
        CHECK_FALSE(scheduler.hasTasks());
        CHECK(scheduler.taskCount() == 0);

        // Add multiple tasks
        auto task1 = std::make_shared<CleaningTask>(1, CleaningTask::Priority::HIGH, 
                                                   CleaningTask::CleanType::VACUUM, fixture.room1.get());
        auto task2 = std::make_shared<CleaningTask>(2, CleaningTask::Priority::MEDIUM, 
                                                   CleaningTask::CleanType::SCRUB, fixture.room2.get());
        
        scheduler.enqueueTask(task1);
        scheduler.enqueueTask(task2);
        
        // Verify queue state
        CHECK(scheduler.hasTasks());
        CHECK(scheduler.taskCount() == 2);
        
        // Remove one task
        auto dequeuedTask = scheduler.dequeueTask();
        REQUIRE(dequeuedTask != nullptr);
        CHECK(dequeuedTask->getID() == 1); // Should be the HIGH priority task
        
        // Verify updated queue state
        CHECK(scheduler.hasTasks());
        CHECK(scheduler.taskCount() == 1);
    }
}
