
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <cctype>
#include <cmath>

using namespace std;

struct Expense {
    int id;
    string name;
    string category;
    double amount;
};

const string FILE_NAME = "expenses.txt";
vector<Expense> expenses;

// Check that text can safely be stored in our file format.
bool validText(const string& text) {
    return !text.empty() &&
           text.find('|') == string::npos;
}

// Convert text to lowercase for searching.
string toLowerCase(string text) {
    for (char& ch : text) {
        ch = static_cast<char>(
            tolower(static_cast<unsigned char>(ch))
        );
    }
    return text;
}

// Read a whole line, including spaces.
string readText(const string& prompt) {
    string text;

    cout << prompt;
    getline(cin, text);

    return text;
}

// Read a valid positive integer.
int readPositiveInt(const string& prompt) {
    string line;
    int value;
    char extra;

    while (true) {
        cout << prompt;
        getline(cin, line);

        stringstream ss(line);

        if ((ss >> value) && !(ss >> extra) && value > 0) {
            return value;
        }

        cout << "Please enter a positive whole number.\n";
    }
}

// Read a valid positive monetary amount.
double readAmount(const string& prompt) {
    string line;
    double amount;
    char extra;

    while (true) {
        cout << prompt;
        getline(cin, line);

        stringstream ss(line);

        if ((ss >> amount) && !(ss >> extra) &&
            isfinite(amount) && amount > 0 &&
            amount <= 1000000000.0) {
            return amount;
        }

        cout << "Enter a valid amount greater than zero.\n";
    }
}

// Save records to the file.
bool saveExpenses(const vector<Expense>& records) {
    ofstream file(FILE_NAME);

    if (!file.is_open()) {
        cout << "Error: Could not open " << FILE_NAME
             << " for saving.\n";
        return false;
    }

    file << fixed << setprecision(2);

    for (const Expense& e : records) {
        file << e.id << '|'
             << e.name << '|'
             << e.category << '|'
             << e.amount << '\n';

        if (!file) {
            cout << "Error while writing expense records.\n";
            return false;
        }
    }

    file.close();

    if (file.fail()) {
        cout << "Error while closing the expense file.\n";
        return false;
    }

    return true;
}

// Load saved records when the program starts.
void loadExpenses() {
    ifstream file(FILE_NAME);

    // A missing file is normal on the first run.
    if (!file.is_open()) {
        ofstream createFile(FILE_NAME);

        if (!createFile.is_open()) {
            cout << "Warning: Could not create "
                 << FILE_NAME << ".\n";
            cout << "Saving may not work in this folder.\n";
        }

        return;
    }

    string line;
    int maxId = 0;

    while (getline(file, line)) {
        stringstream ss(line);
        string idText, name, category, amountText;
        char extra;

        if (!getline(ss, idText, '|') ||
            !getline(ss, name, '|') ||
            !getline(ss, category, '|') ||
            !getline(ss, amountText) ||
            (ss >> extra)) {
            cout << "Warning: Skipping an invalid record.\n";
            continue;
        }

        try {
            size_t usedId = 0;
            size_t usedAmount = 0;

            int id = stoi(idText, &usedId);
            double amount = stod(amountText, &usedAmount);

            if (usedId != idText.size() ||
                usedAmount != amountText.size() ||
                id <= 0 || !validText(name) ||
                !validText(category) ||
                !isfinite(amount) || amount <= 0) {
                cout << "Warning: Skipping an invalid record.\n";
                continue;
            }

            bool duplicate = false;

            for (const Expense& existing : expenses) {
                if (existing.id == id) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate) {
                cout << "Warning: Duplicate ID skipped.\n";
                continue;
            }

            expenses.push_back({id, name, category, amount});

            if (id > maxId) {
                maxId = id;
            }
        } catch (...) {
            cout << "Warning: Skipping an invalid record.\n";
        }
    }

    file.close();

    cout << expenses.size()
         << " expense record(s) loaded.\n";

    // New IDs will be one greater than the highest loaded ID.
    // The next ID is calculated when adding an expense.
    (void)maxId;
}

// Add a new expense.
void addExpense() {
    Expense e;

    e.id = 1;

    for (const Expense& existing : expenses) {
        if (existing.id >= e.id) {
            e.id = existing.id + 1;
        }
    }

    e.name = readText("Enter expense name: ");
    e.category = readText("Enter category: ");

    if (!validText(e.name) || !validText(e.category)) {
        cout << "Name and category must not be empty "
             << "or contain the | character.\n";
        return;
    }

    e.amount = readAmount("Enter amount: ");

    vector<Expense> updated = expenses;
    updated.push_back(e);

    if (saveExpenses(updated)) {
        expenses = updated;
        cout << "Expense added and saved successfully!\n";
        cout << "Expense ID: " << e.id << '\n';
    }
}

// Display every expense.
void displayExpenses() {
    if (expenses.empty()) {
        cout << "\nNo expenses recorded yet.\n";
        return;
    }

    cout << "\n"
         << left << setw(6) << "ID"
         << setw(25) << "Name"
         << setw(20) << "Category"
         << right << setw(12) << "Amount" << '\n';

    cout << string(63, '-') << '\n';

    cout << fixed << setprecision(2);

    for (const Expense& e : expenses) {
        cout << left << setw(6) << e.id
             << setw(25) << e.name
             << setw(20) << e.category
             << right << setw(12) << e.amount << '\n';
    }
}

// Calculate total spending.
void showTotal() {
    double total = 0;

    for (const Expense& e : expenses) {
        total += e.amount;
    }

    cout << fixed << setprecision(2);
    cout << "\nTotal expenses: " << total << '\n';
    cout << "Number of records: " << expenses.size() << '\n';
}

// Search by expense name or category.
void searchExpenses() {
    string query = readText(
        "Enter an expense name or category to search: "
    );

    if (query.empty()) {
        cout << "Search text cannot be empty.\n";
        return;
    }

    query = toLowerCase(query);
    bool found = false;

    cout << "\nSearch results:\n";
    cout << fixed << setprecision(2);

    for (const Expense& e : expenses) {
        if (toLowerCase(e.name).find(query) != string::npos ||
            toLowerCase(e.category).find(query) != string::npos) {

            cout << "ID: " << e.id
                 << " | Name: " << e.name
                 << " | Category: " << e.category
                 << " | Amount: " << e.amount << '\n';

            found = true;
        }
    }

    if (!found) {
        cout << "No matching expenses found.\n";
    }
}

// Delete an expense by ID.
void deleteExpense() {
    if (expenses.empty()) {
        cout << "There are no expenses to delete.\n";
        return;
    }

    int id = readPositiveInt("Enter the expense ID to delete: ");

    vector<Expense> updated;
    bool found = false;

    for (const Expense& e : expenses) {
        if (e.id == id) {
            found = true;
        } else {
            updated.push_back(e);
        }
    }

    if (!found) {
        cout << "Expense ID not found.\n";
        return;
    }

    cout << "Delete this expense? (y/n): ";
    string answer;
    getline(cin, answer);

    if (answer != "y" && answer != "Y") {
        cout << "Deletion cancelled.\n";
        return;
    }

    if (saveExpenses(updated)) {
        expenses = updated;
        cout << "Expense deleted successfully!\n";
    }
}

int main() {
    loadExpenses();

    string input;
    int choice = 0;

    do {
        cout << "\n========== EXPENSE TRACKER ==========\n";
        cout << "1. Add Expense\n";
        cout << "2. View All Expenses\n";
        cout << "3. Search Expenses\n";
        cout << "4. Show Total Spending\n";
        cout << "5. Delete Expense\n";
        cout << "6. Exit\n";
        cout << "Choose an option: ";

        getline(cin, input);
        stringstream ss(input);
        char extra;

        if (!(ss >> choice) || (ss >> extra)) {
            cout << "Please enter a number from 1 to 6.\n";
            continue;
        }

        switch (choice) {
            case 1:
                addExpense();
                break;

            case 2:
                displayExpenses();
                break;

            case 3:
                searchExpenses();
                break;

            case 4:
                showTotal();
                break;

            case 5:
                deleteExpense();
                break;

            case 6:
                cout << "Your expenses are stored in "
                     << FILE_NAME << ". Goodbye!\n";
                break;

            default:
                cout << "Invalid choice. Choose 1 to 6.\n";
        }

    } while (choice != 6);

    return 0;
}
