#ifndef BOOKSTORE_2025_LOG_H
#define BOOKSTORE_2025_LOG_H
#include "MemoryRiver.h"
#include "Account.h"
#include <string>

// 财务日志
struct FinanceLog {
    double amount;         // 金额（正为收入，负为支出）
    int index;       // 第几笔交易（从1开始）

    FinanceLog() : amount(0), index(0) {}
    FinanceLog(double amt, long long idx) : amount(amt), index(idx) {}
};

// 操作日志
struct OperationLog {
    char UserID[31];       // 用户ID
    char operation[151];    // 操作内容（150字符 + '\0'）
    int index;       // 操作序号

    OperationLog() : index(0) {
        std::memset(UserID, 0, sizeof(UserID));
        std::memset(operation, 0, sizeof(operation));
    }

    OperationLog(const std::string& uid, const std::string& op, int idx)
        : index(idx) {
        std::strncpy(UserID, uid.c_str(), 30);
        UserID[30] = '\0';
        std::strncpy(operation, op.c_str(), 150);
        operation[150] = '\0';
    }
};

class LogSystem {
private:
    MemoryRiver<FinanceLog, 3> financeStorage;    // 财务日志存储
    MemoryRiver<OperationLog> operationStorage; // 操作日志存储
    AccountSystem* accountSystem;

    // 统计数据
    int finance_count;      // 财务记录总数
    int operation_count;    // 操作记录总数
    double total_income;          // 总收入
    double total_expense;         // 总支出

public:
    LogSystem(AccountSystem* a);
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