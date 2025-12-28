#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "Account.h"
#include "Book.h"
#include "Log.h"

using namespace std;

// 去除字符串两端的空白字符
string Trim(const string& str) {
    size_t first = str.find_first_not_of(' ');  // 第一个不是空格的值
    if (string::npos == first) {
        return "";
       }
    size_t last = str.find_last_not_of(' ');  // 最后一个不是空格的值
    return str.substr(first, (last - first + 1));
}

// 分割命令行参数
vector<string> Split(const string& line) {
    vector<string> tokens;
    string token;

    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];

        if (c == ' ') {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        }
        else {
            token += c;
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

// 检查字符串是否只包含数字
bool IsDigits(const string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// 处理账户指令
void ProcessAccountCommand(AccountSystem* accountSystem, const vector<string>& tokens) {
    if (tokens.empty()) return;

    string cmd = tokens[0];

    if (cmd == "su") {
        if (tokens.size() < 2 || tokens.size() > 3) {
            cout << "Invalid\n";
            return;
        }
        string userID = Trim(tokens[1]);
        string password = (tokens.size() == 3) ? Trim(tokens[2]) : "";
        accountSystem->su(userID, password);
    }
    else if (cmd == "logout") {
        if (tokens.size() != 1) {
            cout << "Invalid\n";
            return;
        }
        accountSystem->logout();
    }
    else if (cmd == "register") {
        if (tokens.size() != 4) {
            cout << "Invalid\n";
            return;
        }
        string userID = Trim(tokens[1]);
        string password = Trim(tokens[2]);
        string username = Trim(tokens[3]);
        accountSystem->regis(userID, password, username);
    }
    else if (cmd == "passwd") {
        if (tokens.size() < 3 || tokens.size() > 4) {
            cout << "Invalid\n";
            return;
        }
        string userID = Trim(tokens[1]);
        if (tokens.size() == 3) {
            // 格式: passwd [UserID] [NewPassword]
            string newPassword = Trim(tokens[2]);
            accountSystem->passwd(userID, newPassword, "");
        } else {
            // 格式: passwd [UserID] [CurrentPassword] [NewPassword]
            string currentPassword = Trim(tokens[2]);
            string newPassword = Trim(tokens[3]);
            accountSystem->passwd(userID, newPassword, currentPassword);
        }
    }
    else if (cmd == "useradd") {
        if (tokens.size() != 5) {
            cout << "Invalid\n";
            return;
        }
        string userID = Trim(tokens[1]);
        string password = Trim(tokens[2]);

        // 检查权限格式
        string priv_str = Trim(tokens[3]);
        if (!IsDigits(priv_str) || priv_str.length() != 1) {
            cout << "Invalid\n";
            return;
        }
        int privilege = stoi(priv_str);
        // 检查权限值是否合法
        if (privilege != 1 && privilege != 3) {
            cout << "Invalid\n";
            return;
        }

        string username = Trim(tokens[4]);
        accountSystem->useradd(userID, password, privilege, username);
    }
    else if (cmd == "delete") {
        if (tokens.size() != 2) {
            cout << "Invalid\n";
            return;
        }
        string userID = Trim(tokens[1]);
        accountSystem->deleteAccount(userID);
    }
    else {
        cout << "Invalid\n";
    }
}

// 处理图书指令
void ProcessBookCommand(BookSystem* bookSystem, const vector<string>& tokens) {
    if (tokens.empty()) {
        cout << "Invalid\n";
        return;
    }

    string cmd = tokens[0];

    if (cmd == "show") {
        if (tokens.size() == 1) {
            // 显示所有图书
            bookSystem->show();
        }
        else if (tokens.size() == 2) {
            string b_line = Trim(tokens[1]);

            if (b_line.find("-ISBN=") == 0) {
                // 按ISBN查询
                string isbn = b_line.substr(6);
                if (isbn.empty()) {
                    cout << "Invalid\n";
                    return;
                }
                bookSystem->show("ISBN", isbn);
            }
            else if (b_line.find("-name=") == 0) {
                // 按书名查询
                string name = b_line.substr(6);
                if (name.length() < 3) {  // 双引号内不能无内容
                    cout << "Invalid\n";
                    return;
                }
                name = name.substr(1, name.length() - 2);
                bookSystem->show("name", name);
            }
            else if (b_line.find("-author=") == 0) {
                // 按作者查询
                string author = b_line.substr(8);
                if (author.length() < 3) {  // 双引号内不能无内容
                    cout << "Invalid\n";
                    return;
                }
                author = author.substr(1, author.length() - 2);
                bookSystem->show("author", author);
            }
            else if (b_line.find("-keyword=") == 0) {
                // 按关键词查询
                string keyword = b_line.substr(9);
                if (keyword.length() < 3) {  // 双引号内不能无内容
                    cout << "Invalid\n";
                    return;
                }
                keyword = keyword.substr(1, keyword.length() - 2);
                bookSystem->show("keyword", keyword);
            }
            else {
                cout << "Invalid\n";
            }
        }
        else {
            cout << "Invalid\n";
        }
    }
    else if (cmd == "buy") {
        if (tokens.size() != 3) {
            cout << "Invalid\n";
            return;
        }
        string isbn = Trim(tokens[1]);
        string quantity_str = Trim(tokens[2]);

        if (!IsDigits(quantity_str) || quantity_str.empty()) {
            cout << "Invalid\n";
            return;
        }
        long quantity = stol(quantity_str);
        // 检查大小范围
        if (quantity > 2'147'483'647) {
            cout << "Invalid\n";
            return;
        }
        bookSystem->buy(isbn, (int)quantity);
    }
    else if (cmd == "select") {
        if (tokens.size() != 2) {
            cout << "Invalid\n";
            return;
        }
        string isbn = Trim(tokens[1]);
        if (isbn.empty()) {
            cout << "Invalid\n";
            return;
        }
        bookSystem->select(isbn);
    }
    else if (cmd == "modify") {
        if (tokens.size() < 2) {
            cout << "Invalid\n";
            return;
        }

        // 拼接所有参数
        string line = "";
        for (size_t i = 1; i < tokens.size(); i++) {
            if (i > 1) line += " ";
            line += tokens[i];
        }

        bookSystem->modify(line);
    }
    else if (cmd == "import") {
        if (tokens.size() != 3) {
            cout << "Invalid\n";
            return;
        }
        string quantity_str = Trim(tokens[1]);
        string cost_str = Trim(tokens[2]);

        // 检查Quantity
        if (!IsDigits(quantity_str) || quantity_str.empty()) {
            cout << "Invalid\n";
            return;
        }
        long quantity = stol(quantity_str);
        if (quantity > 2'147'483'647) {
            cout << "Invalid\n";
            return;
        }

        // 检查TotalCost
        if (cost_str.length() > 13) {
            cout << "Invalid\n";
            return;  // 长度不能超过13
        }
        bool valid_cost = true;
        bool dot_found = false;
        for (char c : cost_str) {
            if (c == '.') {
                if (dot_found) {
                    valid_cost = false;
                    break;
                }
                dot_found = true;
            }
            else if (!isdigit(c)) {
                valid_cost = false;
                break;
            }
        }
        if (!valid_cost || cost_str.empty()) {
            cout << "Invalid\n";
            return;
        }

        double total_cost = stod(cost_str);
        if (quantity <= 0 || total_cost <= 0) {
            cout << "Invalid\n";
            return;
        }

        bookSystem->import((int)quantity, total_cost);
    }
    else {
        cout << "Invalid\n";
    }
}

// 处理日志指令
void ProcessLogCommand(LogSystem* logSystem, AccountSystem* accountSystem, const vector<string>& tokens) {
    if (tokens.empty()) {
        cout << "Invalid\n";
        return;
    }

    string cmd = tokens[0];

    if (cmd == "show") {
        if (tokens.size() == 2) {
            if (tokens[1] == "finance") {
                // show finance
                logSystem->showFinance(-1);
            } else {
                cout << "Invalid\n";
            }
        }
        else if (tokens.size() == 3) {
            if (tokens[1] == "finance") {
                // show finance [Count]
                string count_str = Trim(tokens[2]);
                if (!IsDigits(count_str) || count_str.empty()) {
                    cout << "Invalid\n";
                    return;
                }
                long count = stol(count_str);
                // 检查大小范围
                if (count > 2'147'483'647) {
                    cout << "Invalid\n";
                    return;
                }
                logSystem->showFinance((int)count);
            } else {
                cout << "Invalid\n";
            }
        }
        else {
            cout << "Invalid\n";
        }
    }
}

int main() {
    // 设置浮点数输出格式
    cout << fixed << setprecision(2);

    // 初始化系统
    AccountSystem* accountSystem = new AccountSystem();
    LogSystem* financeSystem = new LogSystem(accountSystem);
    BookSystem* bookSystem = new BookSystem(accountSystem, financeSystem);

    string line;

    // 主循环
    while (getline(cin, line)) {
        // 去除首尾空格
        line = Trim(line);

        if (line.empty()) {
            continue;  // 空行，继续
        }

        // 检查退出指令
        if (line == "quit" || line == "exit") {
            break;
        }

        // 分割命令行
        vector<string> tokens = Split(line);
        if (tokens.empty()) {
            continue;
        }

        string cmd = tokens[0];

        // 根据指令类型分发
        if (cmd == "su" || cmd == "logout" || cmd == "register" ||
            cmd == "passwd" || cmd == "useradd" || cmd == "delete") {
            ProcessAccountCommand(accountSystem, tokens);
            }
        else if (cmd == "show" || cmd == "buy" || cmd == "select" ||
                 cmd == "modify" || cmd == "import") {
            ProcessBookCommand(bookSystem, tokens);
                 }
        else if (cmd == "log" || cmd == "report") {
            ProcessLogCommand(financeSystem, accountSystem, tokens);
        }
        else {
            cout << "Invalid\n";
        }
    }

    // 清理资源
    delete bookSystem;
    delete financeSystem;
    delete accountSystem;

    return 0;
}