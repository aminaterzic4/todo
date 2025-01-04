#define _CRT_SECURE_NO_WARNINGS  // (Optional) to suppress some MSVC warnings
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <optional>
#include <limits>
#include <cstring>   // for strcpy_s (MSVC)

using namespace std;

/*-------------------------- ENUM & HELPER FUNCTIONS --------------------------*/

enum Priority {
    HIGHEST = 1,
    HIGH,
    MEDIUM,
    LOW,
    LOWEST
};

// Convert enum Priority to string for display
string priorityToString(Priority prio) {
    switch (prio) {
    case HIGHEST: return "Highest";
    case HIGH:    return "High";
    case MEDIUM:  return "Medium";
    case LOW:     return "Low";
    case LOWEST:  return "Lowest";
    default:      return "Unknown";
    }
}

/**
 * Safe conversion from string to Priority.
 * @param str The string representation (e.g. "1", "2", "3", "4", "5").
 * @return optional<Priority> which is std::nullopt if invalid.
 */
optional<Priority> stringToPrioritySafe(const string& str) {
    if (str == "1")      return HIGHEST;
    else if (str == "2") return HIGH;
    else if (str == "3") return MEDIUM;
    else if (str == "4") return LOW;
    else if (str == "5") return LOWEST;
    return {};
}

/**
 * Prompt the user until a valid Priority is entered.
 * Returns the chosen Priority.
 */
Priority promptForPriority() {
    while (true) {
        cout << "Enter priority (1=Highest, 2=High, 3=Medium, 4=Low, 5=Lowest): ";
        string prioStr;
        cin >> prioStr;
        if (!cin) {
            // Clear error flags and ignore leftover input
            cin.clear();
            cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
            cerr << "Invalid input. Please enter a number (1-5).\n";
            continue;
        }

        auto prioOpt = stringToPrioritySafe(prioStr);
        if (prioOpt.has_value()) {
            return prioOpt.value();
        }
        else {
            cerr << "Invalid priority. Must be 1 to 5.\n";
        }
    }
}

/**
 * Prompt for a due date in the format YYYY MM DD (naive).
 * Returns the parsed time_t.
 */
time_t promptForDueDate() {
    while (true) {
        cout << "Enter due date (YYYY MM DD): ";
        int year, month, day;
        cin >> year >> month >> day;
        if (!cin) {
            cin.clear();
            cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
            cerr << "Invalid date input. Please try again.\n";
            continue;
        }
        // Build tm struct
        tm timeStruct = {};
        timeStruct.tm_year = year - 1900; // years since 1900
        timeStruct.tm_mon = month - 1;   // months since January
        timeStruct.tm_mday = day;
        timeStruct.tm_hour = 12;         // set to noon to avoid DST issues
        timeStruct.tm_min = 0;
        timeStruct.tm_sec = 0;

        // Validate that mktime didn't fail (e.g. out-of-range date)
        time_t due = mktime(&timeStruct);
        if (due == -1) {
            cerr << "Failed to parse that date. Please try again.\n";
            continue;
        }
        return due;
    }
}

/*------------------------------ TASK CLASS ----------------------------------*/

class Task {
private:
    int id;
    string description;
    Priority priority;
    bool completed;
    time_t dueDate;  

    static int nextId;
    static vector<Task> tasks;

    // Validate description & priority
    static bool validateTask(const string& desc, Priority prio) {
        if (desc.empty()) {
            cerr << "Error: Description cannot be empty.\n";
            return false;
        }
        if (prio < HIGHEST || prio > LOWEST) {
            cerr << "Error: Priority must be between 1 and 5.\n";
            return false;
        }
        return true;
    }

public:
    // Constructor
    Task(const string& desc, Priority prio, time_t due)
        : id(nextId++), description(desc), priority(prio), completed(false), dueDate(due)
    {}

    /*-------------------------- STATIC METHODS --------------------------*/

    // Load tasks from file
    static void loadTasksFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            // It's not necessarily an error if the file doesn't exist,
            // so we won't print anything here.
            return;
        }

        tasks.clear();
        while (true) {
            if (!file.good() || file.peek() == EOF) break;

            // We expect each task line to have the format:
            // description|priority completed dueDate
            // Example:
            // This is a task|3 0 1700000000
            // We use '|' as a delimiter for the description, which may have spaces.
            // Then we read priority, completed, and due date.

            string desc;
            getline(file, desc, '|');  // read description until '|'
            if (!file.good()) break;

            int prioInt = 0;
            int compInt = 0;
            time_t due = 0;

            file >> prioInt;
            if (!file.good()) break;

            file >> compInt;
            if (!file.good()) break;

            file >> due;
            if (!file.good()) break;

            // Move to the next line
            file.ignore(std::numeric_limits<streamsize>::max(), '\n');

            // Validate data
            if (desc.empty() || prioInt < HIGHEST || prioInt > LOWEST) {
                // If it's invalid, we skip
                cerr << "Skipping invalid task from file.\n";
                continue;
            }

            Task task(desc, static_cast<Priority>(prioInt), due);
            task.completed = static_cast<bool>(compInt);

            tasks.push_back(task);
        }
        file.close();

        // Sync nextId if tasks loaded
        int maxID = 0;
        for (auto& task : tasks) {
            if (task.id > maxID) maxID = task.id;
        }
        nextId = maxID + 1;
    }

    // Save tasks to file
    static void saveTasksToFile(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Unable to open file for saving.\n";
            return;
        }
        // Format: description|priority completed dueDate
        for (auto& task : tasks) {
            if (!validateTask(task.description, task.priority)) {
                cerr << "Error: Invalid Task with ID " << task.id << " - not saved.\n";
                continue;
            }
            file << task.description << "|"
                << task.priority << " "
                << task.completed << " "
                << task.dueDate << "\n";
        }
        file.close();
    }

    // Add Task
    static void addTask(const string& desc, Priority prio, time_t due) {
        if (validateTask(desc, prio)) {
            tasks.push_back(Task(desc, prio, due));
        }
    }

    // Delete Task
    static void deleteTask(int id) {
        auto oldSize = tasks.size();
        tasks.erase(remove_if(tasks.begin(), tasks.end(),
            [id](const Task& t) { return t.id == id; }),
            tasks.end());
        if (tasks.size() == oldSize) {
            cerr << "Warning: No task found with ID " << id << ".\n";
        }
    }

    // Update Task
    static void updateTask(int id,
        optional<string> desc = {},
        optional<Priority> prio = {},
        optional<bool> comp = {},
        optional<time_t> due = {})
    {
        for (auto& task : tasks) {
            if (task.id == id) {
                if (desc) {
                    // Validate the new description with the existing priority
                    if (!validateTask(*desc, task.priority)) {
                        cerr << "Update failed due to invalid description.\n";
                        return;
                    }
                    task.description = *desc;
                }
                if (prio) {
                    // Validate with new priority
                    if (!validateTask(task.description, *prio)) {
                        cerr << "Update failed due to invalid priority.\n";
                        return;
                    }
                    task.priority = *prio;
                }
                if (comp) {
                    task.completed = *comp;
                }
                if (due) {
                    task.dueDate = *due;
                }
                return;
            }
        }
        cerr << "Warning: No task found with ID " << id << ".\n";
    }

    // Display All Tasks
    static void displayTasks() {
        if (tasks.empty()) {
            cout << "No tasks available.\n";
            return;
        }

        cout << left << setw(5) << "ID"
            << setw(25) << "Description"
            << setw(10) << "Priority"
            << setw(10) << "Status"
            << setw(20) << "Due Date"
            << endl;

        for (auto& task : tasks) {
            char buffer[20];
            tm timeStruct;
            // Use localtime_s in MSVC; returns 0 on success.
            if (localtime_s(&timeStruct, &task.dueDate) == 0) {
                strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeStruct);
            }
            else {
                // Fallback if there's an error
                strcpy_s(buffer, "InvalidDate");
            }

            cout << left << setw(5) << task.id
                << setw(25) << task.description
                << setw(10) << priorityToString(task.priority)
                << setw(10) << (task.completed ? "Completed" : "Pending")
                << setw(20) << buffer
                << endl;
        }
    }

    // Sort tasks by Priority
    static void sortTasksByPriority(bool ascending = true) {
        sort(tasks.begin(), tasks.end(),
            [ascending](const Task& a, const Task& b) {
                return ascending ? (a.priority < b.priority)
                    : (a.priority > b.priority);
            });
    }

    // Sort tasks by Due Date
    static void sortTasksByDueDate(bool ascending = true) {
        sort(tasks.begin(), tasks.end(),
            [ascending](const Task& a, const Task& b) {
                return ascending ? (a.dueDate < b.dueDate)
                    : (a.dueDate > b.dueDate);
            });
    }

    // Filter tasks by Status
    static void filterTasksByStatus(bool completedStatus) {
        bool foundAny = false;
        cout << left << setw(5) << "ID"
            << setw(25) << "Description"
            << setw(10) << "Priority"
            << setw(10) << "Status"
            << setw(20) << "Due Date"
            << endl;

        for (auto& task : tasks) {
            if (task.completed == completedStatus) {
                foundAny = true;
                char buffer[20];
                tm timeStruct;
                if (localtime_s(&timeStruct, &task.dueDate) == 0) {
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeStruct);
                }
                else {
                    strcpy_s(buffer, "InvalidDate");
                }

                cout << left << setw(5) << task.id
                    << setw(25) << task.description
                    << setw(10) << priorityToString(task.priority)
                    << setw(10) << (task.completed ? "Completed" : "Pending")
                    << setw(20) << buffer
                    << endl;
            }
        }
        if (!foundAny) {
            cout << "No tasks found with status: "
                << (completedStatus ? "Completed" : "Pending") << endl;
        }
    }

    // Calculate & display completion percentage
    static void displayCompletionPercentage() {
        if (tasks.empty()) {
            cout << "No tasks. Completion percentage: 0%\n";
            return;
        }
        int completedCount = 0;
        for (auto& task : tasks) {
            if (task.completed) {
                completedCount++;
            }
        }
        double percentage = (static_cast<double>(completedCount) / tasks.size()) * 100.0;
        cout << "Completion Percentage: "
            << fixed << setprecision(2) << percentage << "%\n";
    }
};

/*--------------------- STATIC MEMBERS INITIALIZATION -------------------------*/

int Task::nextId = 1;
vector<Task> Task::tasks;

/*------------------------------ MAIN ----------------------------------------*/

int main() {
    const string filename = "tasks.txt";
    Task::loadTasksFromFile(filename);

    bool running = true;
    while (running) {
        cout << "\n========== TO-DO LIST MANAGER ==========\n"
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
        cin >> choice;

        // Clear any leftover newline characters or invalid states
        if (!cin) {
            cin.clear();
            cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
            cerr << "Invalid input. Try again.\n";
            continue;
        }

        switch (choice) {
        case 1: { // Add Task
            cin.ignore(std::numeric_limits<streamsize>::max(), '\n'); // flush leftover
            cout << "Enter task description: ";
            string desc;
            getline(cin, desc);

            Priority prio = promptForPriority();
            time_t due = promptForDueDate();

            Task::addTask(desc, prio, due);
            cout << "Task added successfully.\n";
            break;
        }
        case 2: { // Edit Task
            cout << "Enter task ID to edit: ";
            int id;
            cin >> id;
            if (!cin) {
                cin.clear();
                cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                cerr << "Invalid ID.\n";
                break;
            }

            // flush leftover
            cin.ignore(std::numeric_limits<streamsize>::max(), '\n');

            // Ask user which fields to update
            cout << "Update description? (y/n): ";
            char dOption;
            cin >> dOption;
            optional<string> descOpt;
            if (dOption == 'y' || dOption == 'Y') {
                cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                cout << "New description: ";
                string newDesc;
                getline(cin, newDesc);
                descOpt = newDesc;
            }

            cout << "Update priority? (y/n): ";
            char pOption;
            cin >> pOption;
            optional<Priority> prioOpt;
            if (pOption == 'y' || pOption == 'Y') {
                // We call promptForPriority to ensure valid input
                Priority newPrio = promptForPriority();
                prioOpt = newPrio;
            }

            cout << "Update completion status? (y/n): ";
            char cOption;
            cin >> cOption;
            optional<bool> compOpt;
            if (cOption == 'y' || cOption == 'Y') {
                cout << "Mark as completed? (1=Yes, 0=No): ";
                int compVal;
                cin >> compVal;
                if (!cin) {
                    cin.clear();
                    cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                    cerr << "Invalid input for completion.\n";
                    break;
                }
                compOpt = static_cast<bool>(compVal);
            }

            cout << "Update due date? (y/n): ";
            char dateOption;
            cin >> dateOption;
            optional<time_t> dueOpt;
            if (dateOption == 'y' || dateOption == 'Y') {
                time_t newDue = promptForDueDate();
                dueOpt = newDue;
            }

            Task::updateTask(id, descOpt, prioOpt, compOpt, dueOpt);
            cout << "Task updated (if ID was valid).\n";
            break;
        }
        case 3: { // Delete Task
            cout << "Enter task ID to delete: ";
            int id;
            cin >> id;
            if (!cin) {
                cin.clear();
                cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                cerr << "Invalid ID.\n";
                break;
            }
            Task::deleteTask(id);
            cout << "Task deleted (if ID was valid).\n";
            break;
        }
        case 4: { // Mark Task as Completed
            cout << "Enter task ID to mark as completed: ";
            int id;
            cin >> id;
            if (!cin) {
                cin.clear();
                cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                cerr << "Invalid ID.\n";
                break;
            }
            // We only update 'completed' to true
            Task::updateTask(id, {}, {}, true, {});
            cout << "Task marked as completed (if ID was valid).\n";
            break;
        }
        case 5:
            Task::displayTasks();
            break;
        case 6: { // Filter by Status
            cout << "Filter tasks by status:\n"
                << "1. Completed\n"
                << "2. Pending\n"
                << "Enter your choice: ";
            int statusChoice;
            cin >> statusChoice;
            if (!cin) {
                cin.clear();
                cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                cerr << "Invalid choice.\n";
                break;
            }
            if (statusChoice == 1) {
                Task::filterTasksByStatus(true);
            }
            else if (statusChoice == 2) {
                Task::filterTasksByStatus(false);
            }
            else {
                cerr << "Invalid choice.\n";
            }
            break;
        }
        case 7: { // Sort by Priority
            cout << "Sort by priority:\n"
                << "1. Ascending\n"
                << "2. Descending\n"
                << "Enter your choice: ";
            int sortChoice;
            cin >> sortChoice;
            if (!cin) {
                cin.clear();
                cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                cerr << "Invalid choice.\n";
                break;
            }
            if (sortChoice == 1) {
                Task::sortTasksByPriority(true);
            }
            else if (sortChoice == 2) {
                Task::sortTasksByPriority(false);
            }
            else {
                cerr << "Invalid choice.\n";
            }
            break;
        }
        case 8: { // Sort by Due Date
            cout << "Sort by due date:\n"
                << "1. Ascending\n"
                << "2. Descending\n"
                << "Enter your choice: ";
            int sortChoice;
            cin >> sortChoice;
            if (!cin) {
                cin.clear();
                cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
                cerr << "Invalid choice.\n";
                break;
            }
            if (sortChoice == 1) {
                Task::sortTasksByDueDate(true);
            }
            else if (sortChoice == 2) {
                Task::sortTasksByDueDate(false);
            }
            else {
                cerr << "Invalid choice.\n";
            }
            break;
        }
        case 9:
            Task::displayCompletionPercentage();
            break;
        case 10:
            Task::saveTasksToFile(filename);
            cout << "Tasks saved to file.\n";
            break;
        case 0:
            running = false;
            break;
        default:
            cerr << "Invalid menu choice. Please try again.\n";
            break;
        }
    }

    // Optionally, auto-save before exiting.
    // Task::saveTasksToFile(filename);

    cout << "Exiting program. Goodbye.\n";
    return 0;
}
