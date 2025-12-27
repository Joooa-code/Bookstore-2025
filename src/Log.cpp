#include "Log.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>

// 构造函数
LogSystem::LogSystem(AccountSystem* a)
    : finance_count(0), operation_count(0), total_income(0.0), total_expense(0.0), accountSystem(a) {
    // 初始化存储
    financeStorage.initialise("finance_log.dat");
    operationStorage.initialise("operation_log.dat");

    // 读取文件信息
    double tem;
    financeStorage.get_info(tem, 1);
    finance_count = static_cast<int>(tem);
    financeStorage.get_info(total_income, 2);
    financeStorage.get_info(total_expense, 3);
    operationStorage.get_info(tem, 1);
    operation_count = static_cast<int>(tem);
}

// 析构函数
LogSystem::~LogSystem() {
    // 保存统计数据到文件头部
    financeStorage.write_info(finance_count, 1);
    financeStorage.write_info(total_income, 2);
    financeStorage.write_info(total_expense, 3);
    operationStorage.write_info(operation_count, 1);
}

// 记录交易（++finance_count）
void LogSystem::recordFinance(double amount) {
    FinanceLog log(amount, ++finance_count);
    financeStorage.write(log);

    // 更新收支统计
    if (amount > 0) {
        total_income += amount;
    } else {
        total_expense += (-amount);
    }
}

// 显示财务信息
void LogSystem::showFinance(int count) {
    // 权限检查
    if (accountSystem->get_curpriv() < 7) {
        std::cout << "Invalid\n";
        return;
    }

    if (count == -1) {
        // 输出所有交易总额
        std::cout << std::fixed << std::setprecision(2)
                  << "+ " << total_income << " - " << total_expense << "\n";
        return;
    }

    if (count == 0) {
        // 输出空行
        std::cout << "\n";
        return;
    }

    // 检查count是否大于历史交易总笔数
    if (count > finance_count) {
        std::cout << "Invalid\n";
        return;
    }

    // 计算最近count笔交易的收入和支出
    double recent_income = 0.0;
    double recent_expense = 0.0;

    // 从后往前读取最近的count笔交易
    for (long long i = finance_count; i > finance_count - count; i--) {
        FinanceLog log;
        financeStorage.read(log, (i-1) * sizeof(FinanceLog) + sizeof(double) * 3);
            if (log.amount > 0) {
                recent_income += log.amount;
            } else {
                recent_expense += (-log.amount);
            }

    }

    std::cout << std::fixed << std::setprecision(2)
              << "+ " << recent_income << " - " << recent_expense << "\n";
}
