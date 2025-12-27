#ifndef BOOKSTORE_2025_ACCOUNT_H
#define BOOKSTORE_2025_ACCOUNT_H
#include "Storage.h"
#include "MemoryRiver.h"
#include <string>
struct Account {
    char UserID[31];  // 数字，字母，下划线
    char Password[31];  // 数字，字母，下划线
    char Username[31];  // 除不可见字符以外 ASCII 字符
    int Privilege;

    Account(): Privilege(0) {
        std::memset(UserID, 0, sizeof(UserID));
        std::memset(Password, 0, sizeof(Password));
        std::memset(Username, 0, sizeof(Username));
    }

    bool operator<(const Account& other) const {
        return strcmp(UserID, other.UserID) < 0;
    }
};

class AccountSystem{
private:
    BlockList<31, int> accountIndex; // 用户信息存储:UserID->accountStorage里的位置
    MemoryRiver<Account> accountStorage;  // 账户数据存储

    // 登录栈
    struct LoginRecord {
        std::string UserID;
        int privilege;
        std::string selected_ISBN;
    };
    std::vector<LoginRecord> loginStack;

    void init_root();
    bool ID_pw_check(const std::string& s) const;
    static bool name_check(const std::string& name);
public:
    AccountSystem();
    ~AccountSystem();

    // 获取当前登录用户信息
    std::string get_curID() const;
    int get_curpriv() const;
    std::string get_selected_ISBN() const;
    void set_selected_ISBN(const std::string ISBN);

    // 检查该用户是否已存在
    bool user_exist(const char* UserID);

    // 获取用户信息
    bool get_user_info(const std::string& UserID, Account& account);

    // 登录：若成功则修改登录栈
    // {0}
    void su(const string& UserID, const string& Password = "");

    // 撤销最后一次成功执行的 su 指令效果
    // 若无已登录帐户则操作失败
    // {1}
    void logout();

    // 注册权限等级为 {1} 的新账户
    // 若userID与已注册用户重复则失败
    // {0}
    void regis(const string& UserID, const string& Password, const string& Username);

    // 修改指定帐户的密码
    // 如果该帐户不存在则操作失败；如果密码错误则操作失败
    // 如果当前帐户权限等级为 {7} 则可以省略 [CurrentPassword]
    // {1}
    void passwd(const string& UserID, const string& NewPassword, const string& CurrentPassword = "");

    // 创建账户
    // 如果待创建帐户的权限等级>=当前帐户权限等级则操作失败；如果 [UserID] 与已注册帐户重复则操作失败。
    // {3}
    void useradd(const string& UserID, const string& Password, int Privilege, const string& Username);

    // 删除账户
    // 如果待删除帐户不存在/已登录则操作失败
    // {7}
    void deleteAccount(const string& UserID);
};
#endif //BOOKSTORE_2025_ACCOUNT_H