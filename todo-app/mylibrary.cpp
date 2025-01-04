#include "mylibrary.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS  // If you want to suppress MSVC localtime deprecation warnings
#endif

namespace MyLibrary
{
    /*--------------------- STATIC MEMBER DEFINITIONS ---------------------*/
    int Task::nextId = 1;
    std::vector<Task> Task::tasks;

    /*--------------------- FREE FUNCTION IMPLEMENTATIONS -----------------*/

    std::string priorityToString(Priority prio) {
        switch (prio) {
        case HIGHEST: return "Highest";
        case HIGH:    return "High";
        case MEDIUM:  return "Medium";
        case LOW:     return "Low";
        case LOWEST:  return "Lowest";
        default:      return "Unknown";
        }
    }

    std::optional<Priority> stringToPrioritySafe(const std::string& str) {
        if (str == "1")      return HIGHEST;
        else if (str == "2") return HIGH;
        else if (str == "3") return MEDIUM;
        else if (str == "4") return LOW;
        else if (str == "5") return LOWEST;
        return {};
    }

    Priority promptForPriority() {
        while (true) {
            std::cout << "Enter priority (1=Highest, 2=High, 3=Medium, 4=Low, 5=Lowest): ";
            std::string prioStr;
            std::cin >> prioStr;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid input. Please enter a number (1-5).\n";
                continue;
            }
            auto prioOpt = stringToPrioritySafe(prioStr);
            if (prioOpt.has_value()) {
                return prioOpt.value();
            }
            else {
                std::cerr << "Invalid priority. Must be 1 to 5.\n";
            }
        }
    }

    time_t promptForDueDate() {
        while (true) {
            std::cout << "Enter due date (YYYY MM DD): ";
            int year, month, day;
            std::cin >> year >> month >> day;
            if (!std::cin) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "Invalid date input. Please try again.\n";
                continue;
            }

            tm timeStruct = {};
            timeStruct.tm_year = year - 1900; // years since 1900
            timeStruct.tm_mon = month - 1;   // months since January
            timeStruct.tm_mday = day;
            timeStruct.tm_hour = 12;         // set to noon to avoid DST issues
            timeStruct.tm_min = 0;
            timeStruct.tm_sec = 0;

            time_t due = mktime(&timeStruct);
            if (due == -1) {
                std::cerr << "Failed to parse that date. Please try again.\n";
                continue;
            }
            return due;
        }
    }

    /*-------------------- TASK CLASS IMPLEMENTATIONS ---------------------*/

    Task::Task(const std::string& desc, Priority prio, time_t due)
        : id(nextId++), description(desc), priority(prio), completed(false), dueDate(due)
    {}

    bool Task::validateTask(const std::string& desc, Priority prio) {
        if (desc.empty()) {
            std::cerr << "Error: Description cannot be empty.\n";
            return false;
        }
        if (prio < HIGHEST || prio > LOWEST) {
            std::cerr << "Error: Priority must be between 1 and 5.\n";
            return false;
        }
        return true;
    }

    void Task::loadTasksFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            // If file not found, not necessarily an error; do nothing
            return;
        }

        tasks.clear();
        while (true) {
            if (!file.good() || file.peek() == EOF) break;

            std::string desc;
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

            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (desc.empty() || prioInt < HIGHEST || prioInt > LOWEST) {
                std::cerr << "Skipping invalid task from file.\n";
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

    void Task::saveTasksToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for saving.\n";
            return;
        }
        for (auto& task : tasks) {
            if (!validateTask(task.description, task.priority)) {
                std::cerr << "Error: Invalid Task with ID " << task.id << " - not saved.\n";
                continue;
            }
            // Format: description|priority completed dueDate
            file << task.description << "|"
                << task.priority << " "
                << task.completed << " "
                << task.dueDate << "\n";
        }
        file.close();
    }

    void Task::addTask(const std::string& desc, Priority prio, time_t due) {
        if (validateTask(desc, prio)) {
            tasks.push_back(Task(desc, prio, due));
        }
    }

    void Task::deleteTask(int id) {
        size_t oldSize = tasks.size();
        tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
            [id](const Task& t) { return t.id == id; }),
            tasks.end());
        if (tasks.size() == oldSize) {
            std::cerr << "Warning: No task found with ID " << id << ".\n";
        }
    }

    void Task::updateTask(int id,
        std::optional<std::string> desc,
        std::optional<Priority> prio,
        std::optional<bool> comp,
        std::optional<time_t> due)
    {
        for (auto& task : tasks) {
            if (task.id == id) {
                if (desc) {
                    if (!validateTask(*desc, task.priority)) {
                        std::cerr << "Update failed due to invalid description.\n";
                        return;
                    }
                    task.description = *desc;
                }
                if (prio) {
                    if (!validateTask(task.description, *prio)) {
                        std::cerr << "Update failed due to invalid priority.\n";
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
        std::cerr << "Warning: No task found with ID " << id << ".\n";
    }

    void Task::displayTasks() {
        if (tasks.empty()) {
            std::cout << "No tasks available.\n";
            return;
        }

        std::cout << std::left << std::setw(5) << "ID"
            << std::setw(25) << "Description"
            << std::setw(10) << "Priority"
            << std::setw(10) << "Status"
            << std::setw(20) << "Due Date"
            << std::endl;

        for (auto& task : tasks) {
            char buffer[20];
            tm timeStruct{};
            // localtime_s returns 0 on success (MSVC).
            if (localtime_s(&timeStruct, &task.dueDate) == 0) {
                strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeStruct);
            }
            else {
                strcpy_s(buffer, "InvalidDate");
            }

            std::cout << std::left << std::setw(5) << task.id
                << std::setw(25) << task.description
                << std::setw(10) << priorityToString(task.priority)
                << std::setw(10) << (task.completed ? "Completed" : "Pending")
                << std::setw(20) << buffer
                << std::endl;
        }
    }

    void Task::sortTasksByPriority(bool ascending) {
        std::sort(tasks.begin(), tasks.end(),
            [ascending](const Task& a, const Task& b) {
                return ascending ? (a.priority < b.priority)
                    : (a.priority > b.priority);
            });
    }

    void Task::sortTasksByDueDate(bool ascending) {
        std::sort(tasks.begin(), tasks.end(),
            [ascending](const Task& a, const Task& b) {
                return ascending ? (a.dueDate < b.dueDate)
                    : (a.dueDate > b.dueDate);
            });
    }

    void Task::filterTasksByStatus(bool completedStatus) {
        bool foundAny = false;
        std::cout << std::left << std::setw(5) << "ID"
            << std::setw(25) << "Description"
            << std::setw(10) << "Priority"
            << std::setw(10) << "Status"
            << std::setw(20) << "Due Date"
            << std::endl;

        for (auto& task : tasks) {
            if (task.completed == completedStatus) {
                foundAny = true;
                char buffer[20];
                tm timeStruct{};
                if (localtime_s(&timeStruct, &task.dueDate) == 0) {
                    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeStruct);
                }
                else {
                    strcpy_s(buffer, "InvalidDate");
                }

                std::cout << std::left << std::setw(5) << task.id
                    << std::setw(25) << task.description
                    << std::setw(10) << priorityToString(task.priority)
                    << std::setw(10) << (task.completed ? "Completed" : "Pending")
                    << std::setw(20) << buffer
                    << std::endl;
            }
        }
        if (!foundAny) {
            std::cout << "No tasks found with status: "
                << (completedStatus ? "Completed" : "Pending") << std::endl;
        }
    }

    void Task::displayCompletionPercentage() {
        if (tasks.empty()) {
            std::cout << "No tasks. Completion percentage: 0%\n";
            return;
        }
        int completedCount = 0;
        for (auto& task : tasks) {
            if (task.completed) {
                completedCount++;
            }
        }
        double percentage = (static_cast<double>(completedCount) / tasks.size()) * 100.0;
        std::cout << "Completion Percentage: "
            << std::fixed << std::setprecision(2) << percentage << "%\n";
    }

} // end namespace MyLibrary
