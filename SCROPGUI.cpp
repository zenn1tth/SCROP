#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm> // for std::max and std::min
#include <cstring>   // for strcpy

using namespace std;

// Replace std::clamp for C++11 compatibility
int clampInt(int value, int minVal, int maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}


struct User {
    string name;
    string email;
    string phone;
    bool loggedIn = false;
};

struct Item {
    string name;
    string description;
    string category;
    int quantity;
    float price;
};

struct Discount {
    string type;
    float amount;
};

struct CartEntry {
    Item item;
    int quantity;
    string instructions;
    Discount discount;
    string farmName;
};

struct Farm {
    string name;
    string type;
    string address;
    string phone;
    vector<Item> items;
};


bool showLoginScreen = true;
vector<CartEntry> cart;
vector<Farm> farms;
bool modalVisible = false;
int modalFarmIndex = -1;
int modalItemIndex = -1;
string selectedPage = "Browse";
vector<string> orderHistory;


void renderLoginScreen() {
    ImGui::Begin("SCROP - Login");

    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("🌾 SCROP: Sustainable Crop Ordering Platform").x) * 0.5f);
    ImGui::Text("🌾 SCROP: Sustainable Crop Ordering Platform");


    static char name[64] = "";
    static char email[64] = "";
    static char phone[64] = "";

    ImGui::InputText("Full Name", name, IM_ARRAYSIZE(name));
    ImGui::InputText("Email", email, IM_ARRAYSIZE(email));
    ImGui::InputText("Phone", phone, IM_ARRAYSIZE(phone));

    if (ImGui::Button("Login / Register", ImVec2(-FLT_MIN, 0))) {
        if (strlen(name) > 0 && strlen(email) > 0 && strlen(phone) > 0) {
            currentUser.name = name;
            currentUser.email = email;
            currentUser.phone = phone;
            currentUser.loggedIn = true;
            showLoginScreen = false;
        }
    }

    ImGui::End();
}

void renderHeader() {
    ImGui::Begin("SCROP Navigation", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("\xF0\x9F\x8F\xAA Browse")) selectedPage = "Browse";
    ImGui::SameLine();
    if (ImGui::Button("\xF0\x9F\x9B\x92 Cart")) selectedPage = "Cart";
    ImGui::SameLine();
    if (ImGui::Button("\xF0\x9F\x91\xA4 Profile")) selectedPage = "Profile";
    ImGui::SameLine();
    if (ImGui::Button("\xF0\x9F\x93\x8B History")) selectedPage = "History";
    ImGui::SameLine();
    if (ImGui::Button("\xF0\x9F\x9A\xAA Logout")) {
        currentUser.loggedIn = false;
        showLoginScreen = true;
        selectedPage = "Browse";
    }

    ImGui::End();
}

void renderBrowseFarms() {
    ImGui::Begin("\xF0\x9F\x8F\xA6 Browse Farms");

    if (modalVisible) {
        const Item& modalItem = farms[modalFarmIndex].items[modalItemIndex];
        ImGui::OpenPopup("Add to Cart");
        if (ImGui::BeginPopupModal("Add to Cart", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s - %.2f PHP", modalItem.name.c_str(), modalItem.price);
            static int quantity = 1;
            ImGui::InputInt("Quantity", &quantity);
            quantity = clampInt(quantity, 1, modalItem.quantity); // Fixed: Use clampInt instead of std::clamp
            static char instructions[128] = "";
            ImGui::InputTextMultiline("Instructions", instructions, IM_ARRAYSIZE(instructions));

            if (ImGui::Button("Confirm")) {
                Discount d;
                float r = (float)rand() / RAND_MAX;
                if (r < 0.33f) {
                    int percent = 5 + rand() % 16;
                    d.type = "Percentage (" + to_string(percent) + "%)";
                    d.amount = quantity * modalItem.price * (percent / 100.0f);
                }
                else if (r < 0.66f && quantity >= 3) {
                    int perItemOff = 2 + rand() % 8;
                    d.type = "Bulk (\xE2\x82\xB1" + to_string(perItemOff) + " off/unit)";
                    d.amount = quantity * perItemOff;
                }
                else {
                    d.type = "None";
                    d.amount = 0;
                }

                CartEntry entry{ modalItem, quantity, instructions, d, farms[modalFarmIndex].name };
                cart.push_back(entry);
                farms[modalFarmIndex].items[modalItemIndex].quantity -= quantity;
                modalVisible = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                modalVisible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    for (size_t i = 0; i < farms.size(); ++i) {
        const Farm& farm = farms[i];
        if (ImGui::CollapsingHeader((farm.name + " - " + farm.type).c_str())) {
            ImGui::Text("Address: %s", farm.address.c_str());
            ImGui::Text("Phone: %s", farm.phone.c_str());
            ImGui::Separator();
            ImGui::Text("Available Items:");

            for (size_t j = 0; j < farm.items.size(); ++j) {
                const Item& item = farm.items[j];
                ImGui::PushID(j);
                ImGui::BulletText("%s (%.2f PHP | %d units)", item.name.c_str(), item.price, item.quantity);
                ImGui::SameLine();
                if (ImGui::Button("Add to Cart")) {
                    modalFarmIndex = i;
                    modalItemIndex = j;
                    modalVisible = true;
                }
                ImGui::PopID();
            }
        }
    }

    ImGui::End();
}

void renderCart() {
    ImGui::Begin("\xF0\x9F\x9B\x92 Cart");
    if (cart.empty()) {
        ImGui::Text("Your cart is empty.");
        ImGui::End();
        return;
    }

    float total = 0, totalDiscount = 0;
    for (size_t i = 0; i < cart.size(); ++i) {
        CartEntry& entry = cart[i];
        ImGui::PushID(i);
        ImGui::Separator();
        ImGui::Text("%s from %s", entry.item.name.c_str(), entry.farmName.c_str());
        ImGui::Text("Quantity: %d | Price: %.2f PHP each", entry.quantity, entry.item.price);
        if (!entry.instructions.empty())
            ImGui::Text("Instructions: %s", entry.instructions.c_str());
        ImGui::Text("Discount: %s (%.2f PHP)", entry.discount.type.c_str(), entry.discount.amount);
        float subtotal = std::max(0.0f, entry.item.price * entry.quantity - entry.discount.amount);
        total += subtotal;
        totalDiscount += entry.discount.amount;
        if (ImGui::Button("Remove")) {
            cart.erase(cart.begin() + i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }

    ImGui::Separator();
    ImGui::Text("Total Discount: -%.2f PHP", totalDiscount);
    ImGui::Text("Total to Pay: %.2f PHP", total);
    static bool showCheckout = false;
    if (ImGui::Button("Proceed to Checkout")) showCheckout = true;
    ImGui::End();

    if (showCheckout) {
        ImGui::Begin("\xF0\x9F\x92\xB3 Checkout");
        static char paymentMethod[64] = "Cash";
        ImGui::InputText("Payment Method", paymentMethod, IM_ARRAYSIZE(paymentMethod));
        if (ImGui::Button("Confirm Payment")) {
            int receipt = rand() % 90000000 + 10000000;
            ImGui::OpenPopup("Receipt");
            orderHistory.push_back(string("Receipt #: ") + to_string(receipt) + ", Paid via: " + paymentMethod);
            cart.clear();
            showCheckout = false;
        }
        if (ImGui::Button("Cancel")) showCheckout = false;
        if (ImGui::BeginPopupModal("Receipt", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Thank you for supporting SCROP!");
            if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::End();
    }
}

void renderProfile() {
    ImGui::Begin("\xF0\x9F\x91\xA4 User Profile");
    static char name[64], email[64], phone[32];
    strcpy(name, currentUser.name.c_str());
    strcpy(email, currentUser.email.c_str());
    strcpy(phone, currentUser.phone.c_str());
    ImGui::InputText("Name", name, IM_ARRAYSIZE(name));
    ImGui::InputText("Email", email, IM_ARRAYSIZE(email));
    ImGui::InputText("Phone", phone, IM_ARRAYSIZE(phone));
    if (ImGui::Button("Save Changes")) {
        currentUser.name = name;
        currentUser.email = email;
        currentUser.phone = phone;
    }
    ImGui::End();
}

void renderOrderHistory() {
    ImGui::Begin("\xF0\x9F\x93\x8B Order History");
    if (orderHistory.empty()) ImGui::Text("No previous orders.");
    else for (const auto& order : orderHistory) ImGui::BulletText("%s", order.c_str());
    ImGui::End();
}


int main() {
    // Initialize random seed
    srand(static_cast<unsigned int>(time(nullptr)));

    if (!glfwInit()) return -1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "SCROP GUI", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    farms = {
        {"GreenHarvest", "Vegetable Farm", "123 Farm Road", "09171234567",
            { {"Carrot", "Fresh orange carrots", "Vegetable", 20, 25.0f},
              {"Spinach", "Organic spinach", "Vegetable", 10, 30.0f} } },
        {"Fruitopia", "Fruit Farm", "456 Orchard Lane", "09179876543",
            { {"Mango", "Sweet mangoes", "Fruit", 15, 50.0f},
              {"Banana", "Lakatan bananas", "Fruit", 25, 20.0f} } }
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (showLoginScreen) renderLoginScreen();
        else {
            renderHeader();
            if (selectedPage == "Browse") renderBrowseFarms();
            else if (selectedPage == "Cart") renderCart();
            else if (selectedPage == "Profile") renderProfile();
            else if (selectedPage == "History") renderOrderHistory();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
