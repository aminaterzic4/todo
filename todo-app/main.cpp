#include "mylibrary.h"
#include <iostream>
#include <limits>

int main() {
    using namespace MyLibrary;  // Pull in our library functions/classes

    const std::string filename = "tasks.txt";
    Task::loadTasksFromFile(filename);

    bool running = true;
    while (running) {
        std::cout << "\n========== TO-DO LIST MANAGER ==========\n"
            << "1. Add Task\n"
            << "2. Edit Task\n"
            << "3. Delete Task\n"
            << "4. Mark Task as Completed\n"
            << "5. Display All Tasks\n"
            << "6. Filter by Status (Completed/Pending)\n"
            << "7. Sort Tasks by Priority\n"
            << "8. Sort Tasks by Due Date\n"
            << "9. Display Completion Percentage\n"
            << "10. Save Tasks\n"
            << "0. Exit\n"
            << "========================================\n"
            << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        // Clear any leftover newline characters or invalid states
        if (!std::cin) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Invalid input. Try again.\n";
            continue;
        }

        switch (choice) {
        case 1: { // Add Task
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush leftover
            std::cout << "Enter task description: ";
            std::string desc;
            std::getline(std::cin, desc);

            Priority prio = promptForPriority();
            time_t due = promptForDueDate();

            Task::addTask(desc, prio, due);
            std::cout << "Task added successfully.\n";
            break;
        }
        case 2: { // Edit Task
            std::cout << "Enter task ID to edit: ";
            int id;
            std::cin >> id;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid ID.\n";
                break;
            }

            // flush leftover
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // Ask user which fields to update
            std::cout << "Update description? (y/n): ";
            char dOption;
            std::cin >> dOption;
            std::optional<std::string> descOpt;
            if (dOption == 'y' || dOption == 'Y') {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "New description: ";
                std::string newDesc;
                std::getline(std::cin, newDesc);
                descOpt = newDesc;
            }

            std::cout << "Update priority? (y/n): ";
            char pOption;
            std::cin >> pOption;
            std::optional<Priority> prioOpt;
            if (pOption == 'y' || pOption == 'Y') {
                Priority newPrio = promptForPriority();
                prioOpt = newPrio;
            }

            std::cout << "Update completion status? (y/n): ";
            char cOption;
            std::cin >> cOption;
            std::optional<bool> compOpt;
            if (cOption == 'y' || cOption == 'Y') {
                std::cout << "Mark as completed? (1=Yes, 0=No): ";
                int compVal;
                std::cin >> compVal;
                if (!std::cin) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cerr << "Invalid input for completion.\n";
                    break;
                }
                compOpt = static_cast<bool>(compVal);
            }

            std::cout << "Update due date? (y/n): ";
            char dateOption;
            std::cin >> dateOption;
            std::optional<time_t> dueOpt;
            if (dateOption == 'y' || dateOption == 'Y') {
                time_t newDue = promptForDueDate();
                dueOpt = newDue;
            }

            Task::updateTask(id, descOpt, prioOpt, compOpt, dueOpt);
            std::cout << "Task updated (if ID was valid).\n";
            break;
        }
        case 3: { // Delete Task
            std::cout << "Enter task ID to delete: ";
            int id;
            std::cin >> id;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid ID.\n";
                break;
            }
            Task::deleteTask(id);
            std::cout << "Task deleted (if ID was valid).\n";
            break;
        }
        case 4: { // Mark Task as Completed
            std::cout << "Enter task ID to mark as completed: ";
            int id;
            std::cin >> id;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid ID.\n";
                break;
            }
            Task::updateTask(id, {}, {}, true, {});
            std::cout << "Task marked as completed (if ID was valid).\n";
            break;
        }
        case 5:
            Task::displayTasks();
            break;
        case 6: { // Filter by Status
            std::cout << "Filter tasks by status:\n"
                << "1. Completed\n"
                << "2. Pending\n"
                << "Enter your choice: ";
            int statusChoice;
            std::cin >> statusChoice;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid choice.\n";
                break;
            }
            if (statusChoice == 1) {
                Task::filterTasksByStatus(true);
            }
            else if (statusChoice == 2) {
                Task::filterTasksByStatus(false);
            }
            else {
                std::cerr << "Invalid choice.\n";
            }
            break;
        }
        case 7: { // Sort by Priority
            std::cout << "Sort by priority:\n"
                << "1. Ascending\n"
                << "2. Descending\n"
                << "Enter your choice: ";
            int sortChoice;
            std::cin >> sortChoice;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid choice.\n";
                break;
            }
            if (sortChoice == 1) {
                Task::sortTasksByPriority(true);
            }
            else if (sortChoice == 2) {
                Task::sortTasksByPriority(false);
            }
            else {
                std::cerr << "Invalid choice.\n";
            }
            break;
        }
        case 8: { // Sort by Due Date
            std::cout << "Sort by due date:\n"
                << "1. Ascending\n"
                << "2. Descending\n"
                << "Enter your choice: ";
            int sortChoice;
            std::cin >> sortChoice;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid choice.\n";
                break;
            }
            if (sortChoice == 1) {
                Task::sortTasksByDueDate(true);
            }
            else if (sortChoice == 2) {
                Task::sortTasksByDueDate(false);
            }
            else {
                std::cerr << "Invalid choice.\n";
            }
            break;
        }
        case 9:
            Task::displayCompletionPercentage();
            break;
        case 10:
            Task::saveTasksToFile(filename);
            std::cout << "Tasks saved to file.\n";
            break;
        case 0:
            running = false;
            break;
        default:
            std::cerr << "Invalid menu choice. Please try again.\n";
            break;
        }
    }

    // Optionally, auto-save before exiting.
    // Task::saveTasksToFile(filename);

    std::cout << "Exiting program. Goodbye.\n";
    return 0;
}
