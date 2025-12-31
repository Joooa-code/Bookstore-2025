#include "Log.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
#include <map>

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

// 记录财务交易
void LogSystem::recordFinance(double amount) {
    FinanceLog log(amount, ++finance_count);  // 先递增，然后使用
    financeStorage.write(log);

    // 更新统计
    if (amount > 0) {
        total_income += amount;
    } else {
        total_expense += (-amount);
    }
}

// 记录操作
void LogSystem::recordOperation(const std::string& UserID, const std::string& operation) {
    OperationLog log(UserID, operation, ++operation_count);
    operationStorage.write(log);
}

// 记录交易（购买或进货）
void LogSystem::recordEconomy(const std::string& ISBN, const std::string& BookName,
                                 int Quantity, double UnitPrice, double TotalAmount,
                                 const std::string& UserID) {
    // 记录财务日志
    recordFinance(TotalAmount);

    // 构建操作描述
    std::string operation;
    if (TotalAmount > 0) {
        // 购买操作
        operation = "buy ISBN=" + ISBN + " quantity=" + std::to_string(Quantity) +
                   " unit_price=" + std::to_string(UnitPrice) +
                   " total=" + std::to_string(TotalAmount);
    } else {
        // 进货操作
        operation = "import ISBN=" + ISBN + " quantity=" + std::to_string(Quantity) +
                   " cost=" + std::to_string(-TotalAmount);
    }

    // 记录操作日志
    recordOperation(UserID, operation);
}

// 显示财务信息
void LogSystem::showFinance(int count) {
    // 权限检查应该在调用此函数之前完成（需要权限7）

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
        financeStorage.read(log, (i-1) * sizeof(FinanceLog));
        if (log.amount > 0) {
            recent_income += log.amount;
        } else {
            recent_expense += (-log.amount);
        }

    }

    std::cout << std::fixed << std::setprecision(2)
              << "+ " << recent_income << " - " << recent_expense << "\n";
}

// 生成财务报表
void LogSystem::printFinanceReport() {
    std::cout << "========================= 财务报表 =========================\n";
    std::cout << "交易总笔数: " << finance_count << "\n";
    std::cout << "总收入: " << std::fixed << std::setprecision(2) << total_income << "\n";
    std::cout << "总支出: " << std::fixed << std::setprecision(2) << total_expense << "\n";
    std::cout << "净利润: " << std::fixed << std::setprecision(2)
              << (total_income - total_expense) << "\n";

    if (finance_count > 0) {
        std::cout << "\n最近10笔交易记录:\n";
        std::cout << std::setw(6) << "序号"
                  << std::setw(12) << "金额"
                  << std::setw(8) << "类型" << "\n";
        std::cout << "----------------------------------------\n";

        // 显示最近10笔交易
        int show_count = std::min(10, finance_count);
        for (long long i = finance_count; i > finance_count - show_count; i--) {
            FinanceLog log;
            financeStorage.read(log, (i-1) * sizeof(FinanceLog));
            std::cout << std::setw(6) << log.index
            << std::setw(12) << std::fixed << std::setprecision(2)
            << std::abs(log.amount);
            if (log.amount > 0) {
                std::cout << std::setw(8) << "收入" << "\n";
            } else {
                std::cout << std::setw(8) << "支出" << "\n";
            }
        }
    }
    std::cout << "=========================================================\n";
}

// 生成员工工作报告
void LogSystem::printEmployeeReport() {
    std::cout << "======================= 员工工作报告 =======================\n";

    if (operation_count == 0) {
        std::cout << "暂无员工操作记录\n";
    } else {
        // 按员工ID分组
        std::map<std::string, std::vector<OperationLog>> employee_ops;

        // 读取所有操作记录
        for (long long i = 0; i < operation_count; i++) {
            OperationLog log;
            operationStorage.read(log, i * sizeof(OperationLog));
            std::string uid(log.UserID);
            employee_ops[uid].push_back(log);
        }

        // 输出每个员工的操作记录
        for (const auto& pair : employee_ops) {
            const std::string& user_id = pair.first;
            const auto& logs = pair.second;

            std::cout << "\n员工: " << user_id << "\n";
            std::cout << "操作记录总数: " << logs.size() << "\n";

            if (!logs.empty()) {
                // 按操作序号排序（新->旧）
                auto sorted_logs = logs;
                std::sort(sorted_logs.begin(), sorted_logs.end(),
                    [](const OperationLog& a, const OperationLog& b) {
                        return a.index > b.index;  // 按序号降序
                    });

                std::cout << "最近5条操作记录:\n";
                int show_count = std::min(5, static_cast<int>(sorted_logs.size()));
                for (int i = 0; i < show_count; i++) {
                    const auto& log = sorted_logs[i];
                    std::cout << "  " << (i+1) << ". [操作#" << log.index << "] "
                              << log.operation << "\n";
                }

                if (sorted_logs.size() > 5) {
                    std::cout << "  ... 还有 " << (sorted_logs.size() - 5) << " 条记录\n";
                }
            }

        }
        std::cout << "=========================================================\n";
    }
}
// 生成完整日志报告（对应log指令）
void LogSystem::printLog() {
    std::cout << "========================= 系统日志 =========================\n";

    if (operation_count == 0) {
        std::cout << "暂无系统日志\n";
    } else {
        std::cout << "操作记录总数: " << operation_count << "\n\n";

        // 读取最近的操作记录
        int show_count = std::min(20, operation_count);

        for (long long i = operation_count; i > operation_count - show_count; i--) {
            OperationLog log;
            operationStorage.read(log, (i-1) * sizeof(OperationLog));
                std::cout << "操作#" << log.index
                          << " 用户:" << log.UserID
                          << " 操作:" << log.operation << "\n";
            }

        if (operation_count > show_count) {
            std::cout << "... 还有 " << (operation_count - show_count) << " 条记录\n";
        }
    }
    std::cout << "=========================================================\n";
}