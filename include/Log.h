#ifndef BOOKSTORE_2025_LOG_H
#define BOOKSTORE_2025_LOG_H
#include "Storage.h"
#include "Account.h"

// 财务日志
struct FinanceLog {
    double amount;         // 金额（正为收入，负为支出）
    long long index;       // 第几笔交易
};

// 员工日志
struct EmployeeLog {
    char UserID[31];       // 员工ID
    char operation[150];    // 日志内容
};

struct OperationLog {
    char UserID[31];
    char operation[150];
    long long index;    // 操作序号（第几次操作）
};

class LogSystem {
private:
    BlockList<FinanceLog> finance_logs;    // 财务日志存储
    BlockList<EmployeeLog> employee_logs;  // 员工日志存储
    BlockList<OperationLog> operation_logs; // 操作日志存储
    double total_income;
    double total_expense;
    long long finance_count;

public:
    LogSystem();
    ~LogSystem();

    // 记录交易（++finance_count）
    void recordFinance(double amount);

    // 记录员工工作
    void recordEmployee(const string& UserID, const string& operation);

    // 记录操作
    void recordOperation();

    // count = -1 输出交易总额；count = 0 输出空行
    // Count 大于历史交易总笔数时操作失败
    // {7}
    void showFinance(int count = -1);

    // {7}
    void printFinanceReport();
    void printEmployeeReport();
    void printLog();
};
#endif //BOOKSTORE_2025_LOG_H