#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <limits>
using namespace std;

struct Transaction {
    string type;
    double amount;
    string time;
};

struct Account {
    string username;
    int pin;
    double balance;
    vector<Transaction> history;
    double withdrawnToday = 0;
    bool locked = false;
};

vector<Account> accounts;
const double DAILY_LIMIT = 20000.0;

string currentTime() {
    time_t now = time(0);
    string t = ctime(&now);
    t.pop_back(); // remove newline
    return t;
}

void saveAccounts() {
    ofstream file("accounts.txt");
    for (auto &acc : accounts) {
        file << acc.username << '\n'
             << acc.pin << ' ' << acc.balance << ' ' << acc.withdrawnToday << ' ' << acc.locked << '\n';
        file << acc.history.size() << '\n';
        for (auto &t : acc.history)
            file << t.type << ' ' << t.amount << ' ' << t.time << '\n';
    }
    file.close();
}

void loadAccounts() {
    ifstream file("accounts.txt");
    if (!file) return;
    accounts.clear();

    while (true) {
        Account acc;
        int historySize;
        if (!getline(file, acc.username)) break; // username line
        if (!(file >> acc.pin >> acc.balance >> acc.withdrawnToday >> acc.locked)) break;
        file >> historySize;
        file.ignore();
        acc.history.clear();
        for (int i = 0; i < historySize; i++) {
            Transaction t;
            file >> t.type >> t.amount;
            getline(file, t.time);
            if (!t.time.empty() && t.time[0] == ' ') t.time.erase(0, 1);
            acc.history.push_back(t);
        }
        accounts.push_back(acc);
        file.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    file.close();
}

void createAccount() {
    Account acc;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter username: ";
    getline(cin, acc.username);

    cout << "Enter PIN: ";
    cin >> acc.pin;

    cout << "Enter initial balance: ";
    cin >> acc.balance;

    acc.withdrawnToday = 0;
    acc.locked = false;

    accounts.push_back(acc);
    saveAccounts();
    cout << "Account created successfully!\n";
}

void deleteAccount() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string user;
    int pin;
    cout << "Enter username to delete: ";
    getline(cin, user);
    cout << "Enter PIN: ";
    cin >> pin;

    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].username == user && accounts[i].pin == pin) {
            accounts.erase(accounts.begin() + i);
            saveAccounts();
            cout << "Account deleted successfully!\n";
            return;
        }
    }
    cout << "Account not found or incorrect PIN!\n";
}

void showHistory(Account &acc) {
    cout << "\nLast 5 Transactions:\n";
    int start = acc.history.size() > 5 ? acc.history.size() - 5 : 0;
    for (size_t i = start; i < acc.history.size(); i++) {
        cout << acc.history[i].type << " - Rs." << acc.history[i].amount
             << " on " << acc.history[i].time << endl;
    }
}

void deposit(Account &acc) {
    double amt;
    cout << "Enter deposit amount: ";
    cin >> amt;
    acc.balance += amt;
    acc.history.push_back({"Deposit", amt, currentTime()});
    saveAccounts();
    cout << "Deposit successful! New balance: Rs." << acc.balance << endl;
}

void withdraw(Account &acc) {
    double amt;
    cout << "Enter withdrawal amount: ";
    cin >> amt;
    if (amt > acc.balance) {
        cout << "Insufficient funds!\n";
        return;
    }
    if (acc.withdrawnToday + amt > DAILY_LIMIT) {
        cout << "Daily withdrawal limit exceeded!\n";
        return;
    }
    acc.balance -= amt;
    acc.withdrawnToday += amt;
    acc.history.push_back({"Withdraw", amt, currentTime()});
    saveAccounts();
    cout << "Withdrawal successful! Remaining balance: Rs." << acc.balance << endl;
}

void checkBalance(Account &acc) {
    cout << "\nCurrent Balance: Rs." << acc.balance << endl;
    showHistory(acc);
}

void transfer(Account &acc) {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string user;
    cout << "Enter recipient username: ";
    getline(cin, user);

    for (auto &r : accounts) {
        if (r.username == user) {
            double amt;
            cout << "Enter amount to transfer: ";
            cin >> amt;
            if (amt > acc.balance) {
                cout << "Insufficient funds!\n";
                return;
            }
            acc.balance -= amt;
            r.balance += amt;
            acc.history.push_back({"TransferOut", amt, currentTime()});
            r.history.push_back({"TransferIn", amt, currentTime()});
            saveAccounts();
            cout << "Transfer successful!\n";
            return;
        }
    }
    cout << "Recipient not found!\n";
}

void accountMenu(Account &acc) {
    int choice;
    do {
        cout << "\nWelcome, " << acc.username << "\n";
        cout << "1. Check Balance\n2. Deposit\n3. Withdraw\n4. Transfer\n5. Logout\nEnter choice: ";
        cin >> choice;
        switch (choice) {
            case 1: checkBalance(acc); break;
            case 2: deposit(acc); break;
            case 3: withdraw(acc); break;
            case 4: transfer(acc); break;
            case 5: cout << "Logging out...\n"; break;
            default: cout << "Invalid choice!\n";
        }
    } while (choice != 5);
}

void login() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string user;
    int pin;
    cout << "Enter username: ";
    getline(cin, user);
    int attempts = 0;
    for (auto &acc : accounts) {
        if (acc.username == user) {
            if (acc.locked) {
                cout << "Account is locked for this session.\n";
                return;
            }
            while (attempts < 3) {
                cout << "Enter PIN: ";
                cin >> pin;
                if (pin == acc.pin) {
                    cout << "Login successful!\n";
                    accountMenu(acc);
                    return;
                } else {
                    attempts++;
                    cout << "Incorrect PIN! Attempts left: " << 3 - attempts << endl;
                }
            }
            acc.locked = true;
            cout << "Too many incorrect attempts. Account locked for session.\n";
            return;
        }
    }
    cout << "Username not found!\n";
}

int main() {
    loadAccounts();
    int choice;
    while (true) {
        cout << "\n==== ATM Simulator ====" << endl;
        cout << "1. Create Account\n2. Delete Account\n3. Login\n4. Exit\nEnter choice: ";
        cin >> choice;
        switch (choice) {
            case 1: createAccount(); break;
            case 2: deleteAccount(); break;
            case 3: login(); break;
            case 4: saveAccounts(); cout << "Goodbye!\n"; return 0;
            default: cout << "Invalid choice!\n";
        }
    }
}
