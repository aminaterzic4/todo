#pragma once
#ifndef MYLIBRARY_H
#define MYLIBRARY_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <optional>
#include <limits>

// For MSVC localtime_s usage (optional)
#include <cstring>  

namespace MyLibrary
{
    enum Priority {
        HIGHEST = 1,
        HIGH,
        MEDIUM,
        LOW,
        LOWEST
    };

    std::string priorityToString(Priority prio);

    /**
     * Safe conversion from string to Priority.
     * @param str The string representation (e.g. "1", "2", "3", "4", "5").
     * @return std::optional<Priority> which is std::nullopt if invalid.
     */
    std::optional<Priority> stringToPrioritySafe(const std::string& str);

    /**
     * Prompt the user until a valid Priority is entered.
     * Returns the chosen Priority.
     */
    Priority promptForPriority();

    /**
     * Prompt for a due date in the format YYYY MM DD (naive).
     * Returns the parsed time_t.
     */
    time_t promptForDueDate();

    class Task {
    private:
        int id;
        std::string description;
        Priority priority;
        bool completed;
        time_t dueDate;

        static int nextId;
        static std::vector<Task> tasks;

        // Validate description & priority
        static bool validateTask(const std::string& desc, Priority prio);

    public:
        // Constructor
        Task(const std::string& desc, Priority prio, time_t due);

        // ---------- Static Methods ----------
        static void loadTasksFromFile(const std::string& filename);
        static void saveTasksToFile(const std::string& filename);
        static void addTask(const std::string& desc, Priority prio, time_t due);
        static void deleteTask(int id);
        static void updateTask(int id,
            std::optional<std::string> desc = {},
            std::optional<Priority> prio = {},
            std::optional<bool> comp = {},
            std::optional<time_t> due = {});
        static void displayTasks();
        static void sortTasksByPriority(bool ascending = true);
        static void sortTasksByDueDate(bool ascending = true);
        static void filterTasksByStatus(bool completedStatus);
        static void displayCompletionPercentage();
    };

} // end namespace MyLibrary

#endif // MYLIBRARY_H
