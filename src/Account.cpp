#include "Account.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cctype>

void AccountSystem::init_root() {
    Account root;
    strcpy(root.UserID, "root");
    strcpy(root.Password, "sjtu");
    strcpy(root.Username, "Adiministrator");
    root.Privilege = 7;
    // 写入文件
    int pos = accountStorage.write(root);
    // 建立索引
    accountIndex.insert(root.UserID, pos);
}

bool AccountSystem::ID_pw_check(const std::string& s) const {
    // 长度检查
    if (s.empty() || s.length() > 30) return false;

    // 数字，字母，下划线
    for (char c: s) {
        if (!std::isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

static bool name_check(const std::string& name) {
    // 长度检查
    if (name.empty() || name.length() > 30) return false;

    // 不含不可见字符
    for (char c:name) {
        if (c < 32 || c == 127) {
            return false;
        }
    }
    return true;
}

bool AccountSystem::priv_check(int priv) const {
    int cur = get_curpriv();
    if (cur < priv) {
        return false;
    }
    else return true;
}

int AccountSystem::get_curpriv() const {
    if (loginStack.empty()) return 0; // 游客权限
    return loginStack.back().privilege;
}

// 获取当前用户（登录栈末尾）选中图书，若无返回""
std::string AccountSystem::get_selected_ISBN() const {
    if (loginStack.empty()) return "";
    return loginStack.back().selected_ISBN;
}

// 为当前用户设置选中图书
void AccountSystem::set_selected_ISBN(const std::string& isbn) {
    if (!loginStack.empty()) {
        loginStack.back().selected_ISBN = isbn;
    }
}

// 检查用户是否存在
bool AccountSystem::user_exist(const char* UserID) {
    if (accountIndex.find(UserID).empty()) {
        return false;
    }
    return true;
}

// 获取用户信息
bool AccountSystem::get_user_info(const std::string& UserID, Account& account) {
    auto result = accountIndex.find(UserID.c_str());
    if (result.empty()) {
        return false;
    }
    int pos = result[0];
    accountStorage.read(account, pos);
    return true;
}

AccountSystem::AccountSystem()
    : accountIndex("account_index.dat") {
    accountStorage.initialise("account_data.dat");

    // 检查是否需要初始化根用户
    if (!user_exist("root")) {
        init_root();
    }
}

AccountSystem::~AccountSystem() = default;

// 登录：若成功则修改登录栈
// {0}
void AccountSystem::su(const string& UserID, const string& Password ) {
    // 验证ID
    if (!ID_pw_check(UserID)) {
        std::cout << "Invalid\n";
        return;
    }
    // 检查用户是否存在
    Account account;
    if (!get_user_info(UserID, account)) {
        std::cout << "Invalid\n";
        return;
    }

    int cur_priv = get_curpriv();
    // 检查是否需要密码
    if (Password.empty()) {
        // 无密码：要求当前权限高于要登录的账户
        if (cur_priv <= account.Privilege) {
            std::cout << "Invalid\n";
            return;
        }
    } else {
        // 有密码：检查密码是否正确
        if (strcmp(account.Password, Password.c_str()) != 0) {
            std::cout << "Invalid\n";
            return;
        }
    }
    // 登录成功，加入登录栈
    LoginRecord record;
    record.UserID = UserID;
    record.privilege = account.Privilege;
    record.selected_ISBN = "";
    loginStack.push_back(record);
}

// 撤销最后一次成功执行的 su 指令效果
void AccountSystem::logout() {
    if (loginStack.empty()) {
        std::cout << "Invalid\n";
        return;
    }  // 失败

    loginStack.pop_back();
}

// 注册权限等级为 {1} 的新账户
void AccountSystem::regis(const string& UserID, const string& Password, const string& Username) {
    // 参数检查
    if (!ID_pw_check(UserID) || !ID_pw_check(Password) || !name_check(Username)) {
        std::cout << "Invalid\n";
        return;
    }

    // 检查用户是否已存在
    if (user_exist(UserID.c_str())) {
        std::cout << "Invalid\n";
        return;
    }

    // 创建新账户
    Account new_account;
    strcpy(new_account.UserID, UserID.c_str());
    strcpy(new_account.Password, Password.c_str());
    strcpy(new_account.Username, Username.c_str());
    new_account.Privilege = 1; // 注册账户权限固定为1

    // 存储账户
    int pos = accountStorage.write(new_account);
    accountIndex.insert(UserID.c_str(), pos);
}

// 修改指定帐户的密码
void AccountSystem::passwd(const string& UserID, const string& NewPassword, const string& CurrentPassword ) {
    // 参数检查
    if (!ID_pw_check(UserID) || !ID_pw_check(NewPassword) ||
        (!CurrentPassword.empty() && !ID_pw_check(CurrentPassword))) {
        std::cout << "Invalid\n";
        return;
        }

    // 检查用户是否存在
    Account account;
    if (!get_user_info(UserID, account)) {
        std::cout << "Invalid\n";
        return;
    }

    int cur_priv = get_curpriv();

    // 检查当前密码
    if (CurrentPassword.empty()) {
        // 无当前密码：要求当前权限为7
        if (cur_priv != 7) {
            std::cout << "Invalid\n";
            return;
        }
    } else {
        // 有当前密码：检查是否正确
        if (strcmp(account.Password, CurrentPassword.c_str()) != 0) {
            std::cout << "Invalid\n";
            return;
        }
    }

    // 修改密码
    strcpy(account.Password, NewPassword.c_str());

    // 更新存储
    auto result = accountIndex.find(UserID.c_str());
    int pos = result[0];
    accountStorage.update(account, pos);
}

// 创建账户
void AccountSystem::useradd(const string& UserID, const string& Password, int Privilege, const string& Username) {
    // 权限检查
    if (!priv_check(3)) {
        std::cout << "Invalid\n";
        return;
    }

    // 参数检查
    if (!ID_pw_check(UserID) || !ID_pw_check(Password) || !name_check(Username)) {
        std::cout << "Invalid\n";
        return;
    }
    // 检查权限值是否合法
    if (Privilege != 1 && Privilege != 3 && Privilege != 7) {
        std::cout << "Invalid\n";
        return;
    }

    // 检查当前权限是否高于要创建的账户权限
    int cur_priv = get_curpriv();
    if (cur_priv <= Privilege) {
        std::cout << "Invalid\n";
        return;
    }

    // 检查用户是否已存在
    if (user_exist(UserID.c_str())) {
        std::cout << "Invalid\n";
        return;
    }

    // 创建新账户
    Account new_account;
    strcpy(new_account.UserID, UserID.c_str());
    strcpy(new_account.Password, Password.c_str());
    strcpy(new_account.Username, Username.c_str());
    new_account.Privilege = Privilege;
    // 存储账户
    int pos = accountStorage.write(new_account);
    accountIndex.insert(UserID.c_str(), pos);
}

// 删除账户
void AccountSystem::deleteAccount(const string& UserID) {
    // 权限检查
    if (!priv_check(7)) {
        std::cout << "Invalid\n";
        return;
    }
    // 参数检查
    if (!ID_pw_check(UserID)) {
        std::cout << "Invalid\n";
        return;
    }
    // 检查要删除的用户是否存在
    if (!user_exist(UserID.c_str())) {
        std::cout << "Invalid\n";
        return;
    }
    // 检查是否正在尝试删除root账户
    if (UserID == "root") {
        std::cout << "Invalid\n";
        return;
    }
    // 检查要删除的用户是否已登录
    for (const auto& record : loginStack) {
        if (record.UserID == UserID) {
            std::cout << "Invalid\n";
            return;
        }
    }
    // 删除账户
    auto result = accountIndex.find(UserID.c_str());
    int pos = result[0];
    accountIndex.remove(UserID.c_str(), pos);

    // 存储中的空间回收待实现
}
