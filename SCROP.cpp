#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <limits>
#include <unordered_map>

using namespace std;

class Customer;
class Order;

class FileHandler {
public:
    virtual void saveToFile(const string& filename) const = 0;
    virtual void loadFromFile(const string& filename) = 0;
    virtual ~FileHandler() = default;
};

class DiscountStrategy {
public:
    virtual double calculateDiscount(double originalPrice, int quantity) const = 0;
    virtual string getDiscountType() const = 0;
    virtual ~DiscountStrategy() = default;
};

class PercentageDiscount : public DiscountStrategy {
    double percentage;
public:
    PercentageDiscount(double percent) : percentage(percent) {
        if (percent < 0 || percent > 100) {
            throw invalid_argument("Percentage must be between 0 and 100");
        }
    }
    
    double calculateDiscount(double price, int qty) const override {
        return price * qty * (percentage / 100.0);
    }
    
    string getDiscountType() const override {
        return "Percentage (" + to_string(percentage) + "%)";
    }
};

class BulkDiscount : public DiscountStrategy {
    int minQty;
    double discount;
public:
    BulkDiscount(int minQty, double disc) : minQty(minQty), discount(disc) {
        if (minQty <= 0 || disc < 0) {
            throw invalid_argument("Invalid bulk discount parameters");
        }
    }
    
    double calculateDiscount(double price, int qty) const override {
        return (qty >= minQty) ? discount * qty : 0.0;
    }
    
    string getDiscountType() const override {
        return "Bulk (Min: " + to_string(minQty) + ", P" + to_string(discount) + ")";
    }
};

class FoodItem {
public:
    enum Category { VEGETABLE, FRUIT, GRAIN, DAIRY };

private:
    string name, description;
    int quantity;
    double price;
    Category category;

public:
    FoodItem(string name = "", int qty = 0, double price = 0.0, Category cat = VEGETABLE, string desc = "")
        : name(name), quantity(qty), price(price), category(cat), description(desc) {
        validateInput();
    }

    // Getters
    string getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }
    Category getCategory() const { return category; }
    string getDescription() const { return description; }

    // Setters with validation
    void setQuantity(int q) { 
        if (q < 0) throw invalid_argument("Quantity cannot be negative");
        quantity = q; 
    }
    
    void setPrice(double p) { 
        if (p < 0) throw invalid_argument("Price cannot be negative");
        price = p; 
    }

    string getCategoryString() const {
        switch (category) {
            case VEGETABLE: return "Vegetable";
            case FRUIT: return "Fruit";
            case GRAIN: return "Grain";
            case DAIRY: return "Dairy";
            default: return "Unknown";
        }
    }

    void display() const {
        cout << name << " - P" << fixed << setprecision(2) << price
             << " (Qty: " << quantity << ", " << getCategoryString() << ")\n";
        if (!description.empty()) {
            cout << "   Description: " << description << '\n';
        }
    }

private:
    void validateInput() {
        if (name.empty()) throw invalid_argument("Item name cannot be empty");
        if (quantity < 0) throw invalid_argument("Quantity cannot be negative");
        if (price < 0) throw invalid_argument("Price cannot be negative");
    }
};

class Farm {
protected:
    string name, address, contactnumber;
    vector<FoodItem> inventory;
    double deliveryFee;
    double minimumOrder;

public:
    Farm(string n, string a, string c, double delFee = 50.0, double minOrder = 100.0) 
        : name(n), address(a), contactnumber(c), deliveryFee(delFee), minimumOrder(minOrder) {
        validateInput();
    }
    
    virtual string getFarmType() const = 0;
    virtual void saveToFile(ofstream& out) const = 0;
    virtual ~Farm() = default;

    // Inventory management
    size_t getInventorySize() const { return inventory.size(); }
    
    FoodItem* getItem(size_t idx) {
        if (idx >= inventory.size()) return nullptr;
        return &inventory[idx];
    }
    
    const FoodItem* getItem(size_t idx) const {
        if (idx >= inventory.size()) return nullptr;
        return &inventory[idx];
    }
    
    void addItem(const FoodItem& item) { 
        inventory.push_back(item); 
    }
    
    bool removeItem(const string& itemName) {
        auto it = remove_if(inventory.begin(), inventory.end(),
            [&itemName](const FoodItem& item) { return item.getName() == itemName; });
        bool removed = (it != inventory.end());
        inventory.erase(it, inventory.end());
        return removed;
    }
    
    bool updateItemPrice(const string& itemName, double newPrice) {
        if (newPrice < 0) return false;
        for (auto& item : inventory) {
            if (item.getName() == itemName) {
                item.setPrice(newPrice);
                return true;
            }
        }
        return false;
    }
    
    void displayMenu() const {
        cout << "\nMenu for " << name << ":\n";
        for (size_t i = 0; i < inventory.size(); ++i) {
            cout << i + 1 << ". ";
            inventory[i].display();
        }
    }

    // Getters and Setters
    string getName() const { return name; }
    string getAddress() const { return address; }
    string getPhoneNumber() const { return contactnumber; }
    double getDeliveryFee() const { return deliveryFee; }
    double getMinimumOrder() const { return minimumOrder; }
    
    void setName(const string& n) { 
        if (n.empty()) throw invalid_argument("Farm name cannot be empty");
        name = n; 
    }
    
    void setAddress(const string& a) { 
        if (a.empty()) throw invalid_argument("Farm address cannot be empty");
        address = a; 
    }
    
    void setPhoneNumber(const string& c) { 
        if (c.empty()) throw invalid_argument("Farm phone number cannot be empty");
        contactnumber = c; 
    }

private:
    void validateInput() {
        if (name.empty()) throw invalid_argument("Farm name cannot be empty");
        if (address.empty()) throw invalid_argument("Farm address cannot be empty");
        if (contactnumber.empty()) throw invalid_argument("Farm contact number cannot be empty");
        if (deliveryFee < 0) throw invalid_argument("Delivery fee cannot be negative");
        if (minimumOrder < 0) throw invalid_argument("Minimum order cannot be negative");
    }
};

class VegetableFarm : public Farm {
public:
    VegetableFarm(string n, string a, string c) : Farm(n, a, c) {}
    string getFarmType() const override { return "Vegetable Farm"; }
    void saveToFile(ofstream& out) const override {
        out << getFarmType() << "|" << name << "|" << address << "|" << contactnumber << endl;
    }
};

class FruitFarm : public Farm {
public:
    FruitFarm(string n, string a, string c) : Farm(n, a, c) {}
    string getFarmType() const override { return "Fruit Farm"; }
    void saveToFile(ofstream& out) const override {
        out << getFarmType() << "|" << name << "|" << address << "|" << contactnumber << endl;
    }
};

class GrainFarm : public Farm {
public:
    GrainFarm(string n, string a, string c) : Farm(n, a, c) {}
    string getFarmType() const override { return "Grain Farm"; }
    void saveToFile(ofstream& out) const override {
        out << getFarmType() << "|" << name << "|" << address << "|" << contactnumber << endl;
    }
};

class DairyFarm : public Farm {
public:
    DairyFarm(string n, string a, string c) : Farm(n, a, c) {}
    string getFarmType() const override { return "Dairy Farm"; }
    void saveToFile(ofstream& out) const override {
        out << getFarmType() << "|" << name << "|" << address << "|" << contactnumber << endl;
    }
};

class PaymentMethod{
public:
    virtual void processPayment(int orderCode, double total) const = 0;
    virtual string getPaymentType() const = 0;
    virtual bool requiresOnlineVerification() const = 0;
    virtual double getProcessingFee(double amount) const = 0;
    virtual ~PaymentMethod() = default;
};

class CashPayment : public PaymentMethod {
public:
    void processPayment(int orderCode, double total) const override {
        cout << "\n=== CASH PAYMENT ===" << endl;
        cout << "Receipt Number: " << orderCode << endl;
        cout << "Total Amount: P" << fixed << setprecision(2) << total << endl;
        cout << "Payment processed successfully! Please proceed to claim your order." << endl;
    }

    string getPaymentType() const override { return "Cash"; }
    bool requiresOnlineVerification() const override { return false; }
    double getProcessingFee(double amount) const override { return 0.0; }
};

class DigitalWalletPayment : public PaymentMethod {
protected:
    string walletName;
    double processingFeeRate;

public:
    DigitalWalletPayment(string name, double feeRate) 
        : walletName(name), processingFeeRate(feeRate) {
        if (feeRate < 0 || feeRate > 1.0) {
            throw invalid_argument("Processing fee rate must be between 0 and 1");
        }
    }

    void processPayment(int orderCode, double total) const override {
        double fee = getProcessingFee(total);
        double finalAmount = total + fee;

        cout << "\n=== " << walletName << " PAYMENT ===" << endl;
        cout << "Receipt Number: " << orderCode << endl;
        cout << "Total Amount: P" << fixed << setprecision(2) << finalAmount << endl;
        cout << "Payment processed successfully! Please proceed to claim your order." << endl;
    }

    string getPaymentType() const override { return walletName; }
    bool requiresOnlineVerification() const override { return true; }
    double getProcessingFee(double amount) const override {
        return amount * processingFeeRate;
    }
};

class GCashPayment : public DigitalWalletPayment {
public:
    GCashPayment() : DigitalWalletPayment("GCash", 0.02) {}
};

class PayMayaPayment : public DigitalWalletPayment {
public:
    PayMayaPayment() : DigitalWalletPayment("PayMaya", 0.025) {}
};

class CreditCardPayment : public PaymentMethod {
private:
    string cardType;
    static const double PROCESSING_FEE_RATE;

public:
    CreditCardPayment(string type) : cardType(type) {
        if (type.empty()) throw invalid_argument("Card type cannot be empty");
    }

    void processPayment(int orderCode, double total) const override {
        double fee = getProcessingFee(total);
        double finalAmount = total + fee;

        cout << "\n=== " << cardType << " CREDIT CARD PAYMENT ===" << endl;
        cout << "Receipt Number: " << orderCode << endl;
        cout << "Total Amount: P" << fixed << setprecision(2) << finalAmount << endl;
        cout << "Payment processed successfully! Please proceed to claim your order." << endl;
    }

    string getPaymentType() const override { return cardType + " Credit Card"; }
    bool requiresOnlineVerification() const override { return true; }
    double getProcessingFee(double amount) const override {
        return amount * PROCESSING_FEE_RATE;
    }
};

const double CreditCardPayment::PROCESSING_FEE_RATE = 0.035;

class Customer: public FileHandler{
private:
    string name;
    string email;
    string phonenumber;
    vector<int> orderHistory;

public:
    Customer(string name = "", string email = "", string phone = "")
        : name(name), email(email), phonenumber(phone) {
        validateInput();
    }

    void displayProfile() const {
        cout << "Customer Profile:" << endl;
        cout << "Name: " << name << endl;
        cout << "Email: " << email << endl;
        cout << "Phone: " << phonenumber << endl;
    }

    void addOrderToHistory(int orderCode) {
        if (orderCode <= 0) throw invalid_argument("Invalid order code");
        orderHistory.push_back(orderCode);
    }

    // Getters
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getPhoneNumber() const { return phonenumber; }

    void saveToFile(const string& filename) const override {
        ofstream outfile(filename, ios::app);
        if (!outfile.is_open()) {
            throw runtime_error("Failed to open file for writing: " + filename);
        }
        outfile << "Customer|" << name << "|" << email << "|" << phonenumber << endl;
        outfile.close();
    }

    void loadFromFile(const string& filename) override {
        // Implementation for loading customer data
        ifstream infile(filename);
        if (!infile.is_open()) {
            throw runtime_error("Failed to open file for reading: " + filename);
        }
        // Add implementation as needed
        infile.close();
    }

private:
    void validateInput() {
        if (name.empty()) throw invalid_argument("Customer name cannot be empty");
        if (email.empty()) throw invalid_argument("Customer email cannot be empty");
        if (phonenumber.empty()) throw invalid_argument("Customer phone number cannot be empty");
    }
};

class OrderItem{
private:
    FoodItem item;
    int orderedQuantity;
    unique_ptr<DiscountStrategy> discountStrategy;
    string specialInstructions;

public:
    OrderItem(const FoodItem& item, int orderedQuantity, unique_ptr<DiscountStrategy> discount = nullptr, string instructions = "")
        : item(item), orderedQuantity(orderedQuantity), 
          discountStrategy(move(discount)), specialInstructions(instructions) {
        if (orderedQuantity <= 0) {
            throw invalid_argument("Ordered quantity must be positive");
        }
        if (orderedQuantity > item.getQuantity()) {
            throw invalid_argument("Ordered quantity exceeds available quantity");
        }
    }

    // Move constructor
    OrderItem(OrderItem&& other) noexcept
        : item(move(other.item)), orderedQuantity(other.orderedQuantity),
          discountStrategy(move(other.discountStrategy)), specialInstructions(move(other.specialInstructions)) {}

    // Move assignment
    OrderItem& operator=(OrderItem&& other) noexcept{
        if(this != &other) {
            item = move(other.item);
            orderedQuantity = other.orderedQuantity;
            discountStrategy = move(other.discountStrategy);
            specialInstructions = move(other.specialInstructions);
        }
        return *this;
    }

    // Delete copy operations
    OrderItem(const OrderItem&) = delete;
    OrderItem& operator = (const OrderItem&) = delete;

    double getSubtotal() const{
        return item.getPrice() * orderedQuantity;
    }

    double getDiscountAmount() const {
        if (discountStrategy) {
            return discountStrategy->calculateDiscount(item.getPrice(), orderedQuantity);
        }
        return 0.0;
    }

    double getTotal() const{
        return getSubtotal() - getDiscountAmount();
    }

    const FoodItem& getItem() const{
        return item;
    }

    int getOrderedQuantity() const{
        return orderedQuantity;
    }

    string getDiscountType() const {
        return discountStrategy ? discountStrategy->getDiscountType() : "No Discount";
    }

    string getSpecialInstructions() const {
        return specialInstructions;
    }   
};

class Order : public FileHandler{
private:
    vector<OrderItem> items;
    int orderCode;
    shared_ptr<Customer> customer;
    shared_ptr<Farm> farm;
    string orderDate;
    string status;
    string deliveryAddress;
    double totalAmount;

    static const int MIN_ORDER_CODE = 10000000;
    static const int MAX_ORDER_CODE = 99999999;

public:
    virtual ~Order() noexcept override = default;

    bool isEmpty() const {
        return items.empty();
    }

    Order(shared_ptr<Customer> cust = nullptr, shared_ptr<Farm> frm = nullptr)
    : customer(cust), farm(frm), status("Pending"), totalAmount(0.0){
        generateOrderCode();
        generateOrderDate();
    }

    // Move assignment
    Order& operator=(Order&& other) noexcept{
        if(this != &other){
            items = move(other.items);
            orderCode = other.orderCode;
            customer = other.customer;
            farm = other.farm;
            orderDate = move(other.orderDate);
            status = move(other.status);
            deliveryAddress = move(other.deliveryAddress);
            totalAmount = other.totalAmount;
        }
        return *this;
    }

    // Delete copy operations
    Order(const Order&) = delete;
    Order& operator =(const Order&) = delete;

    void addItem(OrderItem&& item){
        items.push_back(move(item));
    }

    double calculateSubtotal() const {
        double subtotal = 0;
        for (const auto& item : items) {
            subtotal += item.getSubtotal();
        }
        return subtotal;
    }

    double calculateTotalDiscount() const {
        double discount = 0;
        for (const auto& item : items) {
            discount += item.getDiscountAmount();
        }
        return discount;
    }

    double calculateDeliveryFee() const {
        return farm ? farm->getDeliveryFee() : 0.0;
    }

    double calculateTotal() const {
        return calculateSubtotal() - calculateTotalDiscount() + calculateDeliveryFee();
    }

    bool validateOrder() const {
        if (items.empty()) return false;
        if (farm && calculateSubtotal() < farm->getMinimumOrder()) return false;
        return true;
    }

    void generateReceipt(const PaymentMethod& payment) {
        if (!validateOrder()) {
            cout << "Order validation failed!" << endl;
            return;
        }

        totalAmount = calculateTotal();
        double processingFee = payment.getProcessingFee(totalAmount);
        double finalAmount = totalAmount + processingFee;

        cout << "\n" << string(50, '=') << endl;
        cout << "           OFFICIAL RECEIPT" << endl;
        cout << string(50, '=') << endl;
        cout << "Order Code: " << orderCode << endl;
        cout << "Date: " << orderDate << endl;
        if (customer) cout << "Customer: " << customer->getName() << endl;
        if (farm) cout << "Farm: " << farm->getName() << endl;
        cout << string(50, '-') << endl;

        for (const auto& item : items) {
            cout << item.getItem().getName() << " x" << item.getOrderedQuantity() << endl;
            cout << "  @ P" << fixed << setprecision(2) 
                 << item.getItem().getPrice() 
                 << " = P" << item.getSubtotal() << endl;
            if (item.getDiscountAmount() > 0) {
                cout << "  Discount (" 
                     << item.getDiscountType() 
                     << "): -P" 
                     << item.getDiscountAmount() 
                     << endl;
            }
            cout << "  Subtotal: P" 
                 << item.getTotal() 
                 << endl;
            if (!item.getSpecialInstructions().empty()) {
                cout << "  Special: " 
                     << item.getSpecialInstructions() 
                     << endl;
            }
            cout << endl;
        }

        cout << string(50, '-') << endl;
        cout << "Subtotal: P" << calculateSubtotal() << endl;
        cout << "Total Discount: -P" << calculateTotalDiscount() << endl;
        cout << "Delivery Fee: P" << calculateDeliveryFee() << endl;
        cout << "Processing Fee: P" << processingFee << endl;
        cout << "Total Amount: P" << finalAmount << endl;

        // Add order to customer history
        if (customer) {
            customer->addOrderToHistory(orderCode);
        }
    }

    void saveToFile(const string& filename) const override {
        ofstream outfile(filename, ios::app);
        if (!outfile.is_open()) {
            throw runtime_error("Failed to open file for writing: " + filename);
        }
        
        outfile << "Order|" << orderCode << "|" << orderDate << "|"
                << (customer ? customer->getName() : "N/A") << "|"
                << (farm ? farm->getName() : "N/A") << "|" 
                << status << "|" << deliveryAddress << "|" 
                << totalAmount << endl;

        for (const auto& item : items) {
            outfile << item.getItem().getName() << "," 
                    << item.getOrderedQuantity() << ","
                    << item.getDiscountType() << ","
                    << item.getSpecialInstructions() << endl;
        }
        outfile.close();
    }

    void loadFromFile(const string& filename) override {
        ifstream infile(filename);
        if (!infile.is_open()) {
            throw runtime_error("Failed to open file for reading: " + filename);
        }
        
        string line;
        while (getline(infile, line)) {
            istringstream iss(line);
            string token;
            getline(iss, token, '|');
            if (token == "Order") {
                getline(iss, token, '|');
                try {
                    orderCode = stoi(token);
                    getline(iss, orderDate, '|');
                    getline(iss, token, '|'); // customer name
                    getline(iss, token, '|'); // farm name
                    getline(iss, status, '|');
                    getline(iss, deliveryAddress, '|');
                    getline(iss, token);
                    totalAmount = stod(token);
                } catch (const exception& e) {
                    cerr << "Error parsing order data: " << e.what() << endl;
                }
            }
        }
        infile.close();
    }

    int getOrderCode() const { return orderCode; }

private:
    void generateOrderCode() {
        srand(static_cast<unsigned int>(time(0)));
        orderCode = MIN_ORDER_CODE + rand() % (MAX_ORDER_CODE - MIN_ORDER_CODE + 1);
    }

    void generateOrderDate() {
        time_t now = time(0);
        char* dateStr = ctime(&now);
        orderDate = string(dateStr);
        if (!orderDate.empty() && orderDate.back() == '\n') {
            orderDate.pop_back();
        }
    }
};

class InputValidator {
public:
    static int getValidatedIntInput(const string& prompt, int min = INT_MIN, int max = INT_MAX) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value && value >= min && value <= max) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "Invalid input. Please enter a number between " << min << " and " << max << "." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    static double getValidatedDoubleInput(const string& prompt, double min = 0.0, double max = DBL_MAX) {
        double value;
        while (true) {
            cout << prompt;
            if (cin >> value && value >= min && value <= max) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "Invalid input. Please enter a number between " << min << " and " << max << "." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    static string getValidatedStringInput(const string& prompt, bool allowEmpty = false) {
        string value;
        while (true) {
            cout << prompt;
            getline(cin, value);
            if (!value.empty() || allowEmpty) {
                return value;
            }
            cout << "Input cannot be empty. Please try again." << endl;
        }
    }
};

class OrderingSystem{
private:
    vector<shared_ptr<Farm>> farms;
    unordered_map<string, shared_ptr<Customer>> customers; // Using email as key for O(1) lookup
    Order currentOrder;
    shared_ptr<Customer> currentCustomer;

public:
    OrderingSystem() : currentOrder(), currentCustomer(nullptr) {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    void run() {
        try {
            loadFarms();
            loadCustomers();

            cout << "Welcome to SCROP, your go-to produce surplus platform!" << endl;

            handleCustomerAuth();

            while (true) {
                displayMainMenu();
                int choice = InputValidator::getValidatedIntInput("Choice: ", 0, 5);

                switch (choice) {
                    case 1:
                        browseFarms();
                        break;
                    case 2:
                        viewCustomerProfile();
                        break;
                    case 3:
                        viewOrderHistory();
                        break;
                    case 4:
                        checkout();
                        break;
                    case 5:
                        cout << "Exiting the system. Goodbye!" << endl;
                        return;
                    case 0:
                        saveAllData();
                        cout << "Thank you for using our system!" << endl;
                        return;
                    default:
                        cout << "Invalid choice. Please try again." << endl;
                }
            }
        } catch (const exception& e) {
            cerr << "System error: " << e.what() << endl;
        }
    }

private:
    void displayMainMenu() {
        cout << "\n" << string(40, '=') << endl;
        cout << "    SCROP PRODUCE SURPLUS PLATFORM" << endl;
        cout << string(40, '=') << endl;
        cout << "1. Browse Farms" << endl;
        cout << "2. View Customer Profile" << endl;
        cout << "3. View Order History" << endl;
        cout << "4. Checkout" << endl;
        cout << "5. Exit" << endl;
        cout << "0. Save and Exit" << endl;
    }

    void handleCustomerAuth() {
        try {
            int choice = InputValidator::getValidatedIntInput("\n1. Login\n2. Register\nChoice: ", 1, 2);

            string name = InputValidator::getValidatedStringInput("Enter name: ");
            string email = InputValidator::getValidatedStringInput("Enter email: ");
            string phone = InputValidator::getValidatedStringInput("Enter phone: ");

            auto it = customers.find(email);
            if (it != customers.end()) {
                currentCustomer = it->second;
                cout << "Welcome back, " << currentCustomer->getName() << "!" << endl;
            } else {
                auto newCustomer = make_shared<Customer>(name, email, phone);
                customers[email] = newCustomer;
                currentCustomer = newCustomer;
                cout << "Registration successful! Welcome, " << currentCustomer->getName() << "!" << endl;
            }
        } catch (const exception& e) {
            cerr << "Error during authentication: " << e.what() << endl;
            throw;
        }
    }

    void browseFarms() {
        if (farms.empty()) {
            cout << "No farms available!" << endl;
            return;
        }

        cout << "\nAvailable Farms:" << endl;
        for (size_t i = 0; i < farms.size(); ++i) {
            cout << i + 1 << ". " << farms[i]->getName() << " (" << farms[i]->getFarmType() << ")" << endl;
        }

        int farmChoice = InputValidator::getValidatedIntInput("Select farm (0 to return): ", 0, static_cast<int>(farms.size()));

        if (farmChoice == 0) return;

        auto selectedFarm = farms[farmChoice - 1];
        currentOrder = Order(currentCustomer, selectedFarm);
        browseMenu(selectedFarm);
    }

    void browseMenu(shared_ptr<Farm> farm){
        while(true){
            farm->displayMenu();
            int itemChoice = InputValidator::getValidatedIntInput("\nSelect item (0 to return): ", 0, static_cast<int>(farm->getInventorySize()));

            if (itemChoice == 0) return;

            FoodItem* selectedItem = farm->getItem(itemChoice - 1);
            if (!selectedItem) {
                cout << "Item not found!" << endl;
                continue;
            }

            cout << "Selected: " << selectedItem->getName() << endl;
            cout << "Available quantity: " << selectedItem->getQuantity() << endl;
            
            int qty = InputValidator::getValidatedIntInput("Enter quantity to order: ", 1, selectedItem->getQuantity());

            unique_ptr<DiscountStrategy> discount = selectDiscount();

            string instructions = InputValidator::getValidatedStringInput("Special instructions (optional): ", true);

            try {
                OrderItem orderItem(*selectedItem, qty, move(discount), instructions);
                currentOrder.addItem(move(orderItem));

                selectedItem->setQuantity(selectedItem->getQuantity() - qty);

                cout << "Item added to order!" << endl;
                
                string continueChoice = InputValidator::getValidatedStringInput("Continue ordering? (y/n): ");
                if (continueChoice != "y" && continueChoice != "Y") break;
            } catch (const exception& e) {
                cout << "Error adding item to order: " << e.what() << endl;
            }
        }
    }

    unique_ptr<DiscountStrategy> selectDiscount() {
        int discountType = rand() % 3;

        try {
            switch (discountType) {
                case 1: {
                    double percent = 5 + rand() % 16;
                    cout << "Random Discount Applied: " << percent << "% Percentage Discount" << endl;
                    return make_unique<PercentageDiscount>(percent);
                }
                case 2: {
                    int minQty = 3 + rand() % 8;
                    double amount = 2 + rand() % 9;
                    cout << "Random Discount Applied: Bulk Discount (minQty: " << minQty << ", amount: P" << amount << ")" << endl;
                    return make_unique<BulkDiscount>(minQty, amount);
                }
                default:
                    cout << "No discount applied." << endl;
                    return nullptr;
            }
        } catch (const exception& e) {
            cout << "Error applying discount: " << e.what() << endl;
            return nullptr;
        }
    }

    void checkout(){
        try {
            if(currentOrder.isEmpty()) {
                cout << "No items in current order!" << endl;
                return;
            }
            if(!currentOrder.validateOrder()) {
                cout << "Order validation failed. Please check minimum order requirements." << endl;
                return;
            }

            cout << "\nSelect payment method:" << endl;
            cout << "1. Cash" << endl;
            cout << "2. GCash" << endl;
            cout << "3. PayMaya" << endl;
            cout << "4. Credit Card (Visa)" << endl;
            cout << "5. Credit Card (Mastercard)" << endl;

            int paymentChoice = InputValidator::getValidatedIntInput("Choice: ", 1, 5);

            unique_ptr<PaymentMethod> payment;
            switch (paymentChoice) {
                case 1: 
                    payment = make_unique<CashPayment>(); 
                    break;
                case 2: 
                    payment = make_unique<GCashPayment>(); 
                    break;
                case 3: 
                    payment = make_unique<PayMayaPayment>(); 
                    break;
                case 4: 
                    payment = make_unique<CreditCardPayment>("Visa"); 
                    break;
                case 5: 
                    payment = make_unique<CreditCardPayment>("Mastercard"); 
                    break;
                default: 
                    cout << "Invalid payment method!" << endl; 
                    return;
            }

            currentOrder.generateReceipt(*payment);
            
            // Save the order before resetting
            currentOrder.saveToFile("orders.txt");
            
            // Reset for next order
            currentOrder = Order();
        } catch (const exception& e) {
            cout << "Error during checkout: " << e.what() << endl;
        }
    }

    void viewCustomerProfile() {
        if(currentCustomer) {
            currentCustomer->displayProfile();
        } else {
            cout << "No customer logged in!" << endl;
        }
    }

    void viewOrderHistory() {
        cout << "\nOrder History:" << endl;

        try {
            ifstream infile("orders.txt");
            if(!infile.is_open()) {
                cout << "No order history found." << endl;
                return;
            }

            string line;
            bool foundOrders = false;
            while (getline(infile, line)){
                if(line.find("Order|") != string::npos && currentCustomer) {
                    // Parse the order line to check if it belongs to current customer
                    istringstream ss(line);
                    string orderPrefix, orderCode, orderDate, customerName;
                    getline(ss, orderPrefix, '|');
                    getline(ss, orderCode, '|');
                    getline(ss, orderDate, '|');
                    getline(ss, customerName, '|');
                    
                    if (customerName == currentCustomer->getName()) {
                        foundOrders = true;
                        cout << "Order #" << orderCode << " - " << orderDate << endl;
                    }
                }
            }

            if(!foundOrders) {
                cout << "No orders found in history." << endl;
            }
            
            infile.close();
        } catch (const exception& e) {
            cout << "Error reading order history: " << e.what() << endl;
        }
    }

    void manageFarms(){
        while (true) {
            cout << "\n=== FARM MANAGEMENT ===" << endl;
            cout << "1. Add Farm" << endl;
            cout << "2. Remove Farm" << endl;
            cout << "3. Update Farm Details" << endl;
            cout << "4. View Farms" << endl;
            cout << "5. Add Item to Farm" << endl;
            cout << "6. Remove Item from Farm" << endl;
            cout << "7. Update Item Price" << endl;
            cout << "0. Return to Main Menu" << endl;

            int choice = InputValidator::getValidatedIntInput("Choice: ", 0, 7);

            try {
                switch (choice) {
                    case 1: addFarm(); break;
                    case 2: removeFarm(); break;
                    case 3: updateFarmDetails(); break;
                    case 4: viewFarms(); break;
                    case 5: addItemToFarm(); break;
                    case 6: removeItemFromFarm(); break;
                    case 7: updateItemPriceInFarm(); break;
                    case 0: return;
                    default: cout << "Invalid choice!" << endl;
                }
            } catch (const exception& e) {
                cout << "Error in farm management: " << e.what() << endl;
            }
        }
    }

    void viewFarms() {
        if (farms.empty()) {
            cout << "No farms available!" << endl;
            return;
        }
        cout << "\n=== List of Farms ===" << endl;
        for (size_t i = 0; i < farms.size(); ++i) {
            cout << i + 1 << ". " << farms[i]->getName() << " (" << farms[i]->getFarmType() << ")" << endl;
            cout << "   Address: " << farms[i]->getAddress() << endl;
            cout << "   Phone: " << farms[i]->getPhoneNumber() << endl;
            cout << "   Items: " << farms[i]->getInventorySize() << endl;
        }
    }

    void addFarm() {
        try {
            string name = InputValidator::getValidatedStringInput("Enter farm name: ");
            string address = InputValidator::getValidatedStringInput("Enter farm address: ");
            string phone = InputValidator::getValidatedStringInput("Enter farm phone number: ");

            cout << "Select farm type:" << endl;
            cout << "1. Vegetable Farm" << endl;
            cout << "2. Fruit Farm" << endl;
            cout << "3. Grain Farm" << endl;
            cout << "4. Dairy Farm" << endl;
            
            int farmType = InputValidator::getValidatedIntInput("Choice: ", 1, 4);
            
            shared_ptr<Farm> newFarm;
            switch (farmType) {
                case 1: newFarm = make_shared<VegetableFarm>(name, address, phone); break;
                case 2: newFarm = make_shared<FruitFarm>(name, address, phone); break;
                case 3: newFarm = make_shared<GrainFarm>(name, address, phone); break;
                case 4: newFarm = make_shared<DairyFarm>(name, address, phone); break;
                default: cout << "Invalid farm type!" << endl; return;
            }

            farms.push_back(newFarm);
            cout << "Farm added successfully!" << endl;
        } catch (const exception& e) {
            cout << "Error adding farm: " << e.what() << endl;
        }
    }

    void removeFarm() {
        if (farms.empty()) {
            cout << "No farms to remove!" << endl;
            return;
        }

        cout << "\nSelect farm to remove:" << endl;
        for (size_t i = 0; i < farms.size(); ++i) {
            cout << i + 1 << ". " << farms[i]->getName() << endl;
        }

        int choice = InputValidator::getValidatedIntInput("Choice: ", 1, static_cast<int>(farms.size()));

        farms.erase(farms.begin() + choice - 1);
        cout << "Farm removed successfully!" << endl;
    }

    void updateFarmDetails() {
        if (farms.empty()) {
            cout << "No farms available!" << endl;
            return;
        }

        cout << "\nSelect farm to update:" << endl;
        for (size_t i = 0; i < farms.size(); ++i) {
            cout << i + 1 << ". " << farms[i]->getName() << endl;
        }

        int choice = InputValidator::getValidatedIntInput("Choice: ", 1, static_cast<int>(farms.size()));

        auto farm = farms[choice - 1];
        
        try {
            string name = InputValidator::getValidatedStringInput("Enter new name (current: " + farm->getName() + "): ");
            string address = InputValidator::getValidatedStringInput("Enter new address (current: " + farm->getAddress() + "): ");
            string phone = InputValidator::getValidatedStringInput("Enter new phone number (current: " + farm->getPhoneNumber() + "): ");

            farm->setName(name);
            farm->setAddress(address);
            farm->setPhoneNumber(phone);

            cout << "Farm details updated successfully!" << endl;
        } catch (const exception& e) {
            cout << "Error updating farm: " << e.what() << endl;
        }
    }

    void addItemToFarm(){
        if(farms.empty()) {
            cout << "No farms available!" << endl;
            return;
        }

        cout << "\nSelect farm to add item:" << endl;
        for (size_t i = 0; i < farms.size(); ++i) {
            cout << i + 1 << ". " << farms[i]->getName() << endl;
        }

        int choice = InputValidator::getValidatedIntInput("Choice: ", 1, static_cast<int>(farms.size()));
        auto farm = farms[choice - 1];

        try {
            string name = InputValidator::getValidatedStringInput("Enter item name: ");
            string description = InputValidator::getValidatedStringInput("Enter description: ", true);
            int quantity = InputValidator::getValidatedIntInput("Enter quantity: ", 0);
            double price = InputValidator::getValidatedDoubleInput("Enter price: ", 0.0);
            
            cout << "Select category:" << endl;
            cout << "0. Vegetable" << endl;
            cout << "1. Fruit" << endl;
            cout << "2. Grain" << endl;
            cout << "3. Dairy" << endl;
            
            int category = InputValidator::getValidatedIntInput("Choice: ", 0, 3);

            FoodItem::Category cat = static_cast<FoodItem::Category>(category);
            FoodItem newItem(name, quantity, price, cat, description);
            farm->addItem(newItem);

            cout << "Item added successfully!" << endl;
        } catch (const exception& e) {
            cout << "Error adding item: " << e.what() << endl;
        }
    }

    void removeItemFromFarm(){
        if (farms.empty()) {
            cout << "No farms available!" << endl;
            return;
        }

        cout << "\nSelect farm to remove item from:" << endl;
        for (size_t i = 0; i < farms.size(); ++i) {
            cout << i + 1 << ". " << farms[i]->getName() << endl;
        }

        int choice = InputValidator::getValidatedIntInput("Choice: ", 1, static_cast<int>(farms.size()));
        auto farm = farms[choice - 1];
        
        if (farm->getInventorySize() == 0) {
            cout << "Farm has no items to remove!" << endl;
            return;
        }

        farm->displayMenu();

        string itemName = InputValidator::getValidatedStringInput("Enter item name to remove: ");

        if (farm->removeItem(itemName)) {
            cout << "Item removed successfully!" << endl;
        } else {
            cout << "Item not found!" << endl;
        }
    }

    void updateItemPriceInFarm() {
        if (farms.empty()) {
            cout << "No farms available!" << endl;
            return;
        }

        cout << "\nSelect farm to update item price:" << endl;
        for (size_t i = 0; i < farms.size(); ++i) {
            cout << i + 1 << ". " << farms[i]->getName() << endl;
        }

        int choice = InputValidator::getValidatedIntInput("Choice: ", 1, static_cast<int>(farms.size()));
        auto farm = farms[choice - 1];

        if (farm->getInventorySize() == 0) {
            cout << "Farm has no items!" << endl;
            return;
        }

        farm->displayMenu();

        string itemName = InputValidator::getValidatedStringInput("Enter item name to update price: ");
        double newPrice = InputValidator::getValidatedDoubleInput("Enter new price: ", 0.0);

        if (farm->updateItemPrice(itemName, newPrice)) {
            cout << "Item price updated successfully!" << endl;
        } else {
            cout << "Item not found!" << endl;
        }
    }

    void loadFarms() {
        try {
            ifstream infile("farms.txt");
            if (!infile.is_open()) {
                cout << "[INFO] farms.txt not found. Creating sample farms..." << endl;
                createSampleFarms();
                return;
            }

            string line;
            while (getline(infile, line)) {
                if (line == "END" || line.empty()) continue;

                istringstream ss(line);
                string type, name, address, phone;

                getline(ss, type, '|');
                getline(ss, name, '|');
                getline(ss, address, '|');
                getline(ss, phone);

                shared_ptr<Farm> farm;
                if (type == "Vegetable Farm") {
                    farm = make_shared<VegetableFarm>(name, address, phone);
                } else if (type == "Fruit Farm") {
                    farm = make_shared<FruitFarm>(name, address, phone);
                } else if (type == "Grain Farm") {
                    farm = make_shared<GrainFarm>(name, address, phone);
                } else if (type == "Dairy Farm") {
                    farm = make_shared<DairyFarm>(name, address, phone);
                }

                if (farm) {
                    farms.push_back(farm);
                    
                    // Load items for this farm
                    while (getline(infile, line) && line != "END") {
                        if (line.empty()) continue;

                        istringstream itemSS(line);
                        string itemName, qtyStr, priceStr, catStr, desc;

                        getline(itemSS, itemName, ',');
                        getline(itemSS, qtyStr, ',');
                        getline(itemSS, priceStr, ',');
                        getline(itemSS, catStr, ',');
                        getline(itemSS, desc);

                        try {
                            int quantity = stoi(qtyStr);
                            double price = stod(priceStr);
                            int catInt = stoi(catStr);
                            if (catInt >= 0 && catInt <= 3) {
                                FoodItem::Category category = static_cast<FoodItem::Category>(catInt);
                                FoodItem item(itemName, quantity, price, category, desc);
                                farm->addItem(item);
                            }
                        } catch (const exception& e) {
                            cerr << "[WARNING] Failed to parse item: " << line << " - " << e.what() << endl;
                        }
                    }
                }
            }
            infile.close();
            cout << "[INFO] Loaded " << farms.size() << " farms successfully." << endl;
        } catch (const exception& e) {
            cerr << "Error loading farms: " << e.what() << endl;
            createSampleFarms();
        }
    }

    void createSampleFarms() {
        try {
            farms.push_back(make_shared<VegetableFarm>("Green Fields", "123 Veggie Lane", "123-456-7890"));
            farms.push_back(make_shared<FruitFarm>("Fruit Haven", "456 Fruit St", "987-654-3210"));
            farms.push_back(make_shared<GrainFarm>("Golden Grains", "789 Grain Ave", "555-555-5555"));
            farms.push_back(make_shared<DairyFarm>("Dairy Delight", "321 Dairy Blvd", "444-444-4444"));

            farms[0]->addItem(FoodItem("Carrot", 100, 10.0, FoodItem::VEGETABLE, "Fresh organic carrots"));
            farms[0]->addItem(FoodItem("Lettuce", 80, 8.0, FoodItem::VEGETABLE, "Crisp green lettuce"));
            
            farms[1]->addItem(FoodItem("Apple", 50, 15.0, FoodItem::FRUIT, "Crisp red apples"));
            farms[1]->addItem(FoodItem("Banana", 60, 12.0, FoodItem::FRUIT, "Fresh yellow bananas"));
            
            farms[2]->addItem(FoodItem("Wheat", 200, 20.0, FoodItem::GRAIN, "High-quality wheat grains"));
            farms[2]->addItem(FoodItem("Rice", 150, 18.0, FoodItem::GRAIN, "Premium white rice"));
            
            farms[3]->addItem(FoodItem("Milk", 150, 25.0, FoodItem::DAIRY, "Fresh cow's milk"));
            farms[3]->addItem(FoodItem("Cheese", 30, 35.0, FoodItem::DAIRY, "Artisan cheese"));

            saveFarms();
            cout << "[INFO] Sample farms created and saved successfully." << endl;
        } catch (const exception& e) {
            cerr << "Error creating sample farms: " << e.what() << endl;
        }
    }

    void loadCustomers() {
        try {
            ifstream infile("customers.txt");
            if (!infile.is_open()) return;

            string line;
            while (getline(infile, line)) {
                if (line.empty()) continue;

                istringstream ss(line);
                string prefix, name, email, phone;

                getline(ss, prefix, '|');
                if (prefix == "Customer") {
                    getline(ss, name, '|');
                    getline(ss, email, '|');
                    getline(ss, phone);

                    auto customer = make_shared<Customer>(name, email, phone);
                    customers[email] = customer;
                }
            }
            infile.close();
            cout << "[INFO] Loaded " << customers.size() << " customers successfully." << endl;
        } catch (const exception& e) {
            cerr << "Error loading customers: " << e.what() << endl;
        }
    }

    void saveAllData() {
        try {
            saveFarms();
            saveCustomers();
            cout << "All data saved successfully!" << endl;
        } catch (const exception& e) {
            cerr << "Error saving data: " << e.what() << endl;
        }
    }

    void saveFarms() {
        try {
            ofstream outfile("farms.txt", ios::trunc);
            if (!outfile.is_open()) {
                throw runtime_error("Failed to open farms.txt for writing");
            }

            for (const auto& farm : farms) {
                farm->saveToFile(outfile);
                for (size_t i = 0; i < farm->getInventorySize(); ++i) {
                    const FoodItem* item = farm->getItem(i);
                    if (item) {
                        outfile << item->getName() << ","
                                << item->getQuantity() << ","
                                << fixed << setprecision(2) << item->getPrice() << ","
                                << static_cast<int>(item->getCategory()) << ","
                                << item->getDescription() << endl;
                    }
                }
                outfile << "END" << endl;
            }

            outfile.close();
            cout << "[INFO] Farms data saved successfully." << endl;
        } catch (const exception& e) {
            cerr << "Error saving farms: " << e.what() << endl;
        }
    }

    void saveCustomers() {
        try {
            ofstream outfile("customers.txt", ios::trunc);
            if (!outfile.is_open()) {
                throw runtime_error("Failed to open customers.txt for writing");
            }

            for (const auto& pair : customers) {
                const auto& customer = pair.second;
                outfile << "Customer|" << customer->getName() << "|" 
                        << customer->getEmail() << "|" << customer->getPhoneNumber() << endl;
            }

            outfile.close();
            cout << "[INFO] Customers data saved successfully." << endl;
        } catch (const exception& e) {
            cerr << "Error saving customers: " << e.what() << endl;
        }
    }
};

int main() {
    try {
        OrderingSystem system;
        system.run();
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
