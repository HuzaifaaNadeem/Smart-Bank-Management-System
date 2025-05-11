#include <iostream>
#include <fstream>
#include <string>
using namespace std;

string encrypt(string input) {
    string encrypted = input;
    char key = 'K';
    for (char& c : encrypted) {
        c ^= key;
    }
    return encrypted;
}

class User {
public:
    string name;
    string CNIC;
    string address;
    string passwordEncrypted;
    int userID;
    static int userCount;

    User() {
        name = CNIC = address = passwordEncrypted = "";
        userID = ++userCount;
    }

    void inputDetails() {
        cin.ignore();
        cout << "Enter Name: ";
        getline(cin, name);
        cout << "Enter CNIC: ";
        getline(cin, CNIC);
        cout << "Enter Address: ";
        getline(cin, address);

        string pass;
        while (true) {
            cout << "Set 4-digit numeric Password: ";
            getline(cin, pass);

            bool isValid = pass.length() == 4;
            for (char ch : pass) {
                if (ch < '0' || ch > '9') {
                    isValid = false;
                    break;
                }
            }

            if (isValid) break;
            else cout << "Invalid password. Please enter exactly 4 digits.\n";
        }

        passwordEncrypted = encrypt(pass);
    }

    bool login() {
        int attempts = 0;
        string inputPass;

        while (attempts < 3) {
            cout << "Enter Password: ";
            getline(cin, inputPass);
            if (encrypt(inputPass) == passwordEncrypted) return true;
            attempts++;
            cout << "Incorrect password. Attempts left: " << (3 - attempts) << endl;
        }
        cout << "Account locked after 3 failed attempts.\n";
        return false;
    }

    void displayInfo() const {
        cout << "UserID: " << userID << "\nName: " << name
             << "\nCNIC: " << CNIC << "\nAddress: " << address << endl;
    }

    friend class Account;
};
int User::userCount = 0;

class Account {
protected:
    static int nextAccNum;

public:
    int accountNumber;
    double balance;
    User user;

    Account() {
        balance = 0;
        accountNumber = ++nextAccNum;
    }

    virtual void createAccount() {
        user.inputDetails();
        cout << "Enter Initial Deposit: ";
        cin >> balance;
    }

    virtual void deposit(double amount) {
        if (amount <= 0) {
            cout << "Invalid amount.\n";
            return;
        }
        balance += amount;
        cout << "Deposited: " << amount << "\nNew Balance: " << balance << endl;
    }

    virtual void withdraw(double amount) {
        if (amount > balance || amount <= 0) {
            cout << "Invalid withdrawal.\n";
            return;
        }
        balance -= amount;
        cout << "Withdrawn: " << amount << "\nRemaining Balance: " << balance << endl;
    }

    virtual void printStatement() const {
        cout << "--------------------------\n";
        cout << "Account #: " << accountNumber << "\nBalance: " << balance << endl;
        user.displayInfo();
    }

    virtual double calculateInterest() const = 0;

    int getAccountNumber() const {
        return accountNumber;
    }

    bool loginToAccount() {
        cin.ignore();
        return user.login();
    }
};
int Account::nextAccNum = 1000;

class SavingsAccount : public Account {
public:
    double calculateInterest() const override {
        return balance * 0.05;
    }

    void printStatement() const override {
        cout << "\n[Savings Account]\n";
        Account::printStatement();
        cout << "Interest (5%): " << calculateInterest() << endl;
    }
};

class CurrentAccount : public Account {
public:
    double calculateInterest() const override {
        return 0;
    }

    void printStatement() const override {
        cout << "\n[Current Account]\n";
        Account::printStatement();
    }
};

class Bank {
    Account* accounts[100];
    int accountCount = 0;

public:
    ~Bank() {
        for (int i = 0; i < accountCount; i++) {
            delete accounts[i];
        }
    }

    void saveAccountsToFile() const {
        ofstream file("accounts.txt");
        if (!file) {
            cout << "File error.\n";
            return;
        }
        for (int i = 0; i < accountCount; i++) {
            file << accounts[i]->getAccountNumber() << endl;
        }
        file.close();
        cout << "Accounts saved to accounts.txt\n";
    }

    void saveUserDataToFile() const {
        ofstream file("userdata.txt");
        if (!file) {
            cout << "Failed to open userdata.txt\n";
            return;
        }

        for (int i = 0; i < accountCount; i++) {
            string type = dynamic_cast<SavingsAccount*>(accounts[i]) ? "Savings" : "Current";
            file << accounts[i]->getAccountNumber() << ","
                 << type << ","
                 << accounts[i]->user.name << ","
                 << accounts[i]->user.CNIC << ","
                 << accounts[i]->user.address << ","
                 << accounts[i]->user.passwordEncrypted << ","
                 << accounts[i]->balance << "\n";
        }

        file.close();
        cout << "User data saved to userdata.txt\n";
    }

    void createNewAccount() {
        if (accountCount >= 100) {
            cout << "Bank is full. Cannot create more accounts.\n";
            return;
        }

        int choice;
        cout << "Select Account Type:\n1. Savings\n2. Current\nChoice: ";
        cin >> choice;

        Account* acc = nullptr;
        if (choice == 1)
            acc = new SavingsAccount();
        else if (choice == 2)
            acc = new CurrentAccount();
        else {
            cout << "Invalid account type selected.\n";
            return;
        }

        acc->createAccount();
        accounts[accountCount++] = acc;
        cout << "Account Created Successfully.\n";
        cout << "Your Account Number is: " << acc->getAccountNumber() << endl;

        saveAccountsToFile();
        saveUserDataToFile();
    }

    Account* findAccount(int accNum) const {
        for (int i = 0; i < accountCount; i++) {
            if (accounts[i]->getAccountNumber() == accNum)
                return accounts[i];
        }
        return nullptr;
    }

    void depositToAccount(int accNum, double amount) {
        Account* acc = findAccount(accNum);
        if (acc && acc->loginToAccount()) {
            acc->deposit(amount);
            saveAccountsToFile();
            saveUserDataToFile();
        } else {
            cout << "Access Denied or Account not found.\n";
        }
    }

    void withdrawFromAccount(int accNum, double amount) {
        Account* acc = findAccount(accNum);
        if (acc && acc->loginToAccount()) {
            acc->withdraw(amount);
            saveAccountsToFile();
            saveUserDataToFile();
        } else {
            cout << "Access Denied or Account not found.\n";
        }
    }

    void showAllAccounts() const {
        for (int i = 0; i < accountCount; i++) {
            accounts[i]->printStatement();
        }
    }

    void employeeView() const {
        showAllAccounts();
    }

    void customerView(int accNum) const {
        Account* acc = findAccount(accNum);
        if (acc) {
            acc->printStatement();
        } else {
            cout << "Account not found.\n";
        }
    }
};

int main() {
    Bank myBank;
    int choice;

    while (true) {
        cout << "\n========= SMART BANKING SYSTEM =========\n";
        cout << "1. Create New Account\n";
        cout << "2. Deposit Money\n";
        cout << "3. Withdraw Money\n";
        cout << "4. Customer View (Your Account Details)\n";
        cout << "5. Employee View (All Accounts)\n";
        cout << "6. Exit\n";
        cout << "Select an option: ";
        cin >> choice;

        if (choice == 1) {
            myBank.createNewAccount();
        }
        else if (choice == 2) {
            int accNum;
            double amount;
            cout << "Enter Account Number: ";
            cin >> accNum;
            cout << "Enter Amount to Deposit: ";
            cin >> amount;
            myBank.depositToAccount(accNum, amount);
        }
        else if (choice == 3) {
            int accNum;
            double amount;
            cout << "Enter Account Number: ";
            cin >> accNum;
            cout << "Enter Amount to Withdraw: ";
            cin >> amount;
            myBank.withdrawFromAccount(accNum, amount);
        }
        else if (choice == 4) {
            int accNum;
            cout << "Enter Account Number: ";
            cin >> accNum;
            if (myBank.findAccount(accNum) && myBank.findAccount(accNum)->loginToAccount()) {
                myBank.customerView(accNum);
            } else {
                cout << "Access Denied or Account not found.\n";
            }
        }
        else if (choice == 5) {
            myBank.employeeView();
        }
        else if (choice == 6) {
            cout << "Exiting... Thank you for banking with us!\n";
            break;
        }
        else {
            cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}
