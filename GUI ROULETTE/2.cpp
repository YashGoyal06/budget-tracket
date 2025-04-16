#include "raylib.h"
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

const int screenWidth = 1200;
const int screenHeight = 700;
const float pi = 3.14159265358979323846f;

struct Transaction {
    std::string date;
    std::string category;
    float amount;
    std::string description;
};

std::map<std::string, Color> categoryColors = {
    {"Food", ORANGE},
    {"Housing", BLUE},
    {"Transportation", RED},
    {"Entertainment", GREEN},
    {"Utilities", PURPLE},
    {"Healthcare", PINK},
    {"Education", YELLOW},
    {"Shopping", SKYBLUE},
    {"Savings", LIME},
    {"Other", GRAY},
    {"Income", DARKGREEN}
};

std::vector<Transaction> transactions;
std::vector<std::string> categories = {"Food", "Housing", "Transportation", "Entertainment", 
                                     "Utilities", "Healthcare", "Education", "Shopping", 
                                     "Savings", "Other", "Income"};

enum AppScreen { DASHBOARD, ADD_TRANSACTION, MONTHLY_SUMMARY };
AppScreen currentScreen = DASHBOARD;

bool showCategoryDropdown = false;
int selectedCategory = 0;
char amountInput[32] = "";
char descriptionInput[128] = "";
int currentMonth = 0;
int currentYear = 0;
Font customFont;

void DrawDashboard();
void DrawAddTransaction();
void DrawMonthlySummary();
void InitBudgetTracker();
void AddTransaction(const Transaction& transaction);
void LoadTransactionsFromCSV();
void SaveTransactionToCSV(const Transaction& transaction);
std::string GetMonthName(int month);
std::vector<Transaction> GetTransactionsForMonth(int month, int year);
std::map<std::string, float> GetCategorySummary(const std::vector<Transaction>& transactions);
float GetTotalIncome(const std::vector<Transaction>& transactions);
float GetTotalExpense(const std::vector<Transaction>& transactions);

int main() 
{
    InitWindow(screenWidth, screenHeight, "Budget Tracker");
    SetTargetFPS(60);
    
    customFont = LoadFont("Roboto-Regular.ttf");
    if (customFont.texture.id == 0) {
        customFont = GetFontDefault();
    }
    
    time_t now = time(0);
    struct tm* ltm = localtime(&now);
    currentMonth = ltm->tm_mon;
    currentYear = 1900 + ltm->tm_year;
    
    InitBudgetTracker();
    
    while (!WindowShouldClose()) 
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawRectangle(0, 0, screenWidth, 60, (Color){50, 50, 50, 255});
        DrawTextEx(customFont, "Budget Tracker", (Vector2){20, 15}, 30, 1, WHITE);
        
        Rectangle dashboardBtn = {screenWidth - 500, 10, 150, 40};
        Rectangle addTransactionBtn = {screenWidth - 330, 10, 150, 40};
        Rectangle summaryBtn = {screenWidth - 160, 10, 150, 40};
        
        if (CheckCollisionPointRec(GetMousePosition(), dashboardBtn)) {
            DrawRectangleRec(dashboardBtn, (Color){90, 90, 90, 255});
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) currentScreen = DASHBOARD;
        } else {
            DrawRectangleRec(dashboardBtn, (Color){70, 70, 70, 255});
        }
        
        if (CheckCollisionPointRec(GetMousePosition(), addTransactionBtn)) {
            DrawRectangleRec(addTransactionBtn, (Color){90, 90, 90, 255});
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) currentScreen = ADD_TRANSACTION;
        } else {
            DrawRectangleRec(addTransactionBtn, (Color){70, 70, 70, 255});
        }
        
        if (CheckCollisionPointRec(GetMousePosition(), summaryBtn)) {
            DrawRectangleRec(summaryBtn, (Color){90, 90, 90, 255});
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) currentScreen = MONTHLY_SUMMARY;
        } else {
            DrawRectangleRec(summaryBtn, (Color){70, 70, 70, 255});
        }
        
        DrawTextEx(customFont, "Dashboard", (Vector2){screenWidth - 480, 20}, 20, 1, WHITE);
        DrawTextEx(customFont, "Add Transaction", (Vector2){screenWidth - 320, 20}, 20, 1, WHITE);
        DrawTextEx(customFont, "Summary", (Vector2){screenWidth - 130, 20}, 20, 1, WHITE);
        
        switch (currentScreen) {
            case DASHBOARD:
                DrawDashboard();
                break;
            case ADD_TRANSACTION:
                DrawAddTransaction();
                break;
            case MONTHLY_SUMMARY:
                DrawMonthlySummary();
                break;
        }
        
        EndDrawing();
    }
    
    UnloadFont(customFont);
    CloseWindow();
    return 0;
}

void InitBudgetTracker() {
    LoadTransactionsFromCSV();
    
    if (transactions.empty()) {
        time_t now = time(0);
        struct tm* ltm = localtime(&now);
        int year = 1900 + ltm->tm_year;
        int month = 1 + ltm->tm_mon;
        
        std::stringstream ss;
        ss << year << "-" << std::setw(2) << std::setfill('0') << month << "-" << std::setw(2) << std::setfill('0') << 15;
        std::string currentDate = ss.str();
        
        std::vector<Transaction> sampleTransactions = {
            {"2023-01-05", "Income", 3000.0f, "Monthly Salary"},
            {"2023-01-10", "Housing", -1200.0f, "Rent"},
            {"2023-01-15", "Food", -350.0f, "Groceries"},
            {"2023-01-20", "Transportation", -120.0f, "Gas"},
            {"2023-01-25", "Entertainment", -80.0f, "Movie night"},
            {currentDate, "Income", 3000.0f, "Monthly Salary"},
            {currentDate, "Housing", -1200.0f, "Rent"},
            {currentDate, "Food", -400.0f, "Groceries"},
            {currentDate, "Utilities", -150.0f, "Electricity"},
            {currentDate, "Transportation", -200.0f, "Gas and maintenance"}
        };
        
        for (const auto& transaction : sampleTransactions) {
            AddTransaction(transaction);
        }
    }
}

void LoadTransactionsFromCSV() {
    transactions.clear();
    std::ifstream file("transactions.csv");
    if (!file.is_open()) return;
    
    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string date, category, description, amountStr;
        
        std::getline(ss, date, ',');
        std::getline(ss, category, ',');
        std::getline(ss, amountStr, ',');
        std::getline(ss, description);
        
        try {
            float amount = std::stof(amountStr);
            transactions.push_back({date, category, amount, description});
        } catch (...) {
            continue; // Skip invalid lines
        }
    }
    
    file.close();
}

void SaveTransactionToCSV(const Transaction& transaction) {
    std::ifstream fileCheck("transactions.csv");
    bool fileExists = fileCheck.good();
    fileCheck.close();
    
    std::ofstream file("transactions.csv", std::ios::app);
    if (!file.is_open()) return;
    
    if (!fileExists) {
        file << "Date,Category,Amount,Description\n";
    }
    
    file << transaction.date << "," << transaction.category << "," 
         << std::fixed << std::setprecision(2) << transaction.amount << "," 
         << transaction.description << "\n";
    file.close();
}

void AddTransaction(const Transaction& transaction) {
    transactions.push_back(transaction);
    SaveTransactionToCSV(transaction);
}

std::string GetMonthName(int month) {
    const std::string monthNames[] = {"January", "February", "March", "April", "May", "June", 
                                     "July", "August", "September", "October", "November", "December"};
    return monthNames[month];
}

std::vector<Transaction> GetTransactionsForMonth(int month, int year) {
    std::vector<Transaction> result;
    std::stringstream prefix;
    prefix << year << "-" << std::setw(2) << std::setfill('0') << (month + 1);
    std::string monthPrefix = prefix.str();
    
    for (const auto& transaction : transactions) {
        if (transaction.date.substr(0, 7) == monthPrefix) {
            result.push_back(transaction);
        }
    }
    
    return result;
}

std::map<std::string, float> GetCategorySummary(const std::vector<Transaction>& transactions) {
    std::map<std::string, float> categorySummary;
    
    for (const auto& transaction : transactions) {
        if (transaction.amount < 0) {
            categorySummary[transaction.category] += -transaction.amount;
        }
    }
    
    return categorySummary;
}

float GetTotalIncome(const std::vector<Transaction>& transactions) {
    float total = 0.0f;
    for (const auto& transaction : transactions) {
        if (transaction.amount > 0) {
            total += transaction.amount;
        }
    }
    return total;
}

float GetTotalExpense(const std::vector<Transaction>& transactions) {
    float total = 0.0f;
    for (const auto& transaction : transactions) {
        if (transaction.amount < 0) {
            total += -transaction.amount;
        }
    }
    return total;
}

void DrawDashboard() {
    std::vector<Transaction> currentMonthTransactions = GetTransactionsForMonth(currentMonth, currentYear);
    float totalIncome = GetTotalIncome(currentMonthTransactions);
    float totalExpense = GetTotalExpense(currentMonthTransactions);
    float balance = totalIncome - totalExpense;
    
    DrawTextEx(customFont, TextFormat("Dashboard - %s %d", GetMonthName(currentMonth).c_str(), currentYear), 
               (Vector2){20, 80}, 28, 1, (Color){50, 50, 50, 255});
    
    DrawRectangle(20, 130, 380, 100, (Color){173, 216, 230, 255});
    DrawRectangle(410, 130, 380, 100, (Color){255, 182, 193, 255});
    DrawRectangle(800, 130, 380, 100, (Color){144, 238, 144, 255});
    
    DrawTextEx(customFont, "INCOME", (Vector2){30, 140}, 20, 1, DARKBLUE);
    DrawTextEx(customFont, TextFormat("Rs.%.2f", totalIncome), (Vector2){30, 170}, 32, 1, DARKBLUE);
    
    DrawTextEx(customFont, "EXPENSES", (Vector2){420, 140}, 20, 1, MAROON);
    DrawTextEx(customFont, TextFormat("Rs.%.2f", totalExpense), (Vector2){420, 170}, 32, 1, MAROON);
    
    DrawTextEx(customFont, "BALANCE", (Vector2){810, 140}, 20, 1, DARKGREEN);
    DrawTextEx(customFont, TextFormat("Rs.%.2f", balance), (Vector2){810, 170}, 32, 1, DARKGREEN);
    
    DrawTextEx(customFont, "Recent Transactions", (Vector2){20, 250}, 24, 1, (Color){50, 50, 50, 255});
    DrawLine(20, 280, screenWidth - 20, 280, (Color){200, 200, 200, 255});
    
    DrawTextEx(customFont, "Date", (Vector2){30, 290}, 18, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, "Category", (Vector2){200, 290}, 18, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, "Amount", (Vector2){400, 290}, 18, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, "Description", (Vector2){550, 290}, 18, 1, (Color){50, 50, 50, 255});
    DrawLine(20, 315, screenWidth - 20, 315, (Color){200, 200, 200, 255});
    
    std::vector<Transaction> sortedTransactions = transactions;
    std::sort(sortedTransactions.begin(), sortedTransactions.end(), 
              [](const Transaction& a, const Transaction& b) { return a.date > b.date; });
    
    int y = 325;
    int count = 0;
    for (const auto& transaction : sortedTransactions) {
        if (count >= 10) break;
        
        DrawTextEx(customFont, transaction.date.c_str(), (Vector2){30, y}, 16, 1, (Color){50, 50, 50, 255});
        DrawTextEx(customFont, transaction.category.c_str(), (Vector2){200, y}, 16, 1, (Color){50, 50, 50, 255});
        
        Color amountColor = transaction.amount >= 0 ? DARKGREEN : MAROON;
        DrawTextEx(customFont, TextFormat("Rs.%.2f", fabs(transaction.amount)), (Vector2){400, y}, 16, 1, amountColor);
        
        DrawTextEx(customFont, transaction.description.c_str(), (Vector2){550, y}, 16, 1, (Color){50, 50, 50, 255});
        
        y += 30;
        count++;
    }
    
    std::map<std::string, float> categorySummary = GetCategorySummary(currentMonthTransactions);
    float totalForPie = totalExpense > 0 ? totalExpense : 1.0f;
    
    DrawTextEx(customFont, "Monthly Expense Breakdown", (Vector2){20, 650}, 20, 1, (Color){50, 50, 50, 255});
    
    int legendX = 400;
    int legendY = 650;
    int legendSize = 15;
    int legendSpacing = 25;
    int legendCol = 0;
    
    for (const auto& category : categories) {
        if (category == "Income") continue;
        
        if (categorySummary.find(category) != categorySummary.end() && categorySummary[category] > 0) {
            Color color = categoryColors[category];
            DrawRectangle(legendX + legendCol * 180, legendY, legendSize, legendSize, color);
            DrawTextEx(customFont, TextFormat("%s - Rs.%.2f", category.c_str(), categorySummary[category]), 
                       (Vector2){legendX + legendCol * 180 + legendSize + 5, legendY}, 16, 1, (Color){50, 50, 50, 255});
            
            legendY += legendSpacing;
            if (legendY > 650 + 5 * legendSpacing) {
                legendY = 650;
                legendCol++;
            }
        }
    }
}

void DrawAddTransaction() {
    DrawTextEx(customFont, "Add New Transaction", (Vector2){20, 80}, 28, 1, (Color){50, 50, 50, 255});
    
    time_t now = time(0);
    struct tm* ltm = localtime(&now);
    int day = ltm->tm_mday;
    int month = 1 + ltm->tm_mon;
    int year = 1900 + ltm->tm_year;
    std::string currentDate = TextFormat("%d-%02d-%02d", year, month, day);
    
    DrawTextEx(customFont, "Date (auto):", (Vector2){30, 140}, 18, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, currentDate.c_str(), (Vector2){200, 140}, 18, 1, (Color){50, 50, 50, 255});
    
    DrawTextEx(customFont, "Category:", (Vector2){30, 180}, 18, 1, (Color){50, 50, 50, 255});
    Rectangle categoryBox = {200, 180, 200, 30};
    DrawRectangleRec(categoryBox, (Color){200, 200, 200, 255});
    DrawTextEx(customFont, categories[selectedCategory].c_str(), (Vector2){210, 185}, 18, 1, BLACK);
    DrawTextEx(customFont, "▼", (Vector2){380, 185}, 18, 1, BLACK);
    
    if (CheckCollisionPointRec(GetMousePosition(), categoryBox) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        showCategoryDropdown = !showCategoryDropdown;
    }
    
    if (showCategoryDropdown) {
        for (size_t i = 0; i < categories.size(); i++) {
            Rectangle option = {200, 210 + i * 30.0f, 200, 30};
            bool isHovered = CheckCollisionPointRec(GetMousePosition(), option);
            
            DrawRectangleRec(option, isHovered ? (Color){180, 180, 180, 255} : (Color){200, 200, 200, 255});
            DrawTextEx(customFont, categories[i].c_str(), (Vector2){210, 215 + i * 30}, 18, 1, BLACK);
            
            if (isHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                selectedCategory = i;
                showCategoryDropdown = false;
            }
        }
    }
    
    DrawTextEx(customFont, "Amount (Rs.):", (Vector2){30, 220}, 18, 1, (Color){50, 50, 50, 255});
    Rectangle amountBox = {200, 220, 200, 30};
    DrawRectangleRec(amountBox, (Color){200, 200, 200, 255});
    DrawTextEx(customFont, amountInput, (Vector2){210, 225}, 18, 1, BLACK);
    
    static bool amountFocused = false;
    if (CheckCollisionPointRec(GetMousePosition(), amountBox) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        amountFocused = true;
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), amountBox)) {
        amountFocused = false;
    }
    
    if (amountFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 48 && key <= 57) || key == 46) {
                int len = strlen(amountInput);
                if (len < 31) {
                    amountInput[len] = (char)key;
                    amountInput[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(amountInput);
            if (len > 0) {
                amountInput[len - 1] = '\0';
            }
        }
    }
    
    DrawTextEx(customFont, "Description:", (Vector2){30, 260}, 18, 1, (Color){50, 50, 50, 255});
    Rectangle descBox = {200, 260, 400, 30};
    DrawRectangleRec(descBox, (Color){200, 200, 200, 255});
    DrawTextEx(customFont, descriptionInput, (Vector2){210, 265}, 18, 1, BLACK);
    
    static bool descFocused = false;
    if (CheckCollisionPointRec(GetMousePosition(), descBox) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        descFocused = true;
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), descBox)) {
        descFocused = false;
    }
    
    if (descFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            int len = strlen(descriptionInput);
            if (len < 127) {
                descriptionInput[len] = (char)key;
                descriptionInput[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(descriptionInput);
            if (len > 0) {
                descriptionInput[len - 1] = '\0';
            }
        }
    }
    
    Rectangle addButton = {200, 320, 200, 40};
    bool addButtonHovered = CheckCollisionPointRec(GetMousePosition(), addButton);
    
    DrawRectangleRec(addButton, addButtonHovered ? (Color){34, 139, 34, 255} : DARKGREEN);
    DrawTextEx(customFont, "Add Transaction", (Vector2){230, 330}, 20, 1, WHITE);
    
    if (addButtonHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        if (strlen(amountInput) > 0) {
            float amount = std::stof(amountInput);
            if (categories[selectedCategory] != "Income") {
                amount = -fabs(amount);
            } else {
                amount = fabs(amount);
            }
            
            std::stringstream dateStream;
            dateStream << year << "-" << std::setw(2) << std::setfill('0') << month << "-" 
                       << std::setw(2) << std::setfill('0') << day;
            
            Transaction newTransaction = {
                dateStream.str(),
                categories[selectedCategory],
                amount,
                strlen(descriptionInput) > 0 ? descriptionInput : "No description"
            };
            
            AddTransaction(newTransaction);
            
            amountInput[0] = '\0';
            descriptionInput[0] = '\0';
            
            DrawTextEx(customFont, "Transaction added successfully!", (Vector2){200, 380}, 18, 1, GREEN);
        } else {
            DrawTextEx(customFont, "Please enter an amount!", (Vector2){200, 380}, 18, 1, RED);
        }
    }
    
    DrawTextEx(customFont, "Note: For income, select the 'Income' category and enter a positive amount.", 
               (Vector2){30, 400}, 16, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, "For expenses, select the appropriate category and enter the amount.", 
               (Vector2){30, 425}, 16, 1, (Color){50, 50, 50, 255});
}

void DrawMonthlySummary() {
    DrawTextEx(customFont, TextFormat("Monthly Summary - %s %d", GetMonthName(currentMonth).c_str(), currentYear), 
               (Vector2){20, 80}, 28, 1, (Color){50, 50, 50, 255});
    
    Rectangle prevMonth = {20, 130, 30, 30};
    Rectangle nextMonth = {220, 130, 30, 30};
    
    DrawRectangleRec(prevMonth, (Color){200, 200, 200, 255});
    DrawTextEx(customFont, "<", (Vector2){30, 135}, 18, 1, BLACK);
    
    DrawTextEx(customFont, TextFormat("%s %d", GetMonthName(currentMonth).c_str(), currentYear), 
               (Vector2){60, 135}, 18, 1, BLACK);
    
    DrawRectangleRec(nextMonth, (Color){200, 200, 200, 255});
    DrawTextEx(customFont, ">", (Vector2){230, 135}, 18, 1, BLACK);
    
    if (CheckCollisionPointRec(GetMousePosition(), prevMonth) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        currentMonth--;
        if (currentMonth < 0) {
            currentMonth = 11;
            currentYear--;
        }
    }
    
    if (CheckCollisionPointRec(GetMousePosition(), nextMonth) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        currentMonth++;
        if (currentMonth > 11) {
            currentMonth = 0;
            currentYear++;
        }
    }
    
    std::vector<Transaction> monthTransactions = GetTransactionsForMonth(currentMonth, currentYear);
    float totalIncome = GetTotalIncome(monthTransactions);
    float totalExpense = GetTotalExpense(monthTransactions);
    float balance = totalIncome - totalExpense;
    
    DrawTextEx(customFont, "Income:", (Vector2){20, 180}, 18, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, TextFormat("Rs.%.2f", totalIncome), (Vector2){200, 180}, 18, 1, DARKGREEN);
    
    DrawTextEx(customFont, "Expenses:", (Vector2){20, 210}, 18, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, TextFormat("Rs.%.2f", totalExpense), (Vector2){200, 210}, 18, 1, MAROON);
    
    DrawTextEx(customFont, "Balance:", (Vector2){20, 240}, 18, 1, (Color){50, 50, 50, 255});
    DrawTextEx(customFont, TextFormat("Rs.%.2f", balance), (Vector2){200, 240}, 18, 1, 
               balance >= 0 ? DARKGREEN : MAROON);
    
    DrawTextEx(customFont, "Expense Breakdown:", (Vector2){20, 280}, 18, 1, (Color){50, 50, 50, 255});
    
    std::map<std::string, float> categorySummary = GetCategorySummary(monthTransactions);
    int y = 310;
    
    for (const auto& category : categories) {
        if (category == "Income") continue;
        
        if (categorySummary.find(category) != categorySummary.end() && categorySummary[category] > 0) {
            DrawRectangle(20, y, 15, 15, categoryColors[category]);
            DrawTextEx(customFont, category.c_str(), (Vector2){45, y}, 18, 1, (Color){50, 50, 50, 255});
            DrawTextEx(customFont, TextFormat("Rs.%.2f", categorySummary[category]), (Vector2){200, y}, 18, 1, (Color){50, 50, 50, 255});
            
            float percentage = (categorySummary[category] / totalExpense) * 100.0f;
            DrawTextEx(customFont, TextFormat("%.1f%%", percentage), (Vector2){300, y}, 18, 1, (Color){50, 50, 50, 255});
            
            y += 30;
        }
    }
    
    DrawTextEx(customFont, "Expense Distribution", (Vector2){600, 180}, 24, 1, (Color){50, 50, 50, 255});
    
    int centerX = 700;
    int centerY = 350;
    int radius = 150;
    
    if (totalExpense > 0) {
        float startAngle = 0.0f;
        
        for (const auto& category : categories) {
            if (category == "Income") continue;
            
            if (categorySummary.find(category) != categorySummary.end() && categorySummary[category] > 0) {
                float percentage = categorySummary[category] / totalExpense;
                float endAngle = startAngle + percentage * 2.0f * pi;
                
                DrawCircleSector(Vector2{(float)centerX, (float)centerY}, 
                                radius, 
                                startAngle * RAD2DEG, 
                                endAngle * RAD2DEG, 
                                32, 
                                categoryColors[category]);
                
                if (percentage > 0.05f) {
                    float labelAngle = startAngle + (endAngle - startAngle) / 2.0f;
                    int labelX = centerX + (radius * 0.7f) * cos(labelAngle);
                    int labelY = centerY + (radius * 0.7f) * sin(labelAngle);
                    
                    DrawTextEx(customFont, TextFormat("%.1f%%", percentage * 100.0f), 
                               (Vector2){labelX - 20, labelY}, 18, 1, WHITE);
                }
                
                startAngle = endAngle;
            }
        }
    } else {
        DrawCircleLines(centerX, centerY, radius, (Color){200, 200, 200, 255});
        DrawTextEx(customFont, "No expenses for this month", (Vector2){centerX - 120, centerY - 10}, 
                   18, 1, GRAY);
    }
}