#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <sqlite3.h>

using namespace std;

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


int main() {
    // // Creating users
    // Customer customer1("John", 1, "password1");
    // Employee employee1("Alice", 2, "password2");
    // Manager manager("Bob", 3, "password3");

    // // Creating cars
    // vector<Car> cars;
    // cars.push_back(Car("Toyota", "Good"));
    // cars.push_back(Car("Honda", "Fair"));
    // cars.push_back(Car("Ford", "Excellent"));
    // cars.push_back(Car("Chevrolet", "Fair"));
    // cars.push_back(Car("BMW", "Good"));

    // // Displaying details
    // customer1.displayDetails();
    // employee1.displayDetails();
    // manager.displayDetails();

    // cout << "Available Cars:" << endl;
    // for (const auto& car : cars) {
    //     car.displayDetails();
    // }

    sqlite3* db;
    sqlite3_open("test.db", &db);
    sqlite3_close(db);

    return 0;
}

