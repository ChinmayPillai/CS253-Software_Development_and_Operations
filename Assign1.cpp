#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>

using namespace std;

bool askConfirmation(const string& message) {
    char choice;
    cout << message << " (y/n): ";
    cin >> choice;
    cin.ignore(); // Consume newline character
    return tolower(choice) == 'y';
}

string filename = "car_rental.db";

class Db{

protected:
    string tablename;
    
    Db(){
        sqlite3* db;
        if (connectToDatabase(&db)) {
            cout << "Database  connection successful." << endl;
        } 
    }

    bool isTableEmpty(sqlite3* db, string& table_name) {

        string sql = "SELECT 1 FROM " + table_name + ";";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return true;
        }

        sqlite3_finalize(stmt);
        return false;
    }

    // Function to create the database tables
    void createTable(sqlite3* db, string sql) {
        cout<< "Creating table: "<< tablename << endl;
        // Execute statement and handle errors
        char* errmsg;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK) {
            cerr << "Error creating "<< tablename <<" table: " << errmsg << endl;
            sqlite3_free(errmsg);
        }
    }

public:
    // Function to connect to db and check if the database connection is successful
    static bool connectToDatabase(sqlite3** db) {
        int rc = sqlite3_open(filename.c_str(), db);
        if (rc != SQLITE_OK) {
            cerr << "Error opening database: " << sqlite3_errmsg(*db) << endl;
            return false;
        }
        return true;
    }

    void deleteRecord(int id, sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }
        if (search(id, db)) {
            string sql = "DELETE FROM " + tablename + " WHERE id = ?;";

            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                cerr << "Error preparing statement for deleting: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the value of id to the prepared statement
            sqlite3_bind_int(stmt, 1, id);

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    bool search(int id, sqlite3* db = nullptr) {
        if(db == nullptr){
            if (!connectToDatabase(&db)) return false;
        }
        string sql = "SELECT EXISTS (SELECT 1 FROM " + tablename + " WHERE id = ?);";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement for searching: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        // Bind the value of id to the prepared statement
        sqlite3_bind_int(stmt, 1, id);

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Check if EXISTS returned 1 (first column)
        bool exists = sqlite3_column_int(stmt, 0) == 1;

        sqlite3_finalize(stmt);
        return exists;
    }
};

class CarDb : public Db{
private:
    vector<vector<string>> defaultData = {
        {"Model A", "2023"},
        {"Model B", "2022"},
        {"Model C", "2021"},
        {"Model D", "2020"},
        {"Model E", "2019"} 
    };

    void load(sqlite3* db){
        if (isTableEmpty(db, tablename)) {
            cout << "Loading cars..." << endl;
            for(vector<string> data : defaultData){
                add(data, db);
            }
        }
    }


public:
    CarDb(){
        tablename = "cars";
        sqlite3* db;
        connectToDatabase(&db);
        string sql = "CREATE TABLE IF NOT EXISTS cars (id INTEGER PRIMARY KEY AUTOINCREMENT, model TEXT NOT NULL, year TEXT, available INTEGER NOT NULL DEFAULT 1)";
        //string sql_customers = "CREATE TABLE IF NOT EXISTS customers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, password TEXT NOT NULL, rentedCars INTEGER NOT NULL DEFAULT 0, fineDue DOUBLE NOT NULL DEFAULT 0, customerRecord DOUBLE NOT NULL DEFAULT 0)";
        createTable(db, sql);
        load(db);
    }


    void add(const vector<string> car, sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }
        string sql = "INSERT INTO cars (model, year) VALUES (?, ?)";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement for adding cars: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        // Bind the values of t to the prepared statement
        sqlite3_bind_text(stmt, 1, car[0].c_str(), -1, SQLITE_TRANSIENT); // Model
        sqlite3_bind_text(stmt, 2, car[1].c_str(), -1, SQLITE_TRANSIENT); // Year
        
        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error inserting " << tablename << ": " << sqlite3_errmsg(db) << endl;
        }
        cout<< "Car "<< car[0] <<"("<<car[1]<<")" <<" added successfully." << endl;
        sqlite3_finalize(stmt);
    }

    void update(int id, const vector<string>& car, sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }
        if (search(id, db)) {
            string sql = "UPDATE " + tablename + " SET model = ?, year = ? WHERE id = ?;";

            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                cerr << "Error preparing statement for updating: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the values of car to the prepared statement
            sqlite3_bind_text(stmt, 1, car[0].c_str(), -1, SQLITE_TRANSIENT); // Model
            sqlite3_bind_text(stmt, 2, car[1].c_str(), -1, SQLITE_TRANSIENT); // Year
            sqlite3_bind_int(stmt, 3, id); // ID

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating car: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    void display(sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }

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
        cout << "Displaying " << tablename << " table:" << endl;
        do {
            int carId = sqlite3_column_int(stmt, 0);
            string model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string year = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            // Additional car details can be displayed if available in the table

            cout << carId << ". " << model << " (" << year << ")" << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
    }
};

class CustomerDb : public Db{
    private:
    vector<vector<string>> defaultData = {
        {"Linus","5000", "0", "0", "5"},
        {"Elon","50000", "0", "0", "10"},
        {"Steve","10000", "0", "0", "7"},
        {"Bill","25000", "0", "0", "8"},
        {"John","100", "0", "0", "3"}
    };

    void load(sqlite3* db){
        if (isTableEmpty(db, tablename)) {
            cout << "Loading customers..." << endl;
            for(vector<string> data : defaultData){
                add(data, db);
            }
        }
    }

public:
    CustomerDb(){
        tablename = "customers";
        sqlite3* db;
        connectToDatabase(&db);
        string sql= "CREATE TABLE IF NOT EXISTS customers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, money INTEGER NOT NULL DEFAULT 5000, rentedCars INTEGER NOT NULL DEFAULT 0, fineDue INTEGER NOT NULL DEFAULT 0, customerRecord INTEGER NOT NULL DEFAULT 5)";
        createTable(db, sql);
        load(db);
    }

    void add(const vector<string> cus, sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }
        string sql = "INSERT INTO customers (name, money, rentedCars, fineDue, customerRecord) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement for adding customers: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        // Bind the values of t to the prepared statement
        sqlite3_bind_text(stmt, 1, cus[0].c_str(), -1, SQLITE_TRANSIENT); // Name
        sqlite3_bind_text(stmt, 2, cus[1].c_str(), -1, SQLITE_TRANSIENT); // Money
        sqlite3_bind_text(stmt, 3, cus[2].c_str(), -1, SQLITE_TRANSIENT); // rentedCars
        sqlite3_bind_text(stmt, 4, cus[3].c_str(), -1, SQLITE_TRANSIENT); // fineDue
        sqlite3_bind_text(stmt, 5, cus[4].c_str(), -1, SQLITE_TRANSIENT); // record
        
        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error inserting " << tablename << ": " << sqlite3_errmsg(db) << endl;
        }
        cout<< "Customer "<< cus[0] <<" added successfully." << endl;
        sqlite3_finalize(stmt);
    }

    void update(int id, const vector<string>& cus, sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }
        if (search(id, db)) {
            string sql = "UPDATE " + tablename + " SET name = ?, money = ?, rentedCars = ?, fineDue = ?, customerRecord = ? WHERE id = ?;";

            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                cerr << "Error preparing statement for updating: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the values of t to the prepared statement
            sqlite3_bind_text(stmt, 1, cus[0].c_str(), -1, SQLITE_TRANSIENT); // Name
            sqlite3_bind_text(stmt, 2, cus[1].c_str(), -1, SQLITE_TRANSIENT); // Money
            sqlite3_bind_text(stmt, 3, cus[2].c_str(), -1, SQLITE_TRANSIENT); // rentedCars
            sqlite3_bind_text(stmt, 4, cus[3].c_str(), -1, SQLITE_TRANSIENT); // fineDue
            sqlite3_bind_text(stmt, 5, cus[4].c_str(), -1, SQLITE_TRANSIENT); // record
            sqlite3_bind_int(stmt, 6, id); // ID

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating customers: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    void display(sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }

        string sql = "SELECT * FROM customers";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cout << "No customers." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display all available car details
        cout << "Displaying " << tablename << " table:" << endl;
        do {
            int cusId = sqlite3_column_int(stmt, 0);
            string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string money = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            string rentedCars = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            string fineDue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            string record = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            // Additional car details can be displayed if available in the table

            cout << cusId << ". " << name << ", $" << money << ", " << rentedCars << " cars rented, Fine Due: $" << fineDue << ", Customer Record: " << record << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
    }
};

class EmployeeDb : public Db{
    private:
    vector<vector<string>> defaultData = {
        {"Emp1","500", "0", "0", "7"},
        {"Emp2","5000","0","0","5"},
        {"Emp3","1000","0","0","6"},
        {"Emp4","2500","0","0","8"},
        {"Emp5","10","0","0","3"}
    };

    void load(sqlite3* db){
        if (isTableEmpty(db, tablename)) {
            cout << "Loading employees..." << endl;
            for(vector<string> data : defaultData){
                add(data, db);
            }
        }
    }


public:
    EmployeeDb(){
        tablename = "employees";
        sqlite3* db;
        connectToDatabase(&db);
        string sql= "CREATE TABLE IF NOT EXISTS employees (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, money INTEGER NOT NULL DEFAULT 500, rentedCars INTEGER NOT NULL DEFAULT 0, fineDue INTEGER NOT NULL DEFAULT 0, employeeRecord INTEGER NOT NULL DEFAULT 7)";
        createTable(db, sql);
        load(db);
    }


    void add(const vector<string> emp, sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }
        string sql = "INSERT INTO employees (name, money, rentedCars, fineDue, employeeRecord) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement for adding employees: " << sqlite3_errmsg(db) << endl;
            return;
        }
        
        // Bind the values of t to the prepared statement
        sqlite3_bind_text(stmt, 1, emp[0].c_str(), -1, SQLITE_TRANSIENT); // Name
        sqlite3_bind_text(stmt, 2, emp[1].c_str(), -1, SQLITE_TRANSIENT); // Money
        sqlite3_bind_text(stmt, 3, emp[2].c_str(), -1, SQLITE_TRANSIENT); // rentedCars
        sqlite3_bind_text(stmt, 4, emp[3].c_str(), -1, SQLITE_TRANSIENT); // fineDue
        sqlite3_bind_text(stmt, 5, emp[4].c_str(), -1, SQLITE_TRANSIENT); // record
        
        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error inserting " << tablename << ": " << sqlite3_errmsg(db) << endl;
        }
        cout<< "Employee "<< emp[0] <<" added successfully." << endl;
        sqlite3_finalize(stmt);
    }

    void update(int id, const vector<string>& emp, sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }
        if (search(id, db)) {
            string sql = "UPDATE " + tablename + " SET name = ?, money = ?, rentedCars = ?, fineDue = ?, employeeRecord = ? WHERE id = ?;";

            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                cerr << "Error preparing statement for updating: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the values of t to the prepared statement
            sqlite3_bind_text(stmt, 1, emp[0].c_str(), -1, SQLITE_TRANSIENT); // Name
            sqlite3_bind_text(stmt, 2, emp[1].c_str(), -1, SQLITE_TRANSIENT); // Money
            sqlite3_bind_text(stmt, 3, emp[2].c_str(), -1, SQLITE_TRANSIENT); // rentedCars
            sqlite3_bind_text(stmt, 4, emp[3].c_str(), -1, SQLITE_TRANSIENT); // fineDue
            sqlite3_bind_text(stmt, 5, emp[4].c_str(), -1, SQLITE_TRANSIENT); // record
            sqlite3_bind_int(stmt, 6, id); // ID

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating employees: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    void display(sqlite3* db = nullptr){
        if(db == nullptr){
            if (!connectToDatabase(&db)) return;
        }

        string sql = "SELECT * FROM employees";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            cout << "No employees." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display all available car details
        cout << "Displaying " << tablename << " table:" << endl;
        do {
            int empId = sqlite3_column_int(stmt, 0);
            string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string money = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            string rentedCars = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            string fineDue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            string record = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            // Additional car details can be displayed if available in the table

            cout << empId << ". " << name << ", $" << money << ", " << rentedCars << " cars rented, Fine Due: $" << fineDue << ", Employee Record: " << record << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
    }
};


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

class RentableUser : public User {

protected:
    RentableUser(string n, int i, string p) : User(n, i, p) {}

public:
    void rentCar() {
        // Code to rent a car
        sqlite3* db;
        if (!Db::connectToDatabase(&db)) return;

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
        do {
            int carId = sqlite3_column_int(stmt, 0);
            string model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string year = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            // Additional car details can be displayed if available in the table

            cout << carId << ". " << model << " (" << year << ")" << endl;
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
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return;
            }
            sql = "UPDATE customers SET rentedCars=rentedCars+1 WHERE id=?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                cerr << "Error updating customer rented cars: " << sqlite3_errmsg(db) << endl;
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return;
            }
            cout << "Car rented successfully." << endl;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    void returnCar() {
        // Code to return a car
        sqlite3* db;
        if (!Db::connectToDatabase(&db)) return;

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

        // Display all rented car details
    cout << "Your rented cars:" << endl;
    do {
        int customer_id = sqlite3_column_int(stmt, 0);
        string customer_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int carId = sqlite3_column_int(stmt, 2);
        string model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        string year = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        cout << customer_id << ". " << model << " (" << year << ") - Rented by: " << customer_name << endl;
    } while (sqlite3_step(stmt) == SQLITE_ROW);

    // Get user input for car ID
    int chosenId;
    cout << "Enter the ID of the car you want to return: ";
    cin >> chosenId;

    // Validate user input
    bool foundCar = false;
    sqlite3_reset(stmt); // Reset the statement to the beginning
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int carId = sqlite3_column_int(stmt, 2);
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

    // Ask for confirmation and return car
    if (askConfirmation("Do you want to return this car?")) {
        // Update car availability and user rented cars (same as before)

        sql = "UPDATE cars SET available=1 WHERE id=?";
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, chosenId);
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
        if (!Db::connectToDatabase(&db)) return;

        // Check if customer has any dues
        string sql = "SELECT * FROM customers WHERE id=? AND (fineDue>0)"; // Adjust condition based on your criteria for having dues
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
        double dues = sqlite3_column_double(stmt, 1); // Assuming fineDue is stored in the 2nd column of the customer table
        double record = sqlite3_column_double(stmt, 2); // Assuming customerRecord is stored in the 3rd column
        cout << "Your current dues:" << endl;
        cout << "- Dues: $" << dues << endl;
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

    
    void browseRentedCars() {
        // Code to browse available cars
    }
};

// Class for customers
class Customer : public RentableUser {
private:
    int rentedCars;
    double fineDue;
    double customerRecord; // For simplicity, considering a single customer record value
public:
    Customer(string n, int i, string p) : RentableUser(n, i, p), rentedCars(0), fineDue(0), customerRecord(0) {}

    void displayDetails() const override {
        User::displayDetails();
        cout << "Role: Customer" << endl;
        cout << "Rented Cars: " << rentedCars << ", Fine Due: " << fineDue << ", Record: " << customerRecord << endl;
    }
};

// Class for employees
class Employee : public RentableUser {
private:
    int rentedCars;
    int rentDue;
    double fineDue;
    double employeeRecord; // For simplicity, considering a single employee record value
public:
    Employee(string n, int i, string p) : RentableUser(n, i, p), rentedCars(0), fineDue(0), employeeRecord(0) {}

    void displayDetails() const override {
        User::displayDetails();
        cout << "Role: Employee" << endl;
        cout << "Rented Cars: " << rentedCars << ", Fine Due: " << fineDue << ", Record: " << employeeRecord << endl;
    }
};

// Class for manager
class Manager : public User {

private:
    CarDb cars;
    CustomerDb customers;
    EmployeeDb employees;

public:
    Manager(string n, int i, string p) : User(n, i, p) {}

    void addCustomer(vector<string> cus) {
        // Code to add a customer
        customers.add(cus);
    }

    void updateCustomer(int id, const vector<string>& cus) {
        // Code to update a customer
        customers.update(id, cus);
    }

    void deleteCustomer(int id) {
        // Code to delete a customer
        customers.deleteRecord(id);
    }

    void addEmployee(vector<string> emp) {
        // Code to add an employee
        employees.add(emp);
    }

    void updateEmployee(int id, const vector<string>& emp) {
        // Code to update an employee
        employees.update(id, emp);
    }

    void deleteEmployee(int id) {
        // Code to delete an employee
        employees.deleteRecord(id);
    }

    void addCar(vector<string> car) {
        // Code to add a car
        cars.add(car);
    }

    void updateCar(int id, const vector<string>& car) {
        // Code to update a car
        cars.update(id, car);
    }

    void deleteCar(int id) {
        // Code to delete a car
        cars.deleteRecord(id);
    }

    void displayAllCars() {
        // Code to display all cars
        cars.display();
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
    string tablename = "cars";
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
    Db::connectToDatabase(&db);

    Manager manager("John Doe", 1, "password123");
    vector<string> car = {"Model X", "2020"};
    manager.addCar(car);
    manager.displayAllCars();

    
    // cars.display(db);
    // customers.display(db);
    // employees.display(db);

    // // Check if database file exists
    // if (access("car_rental.db", F_OK) == 0) {
    //     // Database exists, try opening it
    //     if (!Db::connectToDatabase(&db)) {
    //         return 1;
    //     }
    //     cout << "Database opened successfully." << endl;
    // } else {
    //     // Database doesn't exist, create and open it
    //     if (!Db::connectToDatabase(&db)) {
    //         return 1;
    //     }
    //     cout << "Database created and opened." << endl;
    //     createTable(db);

    //     // Add sample car data (replace with your actual data)
    //     vector<pair<string, string>> cars = {
    //         {"Model A", "2023"},
    //         {"Model B", "2022"}
    //     };
    //     addCars(db, cars);

    //     // Add a sample customer (replace with actual name and password)
    //     addCustomer(db, "John Doe", "password123");
    // }


    // Use the rentCar function
    // int userId = 1; // Assuming customer ID is retrieved after adding the customer
    // Customer customer("John Doe", userId, "password123"); // Initialize customer object
    // customer.rentCar();

    sqlite3_close(db);
    return 0;
}



