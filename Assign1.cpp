#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <sqlite3.h>

using namespace std;


// Function to check if the database connection is successful
bool connectToDatabase(sqlite3** db) {
    int rc = sqlite3_open("car_rental.db", db);
    if (rc != SQLITE_OK) {
        cerr << "Error opening database: " << sqlite3_errmsg(*db) << endl;
        return false;
    }
    return true;
}

bool askConfirmation(const string& message) {
        char choice;
        cout << message << " (y/n): ";
        cin >> choice;
        cin.ignore(); // Consume newline character
        return tolower(choice) == 'y';
}

// Base class for users
class User {
protected:
    string name;
    int id;
    string password;
public:
    User(string n, int i, string p) : name(n), id(i), password(p) {}
    // virtual ~User() {}

    virtual void displayDetails() const {
        cout << "Name: " << name << ", ID: " << id << endl;
    }
};

// Class for customers
class Customer : public User {
private:
    int rentedCars;
    double fineDue;
    double customerRecord; // For simplicity, considering a single customer record value
public:
    Customer(string n, int i, string p) : User(n, i, p), rentedCars(0), fineDue(0), customerRecord(0) {}

    void rentCar() {
        // Code to rent a car
        sqlite3* db;
        if (!connectToDatabase(&db)) return;

        // Check if car is available
        string sql = "SELECT * FROM cars WHERE available=1";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cout << "No cars available to rent." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display all available car details
        cout << "Available cars:" << endl;
        int count = 1;
        do {
            int carId = sqlite3_column_int(stmt, 0);
            string model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string year = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            // Additional car details can be displayed if available in the table

            cout << count << ". " << model << " (" << year << ")" << endl;
            count++;
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        // Get user input for car ID
        int chosenId;
        cout << "Enter the ID of the car you want to rent: ";
        cin >> chosenId;

        // Validate user input
        bool foundCar = false;
        sqlite3_reset(stmt); // Reset the statement to the beginning
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int carId = sqlite3_column_int(stmt, 0);
            if (carId == chosenId) {
                foundCar = true;
                break;
            }
        }

        if (!foundCar) {
            cout << "Invalid car ID. Please choose from the list above." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Ask for confirmation and rent car
        if (askConfirmation("Do you want to rent this car?")) {
            // Update car availability and user rented cars (same as before)

            sql = "UPDATE cars SET available=0 WHERE id=?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, chosenId);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating car availability: " << sqlite3_errmsg(db) << endl;
            }
            sql = "UPDATE customers SET rentedCars=rentedCars+1 WHERE id=?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating customer rented cars: " << sqlite3_errmsg(db) << endl;
            }
            cout << "Car rented successfully." << endl;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    void returnCar() {
        // Code to return a car
        sqlite3* db;
        if (!connectToDatabase(&db)) return;

        // Check if customer has rented cars
        string sql = "SELECT * FROM customers WHERE id=? AND rentedCars>0";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cout << "You haven't rented any cars." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Get car details and user confirmation
        int carId = sqlite3_column_int(stmt, 3); // Assuming car_id is stored in the 3rd column of the customer table
        string model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)); // Assuming car model is stored in the 4th column of the customer table
        cout << "You have rented car: " << model << endl;
        if (askConfirmation("Do you want to return this car?")) {
            // Update car availability and user rented cars
            sql = "UPDATE cars SET available=1 WHERE id=?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, carId);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating car availability: " << sqlite3_errmsg(db) << endl;
            }
            sql = "UPDATE customers SET rentedCars=rentedCars-1 WHERE id=?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating customer rented cars: " << sqlite3_errmsg(db) << endl;
            }
            cout << "Car returned successfully." << endl;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    void clear_dues(){
        // Code to clear dues
        sqlite3* db;
        if (!connectToDatabase(&db)) return;

        // Check if customer has any dues
        string sql = "SELECT * FROM customers WHERE id=? AND (fineDue>0 OR customerRecord<1)"; // Adjust condition based on your criteria for having dues
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cout << "You don't have any outstanding dues." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display current dues and ask for confirmation
        double fine = sqlite3_column_double(stmt, 1); // Assuming fineDue is stored in the 2nd column of the customer table
        double record = sqlite3_column_double(stmt, 2); // Assuming customerRecord is stored in the 3rd column
        cout << "Your current dues:" << endl;
        cout << "- Fine: $" << fine << endl;
        cout << "- Customer Record: " << record << endl;
        
        if (!askConfirmation("Do you want to clear your dues?")) {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Update customer record (assuming it can be improved through payment)
        sql = "UPDATE customers SET customerRecord=1 WHERE id=?"; // Adjust update logic based on your criteria for improving customer record
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error updating customer record: " << sqlite3_errmsg(db) << endl;
        }

        // Reset fine due
        sql = "UPDATE customers SET fineDue=0 WHERE id=?";
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error resetting fine due: " << sqlite3_errmsg(db) << endl;
        }

        cout << "Dues cleared successfully." << endl;

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // void browseCars() {
    //     // Code to browse available cars
    // }

    void displayDetails() const override {
        User::displayDetails();
        cout << "Role: Customer" << endl;
        cout << "Rented Cars: " << rentedCars << ", Fine Due: " << fineDue << ", Record: " << customerRecord << endl;
    }
};

// Class for employees
class Employee : public User {
private:
    int rentedCars;
    double fineDue;
    double employeeRecord; // For simplicity, considering a single employee record value
public:
    Employee(string n, int i, string p) : User(n, i, p), rentedCars(0), fineDue(0), employeeRecord(0) {}

    void rentCar() {
        // Code to rent a car
    }

    void returnCar() {
        // Code to return a car
    }

    void clear_dues(){
        // Code to clear dues
    }

    // void browseCars() {
    //     // Code to browse available cars
    // }

    void displayDetails() const override {
        User::displayDetails();
        cout << "Role: Employee" << endl;
        cout << "Rented Cars: " << rentedCars << ", Fine Due: " << fineDue << ", Record: " << employeeRecord << endl;
    }
};

// Class for manager
class Manager : public User {
public:
    Manager(string n, int i, string p) : User(n, i, p) {}

    void addCustomer() {
        // Code to add a customer
    }

    void updateCustomer() {
        // Code to update a customer
    }

    void deleteCustomer() {
        // Code to delete a customer
    }

    void addEmployee() {
        // Code to add an employee
    }

    void updateEmployee() {
        // Code to update an employee
    }

    void deleteEmployee() {
        // Code to delete an employee
    }

    void addCar() {
        // Code to add a car
    }

    void updateCar() {
        // Code to update a car
    }

    void deleteCar() {
        // Code to delete a car
    }

    void displayAllCars() {
        // Code to display all cars
    }

    void displayDetails() const override {
        User::displayDetails();
        cout << "Role: Manager" << endl;
    }
};


// Class for cars
class Car {
private:
    string model;
    string condition;
public:
    Car(string m, string c) : model(m), condition(c) {}

    void rentRequest() {
        // Code to process rent request
    }

    void showDueDate() {
        // Code to show due date
    }

    void displayDetails() const {
        cout << "Model: " << model << ", Condition: " << condition << endl;
    }
};

// Function to create the database tables
void createDatabase(sqlite3* db) {
    // SQL statements to create cars and customers tables
    string sql_cars = "CREATE TABLE IF NOT EXISTS cars (id INTEGER PRIMARY KEY AUTOINCREMENT, model TEXT NOT NULL, year TEXT, available INTEGER NOT NULL DEFAULT 1)";
    string sql_customers = "CREATE TABLE IF NOT EXISTS customers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, password TEXT NOT NULL, rentedCars INTEGER NOT NULL DEFAULT 0, fineDue DOUBLE NOT NULL DEFAULT 0, customerRecord DOUBLE NOT NULL DEFAULT 0)";

    // Execute statements and handle errors
    char* errmsg;
    if (sqlite3_exec(db, sql_cars.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        cerr << "Error creating cars table: " << errmsg << endl;
        sqlite3_free(errmsg);
    }
    if (sqlite3_exec(db, sql_customers.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
        cerr << "Error creating customers table: " << errmsg << endl;
        sqlite3_free(errmsg);
    }
}

// Function to add car objects
void addCars(sqlite3* db, const vector<pair<string, string>>& cars) {
    // Prepare and execute SQL statement for each car
    string sql = "INSERT INTO cars (model, year) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Error preparing statement for adding cars: " << sqlite3_errmsg(db) << endl;
        return;
    }

    for (const auto& car : cars) {
        sqlite3_bind_text(stmt, 1, car.first.c_str(), -1, SQLITE_TRANSIENT); // Model
        sqlite3_bind_text(stmt, 2, car.second.c_str(), -1, SQLITE_TRANSIENT); // Year
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error inserting car: " << sqlite3_errmsg(db) << endl;
        }
        sqlite3_reset(stmt); // Reset statement for next insertion
    }

    sqlite3_finalize(stmt);
}

// Function to add a customer
void addCustomer(sqlite3* db, string name, string password) {
    string sql = "INSERT INTO customers (name, password) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Error preparing statement for adding customer: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cerr << "Error inserting customer: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;
    if (!connectToDatabase(&db)) {
        return 1;
    }

    // Create tables if they don't exist (replace with actual creation logic)
    createDatabase(db);

    // Add sample car data (replace with your actual data)
    vector<pair<string, string>> cars = {
        {"Model A", "2023"},
        {"Model B", "2022"}
    };
    addCars(db, cars);

    // Add a sample customer (replace with actual name and password)
    addCustomer(db, "John Doe", "password123");

    // Use the rentCar function
    int userId = 1; // Assuming customer ID is retrieved after adding the customer
    Customer customer("John Doe", userId, "password123"); // Initialize customer object
    customer.rentCar();

    sqlite3_close(db);
    return 0;
}



