#include <sqlite3.h>
#include <iostream>
#include <string>
#include <limits>

class DatabaseManager {
private:
    sqlite3* db;
    char* errMsg = nullptr;

    // Vienota funkcija, kas palīdz izsaukt SQL vaicājumus
    bool executeSQL(const std::string& sql) {
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

public:
    DatabaseManager() : db(nullptr) {}

    bool initialize() {
        // Atver DB kas glabājās operatīvā atmiņā
        if (sqlite3_open(":memory:", &db)) {
            std::cerr << "Failed to open in-memory DB: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        // Izveido lietotāju tabulu
        const char* queryCreateTable = "CREATE TABLE users(id INTEGER PRIMARY KEY, name TEXT, email TEXT);";
        return executeSQL(queryCreateTable);
    }

    void close() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    // CREATE darbība
    bool addUser(const std::string& name, const std::string& email) {
        std::string queryAddUser = "INSERT INTO users(name, email) VALUES('" + name + "', '" + email + "');";
        return executeSQL(queryAddUser);
    }

    bool isUserExists(int id) {
        std::string query = "SELECT id, name, email FROM users WHERE id = " + std::to_string(id) + ";";
        return executeSQL(query);
    }

    // READ darbība
    void displayAllUsers() {
        const char* querySelectAllUsers = "SELECT id, name, email FROM users;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, querySelectAllUsers, -1, &stmt, nullptr) == SQLITE_OK) {
            std::cout << "\n--- User List ---\n";
            std::cout << "ID\tName\t\tEmail\n";
            std::cout << "-------------------------------\n";

            bool hasData = false;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                hasData = true;
                int id = sqlite3_column_int(stmt, 0);
                const unsigned char* name = sqlite3_column_text(stmt, 1);
                const unsigned char* email = sqlite3_column_text(stmt, 2);

                std::cout << id << "\t" << name << "\t\t" << email << std::endl;
            }

            if (!hasData) {
                std::cout << "No users found in the database.\n";
            }
            std::cout << "-------------------------------\n";
        } else {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_finalize(stmt);
    }

    bool getUserById(int id) {
        std::string query = "SELECT id, name, email FROM users WHERE id = " + std::to_string(id) + ";";
        sqlite3_stmt* stmt;
        bool found = false;

        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                found = true;
                int userId = sqlite3_column_int(stmt, 0);
                const unsigned char* name = sqlite3_column_text(stmt, 1);
                const unsigned char* email = sqlite3_column_text(stmt, 2);

                std::cout << "\n--- User Details ---\n";
                std::cout << "ID: " << userId << std::endl;
                std::cout << "Name: " << name << std::endl;
                std::cout << "Email: " << email << std::endl;
            } else {
                std::cout << "User with ID " << id << " not found.\n";
            }
        } else {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_finalize(stmt);
        return found;
    }

    // UPDATE darbība
    bool updateUser(int id, const std::string& name, const std::string& email) {
        if (!isUserExists(id)) {
            return false;
        }

        std::string sql = "UPDATE users SET name = '" + name + "', email = '" + email +
                          "' WHERE id = " + std::to_string(id) + ";";
        bool success = executeSQL(sql);

        if (success) {
            std::cout << "User updated successfully!\n";
        }

        return success;
    }

    // DELETE darbība
    bool deleteUser(int id) {
        if (!isUserExists(id)) {
            return false;
        }

        std::string sql = "DELETE FROM users WHERE id = " + std::to_string(id) + ";";
        bool success = executeSQL(sql);

        if (success) {
            std::cout << "User deleted successfully!\n";
        }

        return success;
    }
};

void displayMenu() {
    std::cout << "\n===== User Management System =====\n";
    std::cout << "1. Add a new user\n";
    std::cout << "2. Display all users\n";
    std::cout << "3. Find user by ID\n";
    std::cout << "4. Update user information\n";
    std::cout << "5. Delete a user\n";
    std::cout << "6. Exit\n";
    std::cout << "Enter your choice (1-6): ";
}

int getMenuChoice() {
    int choice;
    std::cin >> choice;

    while (std::cin.fail() || choice < 1 || choice > 6) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid choice. Please enter a number between 1 and 6: ";
        std::cin >> choice;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return choice;
}

int getIntInput(const std::string& prompt) {
    int value;
    std::cout << prompt;
    std::cin >> value;

    while (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. " << prompt;
        std::cin >> value;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

std::string getStringInput(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return value;
}

void addSampleData(DatabaseManager& dbManager) {
    std::cout << "Adding sample users to the database...\n";
    dbManager.addUser("Alice Smith", "alice@example.com");
    dbManager.addUser("Bob Johnson", "bob@example.com");
    dbManager.addUser("Charlie Brown", "charlie@example.com");
    std::cout << "Sample users added successfully!\n";
}

int main() {
    DatabaseManager dbManager;

    if (!dbManager.initialize()) {
        std::cerr << "Failed to initialize database. Exiting.\n";
        return 1;
    }

    std::cout << "Database initialized successfully.\n";

    addSampleData(dbManager);

    bool running = true;
    while (running) {
        displayMenu();
        int choice = getMenuChoice();

        switch (choice) {
            case 1: {  // Add user
                std::string name = getStringInput("Enter user name: ");
                std::string email = getStringInput("Enter user email: ");

                if (dbManager.addUser(name, email)) {
                    std::cout << "User added successfully!\n";
                } else {
                    std::cout << "Failed to add user.\n";
                }
                break;
            }

            case 2:  // Display all users
                dbManager.displayAllUsers();
                break;

            case 3: {  // Find user by ID
                int id = getIntInput("Enter user ID to find: ");
                dbManager.getUserById(id);
                break;
            }

            case 4: {  // Update user
                int id = getIntInput("Enter user ID to update: ");
                std::string name = getStringInput("Enter new name: ");
                std::string email = getStringInput("Enter new email: ");

                dbManager.updateUser(id, name, email);
                break;
            }

            case 5: {  // Delete user
                int id = getIntInput("Enter user ID to delete: ");
                dbManager.deleteUser(id);
                break;
            }

            case 6:  // Exit
                std::cout << "Exiting application...\n";
                running = false;
                break;
            default: ;
        }

        if (running) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
    }

    dbManager.close();

    return 0;
}