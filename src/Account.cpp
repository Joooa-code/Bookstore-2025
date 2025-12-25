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
    return priv == 1 || priv == 3 || priv == 7;
}

std::string AccountSystem::get_curID() const {
    if (loginStack.empty()) return "";
    return loginStack.back().UserID;
}

int AccountSystem::get_curpriv() const {
    if (loginStack.empty()) return 0; // 游客权限
    return loginStack.back().privilege;
}

std::string AccountSystem::get_selected_ISBN() const {
    if (loginStack.empty()) return "";
    return loginStack.back().selected_ISBN;
}

void AccountSystem::set_selected_ISBN(const std::string isbn) {
    if (!loginStack.empty()) {
        loginStack.back().selected_ISBN = isbn;
    }
}

AccountSystem::AccountSystem() {
    BlockList<31, int> accountIndex("account_index.dat");
    MemoryRiver<Account> accountStorage("account_data.dat");

    // 检查是否需要初始化根用户
    if (!user_exist("root")) {
        init_root();
    }
}

AccountSystem::~AccountSystem() = default;

// 检查用户是否存在
bool AccountSystem::user_exist(const char* UserID) {
    if (accountIndex.find(UserID).empty()) {
        return false;
    }
    return true;
}