#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>

using namespace std;

#define FILENAME "car_rental.db"
#define RENT_DAYS_ALLOWED 7
#define RENT_PER_DAY 100
#define EMPLOYEE_DISCOUNT 0.15

bool askConfirmation(const string &message)
{
    char choice;
    cout << message << " (y/n): ";
    cin >> choice;
    cin.ignore(); // Consume newline character
    return tolower(choice) == 'y';
}

class Db
{
protected:
    string tablename;

    Db()
    {
        sqlite3 *db;
        if (connectToDatabase(&db))
        {
            cout << "Database  connection successful." << endl;
        }
    }

    bool isTableEmpty(sqlite3 *db, string &table_name)
    {

        string sql = "SELECT 1 FROM " + table_name + ";";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            sqlite3_finalize(stmt);
            return true;
        }

        sqlite3_finalize(stmt);
        return false;
    }

    // Function to create the database tables
    void createTable(sqlite3 *db, string sql)
    {
        cout << "Creating table: " << tablename << endl;
        // Execute statement and handle errors
        char *errmsg;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg) != SQLITE_OK)
        {
            cerr << "Error creating " << tablename << " table: " << errmsg << endl;
            sqlite3_free(errmsg);
        }
    }

public:
    // Function to connect to db and check if the database connection is successful
    static bool connectToDatabase(sqlite3 **db)
    {
        int rc = sqlite3_open(FILENAME, db);
        if (rc != SQLITE_OK)
        {
            cerr << "Error opening database: " << sqlite3_errmsg(*db) << endl;
            return false;
        }
        return true;
    }

    void deleteRecord(int id, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }
        if (search(id, db))
        {
            string sql = "DELETE FROM " + tablename + " WHERE id = ?;";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            {
                cerr << "Error preparing statement for deleting: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the value of id to the prepared statement
            sqlite3_bind_int(stmt, 1, id);

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    bool search(int id, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return false;
        }
        string sql = "SELECT EXISTS (SELECT 1 FROM " + tablename + " WHERE id = ?);";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for searching: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        // Bind the value of id to the prepared statement
        sqlite3_bind_int(stmt, 1, id);

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Check if EXISTS returned 1 (first column)
        bool exists = sqlite3_column_int(stmt, 0) == 1;

        sqlite3_finalize(stmt);
        return exists;
    }

    static bool searchTable(int id, string table_name, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return false;
        }
        string sql = "SELECT EXISTS (SELECT 1 FROM " + table_name + " WHERE id = ?);";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for searching: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        // Bind the value of id to the prepared statement
        sqlite3_bind_int(stmt, 1, id);

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Check if EXISTS returned 1 (first column)
        bool exists = sqlite3_column_int(stmt, 0) == 1;

        sqlite3_finalize(stmt);
        return exists;
    }

    static void updateDues(int cusId, int money, int dues, string table)
    {
        sqlite3 *db;
        if (!connectToDatabase(&db))
            return;
        string sql = "UPDATE " + table + " SET money = ?, fineDue = ? WHERE id = ?;";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for updating: " << sqlite3_errmsg(db) << endl;
            return;
        }

        // Bind the values of t to the prepared statement
        sqlite3_bind_int(stmt, 1, money); // Money
        sqlite3_bind_int(stmt, 2, dues);  // Dues
        sqlite3_bind_int(stmt, 3, cusId); // ID

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error updating customers: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
    }
};

class CarDb : public Db
{
private:
    vector<vector<string>> defaultData = {
        {"Lambo Aventador", "2023"},
        {"Ferrari F8", "2022"},
        {"Koenigsegg Agera", "2021"},
        {"Bugatti Veyron", "2020"},
        {"Rolls Royce Spectre", "2019"}};

    void load(sqlite3 *db)
    {
        if (isTableEmpty(db, tablename))
        {
            cout << "Loading cars..." << endl;
            for (vector<string> data : defaultData)
            {
                add(data, db);
            }
        }
    }

public:
    CarDb()
    {
        tablename = "cars";
        sqlite3 *db;
        connectToDatabase(&db);
        string sql = "CREATE TABLE IF NOT EXISTS cars (id INTEGER PRIMARY KEY AUTOINCREMENT, model TEXT NOT NULL, year TEXT, available INTEGER NOT NULL DEFAULT 1, rentedBy INTEGER NOT NULL DEFAULT -1, rentedOn INTEGER NOT NULL DEFAULT -1, FOREIGN KEY(rentedBy) REFERENCES customers(id) ON DELETE SET DEFAULT)";
        // string sql_customers = "CREATE TABLE IF NOT EXISTS customers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, password TEXT NOT NULL, rentedCars INTEGER NOT NULL DEFAULT 0, fineDue DOUBLE NOT NULL DEFAULT 0, customerRecord DOUBLE NOT NULL DEFAULT 0)";
        createTable(db, sql);
        load(db);
    }

    static bool searchRentableCar(int id, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return false;
        }
        string sql = "SELECT EXISTS (SELECT 1 FROM cars WHERE id = ? AND available=1);";

        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for searching: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        // Bind the value of id to the prepared statement
        sqlite3_bind_int(stmt, 1, id);

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            return false;
        }

        // Check if EXISTS returned 1 (first column)
        bool exists = sqlite3_column_int(stmt, 0) == 1;

        sqlite3_finalize(stmt);
        return exists;
    }

    static vector<string> searchCar(int id)
    {
        sqlite3 *db;
        if (!connectToDatabase(&db))
            return {};
        string sql = "SELECT * FROM cars WHERE id = ?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for searching: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        // Bind the value of id to the prepared statement
        sqlite3_bind_int(stmt, 1, id);

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        vector<string> car;
        car.push_back(to_string(sqlite3_column_int(stmt, 0)));
        car.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        car.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
        car.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));
        car.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));
        car.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5)));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return car;
    }

    static void displayCar(int id)
    {
        vector<string> car = searchCar(id);
        cout << "Car Model: " << car[1] << " (" << car[2] << "), "
             << "ID: " << id << endl;
        if (car[3] == "1")
        {
            cout << "Available" << endl;
        }
        else
        {
            cout << "Rented by: " << car[4] << " on Day: " << car[5] << endl;
        }
    }

    void add(const vector<string> car, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }
        string sql = "INSERT INTO cars (model, year) VALUES (?, ?)";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for adding cars: " << sqlite3_errmsg(db) << endl;
            return;
        }

        // Bind the values of t to the prepared statement
        sqlite3_bind_text(stmt, 1, car[0].c_str(), -1, SQLITE_TRANSIENT); // Model
        sqlite3_bind_text(stmt, 2, car[1].c_str(), -1, SQLITE_TRANSIENT); // Year

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error inserting " << tablename << ": " << sqlite3_errmsg(db) << endl;
        }
        cout << "Car " << car[0] << "(" << car[1] << ")"
             << " added successfully." << endl;
        sqlite3_finalize(stmt);
    }

    static void update(int id, const vector<string> &car, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }
        if (searchTable(id, "cars", db))
        {
            string sql = "UPDATE cars SET model = ?, year = ?, available = ?, rentedBy = ?, rentedOn = ? WHERE id = ?;";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            {
                cerr << "Error preparing statement for updating: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the values of car to the prepared statement
            sqlite3_bind_text(stmt, 1, car[0].c_str(), -1, SQLITE_TRANSIENT); // Model
            sqlite3_bind_text(stmt, 2, car[1].c_str(), -1, SQLITE_TRANSIENT); // Year
            sqlite3_bind_text(stmt, 3, car[2].c_str(), -1, SQLITE_TRANSIENT); // Available
            sqlite3_bind_text(stmt, 4, car[3].c_str(), -1, SQLITE_TRANSIENT); // RentedBy
            sqlite3_bind_text(stmt, 5, car[4].c_str(), -1, SQLITE_TRANSIENT); // RentedBy
            sqlite3_bind_int(stmt, 6, id);                                    // ID

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                cerr << "Error updating car: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    static void display(sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }

        string sql = "SELECT * FROM cars WHERE available=1";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cout << "No cars available to rent." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display all available car details
        cout << "Displaying all available cars:" << endl;
        do
        {
            int carId = sqlite3_column_int(stmt, 0);
            string model = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            string year = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            // Additional car details can be displayed if available in the table

            cout << carId << ". " << model << " (" << year << ")" << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
    }

    static void displayAll(sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }

        string sql = "SELECT * FROM cars";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cout << "No cars available." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display all available car details
        cout << "Displaying all cars:" << endl;
        do
        {
            int carId = sqlite3_column_int(stmt, 0);
            string model = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            string year = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            string available = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            string rentedBy = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            string rentedOn = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));

            cout << carId << ". " << model << " (" << year << "), ";
            if (available == "1")
            {
                cout << "Available" << endl;
            }
            else
            {
                cout << "Rented by: " << rentedBy << " on Day: " << rentedOn << endl;
            }
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
    }
};

class CustomerDb : public Db
{
private:
    vector<vector<string>> defaultData = {
        {"Linus", "5000", "0", "0", "5"},
        {"Elon", "50000", "0", "0", "10"},
        {"Steve", "10000", "0", "0", "7"},
        {"Bill", "25000", "0", "0", "8"},
        {"John", "100", "0", "0", "3"}};

    void load(sqlite3 *db)
    {
        if (isTableEmpty(db, tablename))
        {
            cout << "Loading customers..." << endl;
            for (vector<string> data : defaultData)
            {
                add(data, db);
            }
        }
    }

public:
    CustomerDb()
    {
        tablename = "customers";
        sqlite3 *db;
        connectToDatabase(&db);
        string sql = "CREATE TABLE IF NOT EXISTS customers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, money INTEGER NOT NULL DEFAULT 5000, rentedCars INTEGER NOT NULL DEFAULT 0, fineDue INTEGER NOT NULL DEFAULT 0, customerRecord INTEGER NOT NULL DEFAULT 5)";
        createTable(db, sql);
        load(db);
    }

    static vector<string> searchCus(int id)
    {
        sqlite3 *db;
        if (!connectToDatabase(&db))
            return {};
        string sql = "SELECT * FROM customers WHERE id = ?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for searching: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        // Bind the value of id to the prepared statement
        sqlite3_bind_int(stmt, 1, id);

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        vector<string> cus;
        cus.push_back(to_string(sqlite3_column_int(stmt, 0)));
        cus.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        cus.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
        cus.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));
        cus.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));
        cus.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5)));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return cus;
    }

    void add(const vector<string> cus, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }
        string sql = "INSERT INTO customers (name, money, rentedCars, fineDue, customerRecord) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
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
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error inserting " << tablename << ": " << sqlite3_errmsg(db) << endl;
        }
        cout << "Customer " << cus[0] << " added successfully." << endl;
        sqlite3_finalize(stmt);
    }

    void update(int id, const vector<string> &cus, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }
        if (search(id, db))
        {
            string sql = "UPDATE " + tablename + " SET name = ?, money = ?, rentedCars = ?, fineDue = ?, customerRecord = ? WHERE id = ?;";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            {
                cerr << "Error preparing statement for updating: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the values of t to the prepared statement
            sqlite3_bind_text(stmt, 1, cus[0].c_str(), -1, SQLITE_TRANSIENT); // Name
            sqlite3_bind_text(stmt, 2, cus[1].c_str(), -1, SQLITE_TRANSIENT); // Money
            sqlite3_bind_text(stmt, 3, cus[2].c_str(), -1, SQLITE_TRANSIENT); // rentedCars
            sqlite3_bind_text(stmt, 4, cus[3].c_str(), -1, SQLITE_TRANSIENT); // fineDue
            sqlite3_bind_text(stmt, 5, cus[4].c_str(), -1, SQLITE_TRANSIENT); // record
            sqlite3_bind_int(stmt, 6, id);                                    // ID

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                cerr << "Error updating customers: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    static void display(sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }

        string sql = "SELECT * FROM customers";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cout << "No customers." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display all available car details
        cout << "Displaying all customers:" << endl;
        do
        {
            int cusId = sqlite3_column_int(stmt, 0);
            string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            string money = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            string rentedCars = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            string fineDue = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            string record = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
            // Additional car details can be displayed if available in the table

            cout << cusId << ". " << name << ", $" << money << ", " << rentedCars << " cars rented, Fine Due: $" << fineDue << ", Customer Record: " << record << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
    }

    static void displayCustomer(int id)
    {
        vector<string> cus = searchCus(id);
        cout << "Customer Name: " << cus[1] << ", ID: " << id << endl;
        cout << "Money: " << cus[2] << " Rented Cars: " << cus[3] << endl;
        cout << "Fine Due: " << cus[4] << ", Customer Record: " << cus[5] << endl;
    }
};

class EmployeeDb : public Db
{
private:
    vector<vector<string>> defaultData = {
        {"Emp1", "500", "0", "0", "7"},
        {"Emp2", "5000", "0", "0", "5"},
        {"Emp3", "1000", "0", "0", "6"},
        {"Emp4", "2500", "0", "0", "8"},
        {"Emp5", "10", "0", "0", "3"}};

    void load(sqlite3 *db)
    {
        if (isTableEmpty(db, tablename))
        {
            cout << "Loading employees..." << endl;
            for (vector<string> data : defaultData)
            {
                add(data, db);
            }
        }
    }

public:
    EmployeeDb()
    {
        tablename = "employees";
        sqlite3 *db;
        connectToDatabase(&db);
        string sql = "CREATE TABLE IF NOT EXISTS employees (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, money INTEGER NOT NULL DEFAULT 500, rentedCars INTEGER NOT NULL DEFAULT 0, fineDue INTEGER NOT NULL DEFAULT 0, employeeRecord INTEGER NOT NULL DEFAULT 7)";
        createTable(db, sql);
        load(db);
    }

    static vector<string> searchEmp(int i)
    {
        sqlite3 *db;
        if (!connectToDatabase(&db))
            return {};
        string sql = "SELECT * FROM employees WHERE id = ?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement for searching: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        // Bind the value of id to the prepared statement
        sqlite3_bind_int(stmt, 1, i);

        // Execute the statement
        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cerr << "Error executing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        vector<string> emp;
        emp.push_back(to_string(sqlite3_column_int(stmt, 0)));
        emp.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
        emp.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
        emp.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));
        emp.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));
        emp.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5)));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return emp;
    }

    void add(const vector<string> emp, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }
        string sql = "INSERT INTO employees (name, money, rentedCars, fineDue, employeeRecord) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
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
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error inserting " << tablename << ": " << sqlite3_errmsg(db) << endl;
        }
        cout << "Employee " << emp[0] << " added successfully." << endl;
        sqlite3_finalize(stmt);
    }

    void update(int id, const vector<string> &emp, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }
        if (search(id, db))
        {
            string sql = "UPDATE " + tablename + " SET name = ?, money = ?, rentedCars = ?, fineDue = ?, employeeRecord = ? WHERE id = ?;";

            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            {
                cerr << "Error preparing statement for updating: " << sqlite3_errmsg(db) << endl;
                return;
            }

            // Bind the values of t to the prepared statement
            sqlite3_bind_text(stmt, 1, emp[0].c_str(), -1, SQLITE_TRANSIENT); // Name
            sqlite3_bind_text(stmt, 2, emp[1].c_str(), -1, SQLITE_TRANSIENT); // Money
            sqlite3_bind_text(stmt, 3, emp[2].c_str(), -1, SQLITE_TRANSIENT); // rentedCars
            sqlite3_bind_text(stmt, 4, emp[3].c_str(), -1, SQLITE_TRANSIENT); // fineDue
            sqlite3_bind_text(stmt, 5, emp[4].c_str(), -1, SQLITE_TRANSIENT); // record
            sqlite3_bind_int(stmt, 6, id);                                    // ID

            // Execute the statement
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                cerr << "Error updating employees: " << sqlite3_errmsg(db) << endl;
            }

            sqlite3_finalize(stmt);
        }
    }

    static void display(sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!connectToDatabase(&db))
                return;
        }

        string sql = "SELECT * FROM employees";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cout << "No employees." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }

        // Display all available car details
        cout << "Displaying all employees" << endl;
        do
        {
            int empId = sqlite3_column_int(stmt, 0);
            string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            string money = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            string rentedCars = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            string fineDue = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            string record = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
            // Additional car details can be displayed if available in the table

            cout << empId << ". " << name << ", $" << money << ", " << rentedCars << " cars rented, Fine Due: $" << fineDue << ", Employee Record: " << record << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
    }

    static void displayEmployee(int id)
    {
        vector<string> cus = searchEmp(id);
        cout << "Employee Name: " << cus[1] << ", ID: " << id << endl;
        cout << "Money: " << cus[2] << " Rented Cars: " << cus[3] << endl;
        cout << "Fine Due: " << cus[4] << ", Employee Record: " << cus[5] << endl;
        cout << "Employee Discount: " << EMPLOYEE_DISCOUNT * 100 << "%" << endl;
    }
};

// Base class for users
class User
{
protected:
    string name;
    int id;
    string password;

    User(int i) : id(i) {}

    User(string n, int i, string p) : name(n), id(i), password(p) {}

    virtual void displayDetails() const
    {
        cout << "Name: " << name << ", ID: " << id << endl;
    }
};

// Class for cars
class Car
{
private:
    string model;
    string condition;
    string tablename = "cars";

public:
    Car(string m, string c) : model(m), condition(c) {}

    static bool rent(int cusId, int carId, int date, string table, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!Db::connectToDatabase(&db))
                return false;
        }

        // Update car availability and user rented cars
        sqlite3_stmt *stmt;
        string sql = "UPDATE cars SET available=available-1, rentedBy=?, rentedOn=? WHERE id=?";
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, cusId);
        sqlite3_bind_int(stmt, 2, date);
        sqlite3_bind_int(stmt, 3, carId);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error updating car availability: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }

        sql = "UPDATE " + table + " SET rentedCars=rentedCars+1 WHERE id=?";
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, cusId);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error updating " + table + " rented cars: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }
        sqlite3_finalize(stmt);
        cout << "Car rented successfully." << endl;
        return true;
    }

    static vector<int> checkRents(int cusId, string table, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!Db::connectToDatabase(&db))
                return {};
        }
        // Check if customer has rented cars
        string sql = "SELECT * FROM cars WHERE rentedBy=?";
        sqlite3_stmt *stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        sqlite3_bind_int(stmt, 1, cusId);

        if (sqlite3_step(stmt) != SQLITE_ROW)
        {
            cout << "You haven't rented any cars." << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return {};
        }

        vector<int> rentedCars;

        // Display all rented car details
        cout << "Your rented cars:" << endl;
        do
        {
            int car_id = sqlite3_column_int(stmt, 0);
            string model = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            string year = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));

            cout << car_id << ". " << model << " (" << year << ")" << endl;
            rentedCars.push_back(car_id);

        } while (sqlite3_step(stmt) == SQLITE_ROW);

        sqlite3_finalize(stmt);
        return rentedCars;
    }

    static bool returnCar(int cusId, int carId, int date, string table, int daysAllowed, int rentPerDay, double employeeDiscount, sqlite3 *db = nullptr)
    {
        if (db == nullptr)
        {
            if (!Db::connectToDatabase(&db))
                return false;
        }

        sqlite3_stmt *stmt;

        vector<string> car = CarDb::searchCar(carId);
        int rentedOn = stoi(car[5]);
        int rentDays = date - rentedOn;
        int fine = rentDays * rentPerDay;
        if (table == "employees")
        {
            fine = (1 - employeeDiscount) * fine;
        }
        if (rentDays > daysAllowed)
        {
            cout << "You have exceeded the allowed rental period. A fine of $10 per day will be added to your account." << endl;
            fine += 10 * (rentDays - daysAllowed);

            string sql = "UPDATE " + table + " SET customerRecord=customerRecord-2 WHERE id=?";
            sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, cusId);
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                cerr << "Error updating " + table + " customer record: " << sqlite3_errmsg(db) << endl;
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return false;
            }
            sqlite3_finalize(stmt);
        }

        // Update car availability and user rented cars

        string sql = "UPDATE cars SET available=available+1, rentedBy=-1 WHERE id=?";
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, carId);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error updating car availability: " << sqlite3_errmsg(db) << endl;
            return false;
        }
        sql = "UPDATE " + table + " SET rentedCars=rentedCars-1, fineDue=fineDue+? WHERE id=?";
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, fine);
        sqlite3_bind_int(stmt, 2, cusId);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error updating " + table + " rented cars: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        sqlite3_finalize(stmt);
        cout << "Car returned successfully." << endl;
        return true;
    }

    void displayDetails() const
    {
        cout << "Model: " << model << ", Condition: " << condition << endl;
    }
};

// Class for manager
class Manager : public User
{

private:
    CarDb cars;
    CustomerDb customers;
    EmployeeDb employees;

public:
    Manager(string n, int i, string p) : User(n, i, p) {}

    void addCustomer(vector<string> cus)
    {
        // Code to add a customer
        customers.add(cus);
    }

    void updateCustomer(int id, const vector<string> &cus)
    {
        // Code to update a customer
        customers.update(id, cus);
    }

    void deleteCustomer(int id)
    {
        // Code to delete a customer
        customers.deleteRecord(id);
    }

    void addEmployee(vector<string> emp)
    {
        // Code to add an employee
        employees.add(emp);
    }

    void updateEmployee(int id, const vector<string> &emp)
    {
        // Code to update an employee
        employees.update(id, emp);
    }

    void deleteEmployee(int id)
    {
        // Code to delete an employee
        employees.deleteRecord(id);
    }

    void addCar(vector<string> car)
    {
        // Code to add a car
        cars.add(car);
    }

    void updateCar(int id, const vector<string> &car)
    {
        // Code to update a car
        cars.update(id, car);
    }

    void deleteCar(int id)
    {
        // Code to delete a car
        cars.deleteRecord(id);
    }

    static bool rent(int cusId, int carId, string table, sqlite3 *db = nullptr)
    {
        int date;
        cout << "Enter today's date (int)" << endl;
        cin >> date;
        return Car::rent(cusId, carId, date, table, db);
    }

    static vector<int> checkRents(int cusId, string table, sqlite3 *db = nullptr)
    {
        return Car::checkRents(cusId, table, db);
    }

    static bool returnCar(int cusId, int carId, string table, sqlite3 *db = nullptr)
    {
        int date;
        cout << "Enter today's date (int)" << endl;
        cin >> date;
        return Car::returnCar(cusId, carId, date, table, RENT_DAYS_ALLOWED, RENT_PER_DAY, EMPLOYEE_DISCOUNT, db);
    }

    static void updateDues(int cusId, int money, int dues, string table)
    {
        Db::updateDues(cusId, money, dues, table);
        return;
    }

    void displayAvailableCars()
    {
        // Code to display all cars
        cars.display();
    }

    void displayAllCars()
    {
        // Code to display all cars
        cars.displayAll();
    }

    void displayDetails() const override
    {
        User::displayDetails();
        cout << "Role: Manager" << endl;
    }
};

class RentableUser : public User
{

protected:
    string table;

    RentableUser(int i, string table) : User(i)
    {
        this->table = table;
    }

public:
    void rentCar()
    {
        // Code to rent a car
        sqlite3 *db;
        if (!Db::connectToDatabase(&db))
            return;

        CarDb::display(db);

        int carId;
        cout << "Enter the ID of the car you want to rent: ";
        cin >> carId;

        // Validate user input
        bool foundCar = CarDb::searchRentableCar(carId, db);

        if (!foundCar)
        {
            cout << "Invalid car ID. Please choose from the list above." << endl;
            sqlite3_close(db);
            return;
        }

        // Ask for confirmation and rent car
        if (askConfirmation("Do you want to rent this car?"))
        {
            Manager::rent(id, carId, table, db);
        }

        sqlite3_close(db);
    }

    void returnCar()
    {
        // Code to return a car
        sqlite3 *db;
        if (!Db::connectToDatabase(&db))
            return;

        // Check and display rented cars
        vector<int> rentedCars = Manager::checkRents(id, table, db);
        if (rentedCars.size() == 0)
        {
            cout << "You haven't rented any cars." << endl;
            return;
        }

        // Get user input for car ID
        int chosenId;
        cout << "Enter the ID of the car you want to return: ";
        cin >> chosenId;

        // Validate user input
        bool foundCar = false;
        for (int carId : rentedCars)
        {
            if (carId == chosenId)
            {
                foundCar = true;
                break;
            }
        }

        if (!foundCar)
        {
            cout << "Invalid car ID. Please choose from the list above." << endl;
            sqlite3_close(db);
            return;
        }

        // Ask for confirmation and return car
        if (askConfirmation("Do you want to return this car?"))
        {
            Manager::returnCar(id, chosenId, table, db);
        }

        sqlite3_close(db);
    }

    // void clear_dues(int money)
    // {
    //     // Code to clear dues
    //     sqlite3 *db;
    //     if (!Db::connectToDatabase(&db))
    //         return;

    //     // Check if customer has any dues
    //     string sql = "SELECT * FROM " + table + " WHERE id=? AND (fineDue>0)"; // Adjust condition based on your criteria for having dues
    //     sqlite3_stmt *stmt;
    //     if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    //     {
    //         cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
    //         sqlite3_finalize(stmt);
    //         sqlite3_close(db);
    //         return;
    //     }

    //     sqlite3_bind_int(stmt, 1, id);

    //     if (sqlite3_step(stmt) != SQLITE_ROW)
    //     {
    //         cout << "You don't have any outstanding dues." << endl;
    //         sqlite3_finalize(stmt);
    //         sqlite3_close(db);
    //         return;
    //     }

    //     // Display current dues and ask for confirmation
    //     double dues = sqlite3_column_double(stmt, 1);   // Assuming fineDue is stored in the 2nd column of the customer table
    //     double record = sqlite3_column_double(stmt, 2); // Assuming customerRecord is stored in the 3rd column
    //     cout << "Your current dues:" << endl;
    //     cout << "- Dues: $" << dues << endl;
    //     cout << "- Customer Record: " << record << endl;

    //     if (!askConfirmation("Do you want to clear your dues?"))
    //     {
    //         sqlite3_finalize(stmt);
    //         sqlite3_close(db);
    //         return;
    //     }

    //     // Update customer record (assuming it can be improved through payment)
    //     sql = "UPDATE customers SET customerRecord=1 WHERE id=?"; // Adjust update logic based on your criteria for improving customer record
    //     sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    //     sqlite3_bind_int(stmt, 1, id);
    //     if (sqlite3_step(stmt) != SQLITE_DONE)
    //     {
    //         cerr << "Error updating customer record: " << sqlite3_errmsg(db) << endl;
    //     }

    //     // Reset fine due
    //     sql = "UPDATE customers SET fineDue=0 WHERE id=?";
    //     sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    //     sqlite3_bind_int(stmt, 1, id);
    //     if (sqlite3_step(stmt) != SQLITE_DONE)
    //     {
    //         cerr << "Error resetting fine due: " << sqlite3_errmsg(db) << endl;
    //     }

    //     cout << "Dues cleared successfully." << endl;

    //     sqlite3_finalize(stmt);
    //     sqlite3_close(db);
    // }

    void browseRentedCars()
    {
        // Code to browse available cars
        Car::checkRents(id, table);
    }
};

// Class for customers
class Customer : public RentableUser
{
public:
    Customer(int id) : RentableUser(id, "customers")
    {
        vector<string> cus = CustomerDb::searchCus(id);
        name = cus[1];
    }

    void clear_dues()
    {
        vector<string> cus = CustomerDb::searchCus(id);

        int money = stoi(cus[2]);
        int dues = stoi(cus[4]);

        if (dues == 0)
        {
            cout << "You don't have any outstanding dues." << endl;
            return;
        }

        if (money >= dues)
        {
            money -= dues;
            dues = 0;
            cout << "Dues cleared successfully." << endl;
        }
        else
        {
            cout << "ALERT!!! You don't have enough money to clear your dues." << endl;
            cout << "Please add money to your account." << endl;
            cout << "Cleared " << money << " of your dues. " << dues - money << " is still pending." << endl;
            dues -= money;
            money = 0;
        }
        Manager::updateDues(id, money, dues, "customers");
        return;
    }

    void displayDetails() const override
    {
        vector<string> cus = CustomerDb::searchCus(id);
        if (cus.size() == 0)
        {
            cout << "Invalid Customer ID" << endl;
            exit(1);
        }
        User::displayDetails();
        cout << "Role: Customer" << endl;
        cout << "Money: " << cus[2] << " Rented Cars: " << cus[3] << ", Fine Due: " << cus[4] << ", Customer Record: " << cus[5] << endl;
    }
};

// Class for employees
class Employee : public RentableUser
{
public:
    Employee(int id) : RentableUser(id, "employees") {}

    void clear_dues()
    {
        vector<string> emp = EmployeeDb::searchEmp(id);

        int money = stoi(emp[2]);
        int dues = stoi(emp[4]);

        if (dues == 0)
        {
            cout << "You don't have any outstanding dues." << endl;
            return;
        }

        if (money >= dues)
        {
            money -= dues;
            dues = 0;
            cout << "Dues cleared successfully." << endl;
        }
        else
        {
            cout << "ALERT!!! You don't have enough money to clear your dues." << endl;
            cout << "Please add money to your account." << endl;
            cout << "Cleared " << money << " of your dues. " + dues - money << " is still pending." << endl;
            dues -= money;
            money = 0;
        }
        Manager::updateDues(id, money, dues, "employees");
        return;
    }

    void displayDetails() const override
    {
        vector<string> emp = EmployeeDb::searchEmp(id);
        if (emp.size() == 0)
        {
            cout << "Invalid Employee ID" << endl;
            exit(1);
        }
        User::displayDetails();
        cout << "Role: Employee" << endl;
        cout << "Money: " << emp[2] << ", Rented Cars: " << emp[3] << ", Fine Due: " << emp[4] << ", Employee Record: " << emp[5] << endl;
    }
};

int main()
{

    sqlite3 *db;
    Db::connectToDatabase(&db);

    Manager manager("John Doe", 1, "password123");

    cout << "Enter your role (1/2/3): 1. Manager, 2. Customer, 3. Employee" << endl;
    int role;
    cin >> role;
    vector<string> cus;

    int id;
    string command;

    if (role == 1)
    {
        cout << "Welcome Manager" << endl;
        while (true)
        {
            cout << "Enter a command: (Type 'help' for commands list)" << endl;
            cin >> command;
            if (command == "addCustomer")
            {
                string name;
                int money;
                int dues;
                int record;

                cout << "Enter customer name: ";
                cin >> name;

                cout << "Enter customer money: ";
                cin >> money;

                cout << "Enter customer dues: ";
                cin >> dues;

                cout << "Enter customer record: ";
                cin >> record;

                vector<string> cus = {name, to_string(money), "0", to_string(dues), to_string(record)};
                manager.addCustomer(cus);
            }
            else if (command == "updateCustomer")
            {
                int newId;
                cout << "Enter the ID of the customer you want to update: ";
                cin >> newId;
                vector<string> cus = CustomerDb::searchCus(newId);
                if (cus.size() == 0)
                {
                    cout << "Invalid Customer ID" << endl;
                    exit(1);
                }
                CustomerDb::displayCustomer(newId);

                cout << "Enter new customer name: (Previously: " << cus[1] << ")";
                cin >> cus[1];

                cout << "Enter new customer money: (Previously: " << cus[2] << ")";
                cin >> cus[2];

                cout << "Enter new customer rentedCars: (Previously: " << cus[3] << ")";
                cin >> cus[3];

                cout << "Enter new customer dues: (Previously: " << cus[4] << ")";
                cin >> cus[4];

                cout << "Enter new customer record: (Previously: " << cus[5] << ")";
                cin >> cus[5];

                manager.updateCustomer(1, cus);
            }
            else if (command == "deleteCustomer")
            {
                int newId;
                cout << "Enter the ID of the customer you want to delete: ";
                cin >> newId;
                manager.deleteCustomer(newId);
            }
            else if (command == "addEmployee")
            {
                string name;
                int money;
                int dues;
                int record;

                cout << "Enter employee name: ";
                cin >> name;

                cout << "Enter employee money: ";
                cin >> money;

                cout << "Enter employee dues: ";
                cin >> dues;

                cout << "Enter employee record: ";
                cin >> record;

                vector<string> cus = {name, to_string(money), "0", to_string(dues), to_string(record)};
                manager.addEmployee(cus);
            }
            else if (command == "updateEmployee")
            {
                int newId;
                cout << "Enter the ID of the employee you want to update: ";
                cin >> newId;
                vector<string> cus = EmployeeDb::searchEmp(newId);
                if (cus.size() == 0)
                {
                    cout << "Invalid Employee ID" << endl;
                    exit(1);
                }
                EmployeeDb::displayEmployee(newId);

                cout << "Enter new employee name: (Previously: " << cus[1] << ")";
                cin >> cus[1];

                cout << "Enter new employee money: (Previously: " << cus[2] << ")";
                cin >> cus[2];

                cout << "Enter new employee rentedCars: (Previously: " << cus[3] << ")";
                cin >> cus[3];

                cout << "Enter new employee dues: (Previously: " << cus[4] << ")";
                cin >> cus[4];

                cout << "Enter new employee record: (Previously: " << cus[5] << ")";
                cin >> cus[5];

                manager.updateEmployee(newId, cus);
            }
            else if (command == "deleteEmployee")
            {
                int newId;
                cout << "Enter the ID of the employee you want to delete: ";
                cin >> newId;
                manager.deleteEmployee(newId);
            }
            else if (command == "addCar")
            {
                string model;
                string year;

                cout << "Enter car model: ";
                cin >> model;
                cout << "Enter car year: ";
                cin >> year;

                vector<string> car = {model, year, "1", "0", "0"};
                manager.addCar(car);
            }
            else if (command == "updateCar")
            {
                int newId;
                cout << "Enter the ID of the car you want to update: ";
                cin >> newId;
                vector<string> car = CarDb::searchCar(newId);
                if (car.size() == 0)
                {
                    cout << "Invalid Car ID" << endl;
                    exit(1);
                }
                CarDb::displayCar(newId);

                cout << "Enter new car model: (Previously: " << car[1] << ")";
                cin >> car[1];
                cout << "Enter new car year: (Previously: " << car[2] << ")";
                cin >> car[2];
                cout << "Enter new car available: (Previously: " << car[3] << ")";
                cin >> car[3];
                cout << "Enter new car rentedBy: (Previously: " << car[4] << ")";
                cin >> car[4];
                cout << "Enter new car rentedOn: (Previously: " << car[5] << ")";
                cin >> car[5];

                manager.updateCar(newId, car);
            }
            else if (command == "deleteCar")
            {
                int newId;
                cout << "Enter the ID of the car you want to delete: ";
                cin >> newId;
                manager.deleteCar(newId);
            }
            else if (command == "displayAvailableCars")
            {
                manager.displayAvailableCars();
            }
            else if (command == "displayAllCars")
            {
                manager.displayAllCars();
            }
            else if (command == "help")
            {
                cout << endl;
                cout << "Commands: " << endl;
                cout << "-----------------" << endl;
                cout << "addCustomer: Add a customer." << endl;
                cout << "updateCustomer: Update a customer." << endl;
                cout << "deleteCustomer: Delete a customer." << endl;
                cout << "addEmployee: Add an employee." << endl;
                cout << "updateEmployee: Update an employee." << endl;
                cout << "deleteEmployee: Delete an employee." << endl;
                cout << "addCar: Add a car." << endl;
                cout << "updateCar: Update a car." << endl;
                cout << "deleteCar: Delete a car." << endl;
                cout << "exit: Exit the program." << endl;
            }
            else if (command == "exit")
            {
                break;
            }
            else
            {
                std::cout << "Invalid command." << std::endl;
            }
        }
    }
    else if (role == 2)
    {
        cout << "Welcome Customer" << endl;
        cout << "Enter your ID: ";
        id;
        cin >> id;
        cus = CustomerDb::searchCus(id);
        if (cus.size() == 0)
        {
            cout << "Invalid Customer ID" << endl;
            exit(1);
        }
        Customer customer(id);
        while (true)
        {
            std::cout << "Enter a command: (Type 'help' for commands list)" << endl;
            std::cin >> command;

            if (command == "myDetails")
            {
                CustomerDb::displayCustomer(id);
            }
            else if (command == "rentCar")
            {
                customer.rentCar();
            }
            else if (command == "returnCar")
            {
                customer.returnCar();
            }
            else if (command == "clearDues")
            {
                customer.clear_dues();
            }
            else if (command == "displayAvailableCars")
            {
                manager.displayAvailableCars();
            }
            else if (command == "currentlyRentedCars")
            {
                customer.browseRentedCars();
            }
            else if (command == "help")
            {
                cout << endl;
                cout << "Commands: " << endl;
                cout << "-----------------" << endl;
                cout << "myDetails: Display your details." << endl;
                cout << "rentCar: Rent a car." << endl;
                cout << "returnCar: Return a car." << endl;
                cout << "clearDues: Clear your dues." << endl;
                cout << "displayAvailableCars: Display available cars." << endl;
                cout << "currentlyRentedCars: Display currently rented cars." << endl;
                cout << "exit: Exit the program." << endl;
            }
            else if (command == "exit")
            {
                break;
            }
            else
            {
                std::cout << "Invalid command." << std::endl;
            }
        }
    }
    else if (role == 3)
    {
        cout << "Welcome Employee" << endl;
        cout << "Enter your ID: ";
        id;
        cin >> id;
        cus = EmployeeDb::searchEmp(id);
        if (cus.size() == 0)
        {
            cout << "Invalid Employee ID" << endl;
            exit(1);
        }
        Employee employee(id);
        while (true)
        {
            std::cout << "Enter a command: (Type 'help' for commands list)" << endl;
            std::cin >> command;

            if (command == "myDetails")
            {
                EmployeeDb::displayEmployee(id);
            }
            else if (command == "rentCar")
            {
                employee.rentCar();
            }
            else if (command == "returnCar")
            {
                employee.returnCar();
            }
            else if (command == "clearDues")
            {
                employee.clear_dues();
            }
            else if (command == "displayAvailableCars")
            {
                manager.displayAvailableCars();
            }
            else if (command == "currentlyRentedCars")
            {
                employee.browseRentedCars();
            }
            else if (command == "help")
            {
                cout << endl;
                cout << "Commands: " << endl;
                cout << "-----------------" << endl;
                cout << "myDetails: Display your details." << endl;
                cout << "rentCar: Rent a car." << endl;
                cout << "returnCar: Return a car." << endl;
                cout << "clearDues: Clear your dues." << endl;
                cout << "displayAvailableCars: Display available cars." << endl;
                cout << "currentlyRentedCars: Display currently rented cars." << endl;
                cout << "exit: Exit the program." << endl;
            }
            else if (command == "exit")
            {
                break;
            }
            else
            {
                std::cout << "Invalid command." << std::endl;
            }
        }
    }
    else
    {
        cout << "Invalid role." << endl;
    }

    sqlite3_close(db);
    return 0;
}