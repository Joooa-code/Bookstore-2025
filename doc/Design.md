# 总体设计文档
# Bookstore-2025 
## by Joooa-code
## 功能概述
实现一个支持多用户权限、图书管理、交易记录和日志系统的书店管理系统。系统通过命令行交互，支持店主、员工、顾客三类用户的不同操作，所有数据持久化存储到文件。
## 主体逻辑说明
1. **启动阶段**
- 检查并初始化必要的数据文件
- 创建root管理员账户（用户ID：root，密码：sjtu，权限等级：7）
2. **交互阶段**
- 循环读取用户输入的指令
- 验证指令格式和权限
- 执行相应操作并输出结果
- 维护登录栈状态
3. **退出阶段**
- 读入EOF或者quit/exit指令时退出程序
- 保存所有数据到文件
- 清空登录栈
## 文件结构 
**include/**  
├── MemoryRiver.hpp     # 文件读写类（仓库管理）  
├── BlockList.hpp       # 块状链表模板类  
├── Account.hpp            # 用户账户管理  
├── Book.hpp            # 图书管理系统  
├── Log.hpp             # 日志记录系统   
└── Validator.hpp       # 输入验证工具    
**src/**   
├── Account.cpp            # 用户账户管理  
├── Book.cpp            # 图书管理系统  
├── Log.cpp             # 日志记录系统   
└── Validator.cpp       # 输入验证工具  
## 功能设计
### MemoryRiver类
```cpp
    template<class T, int info_len = 2>  
    class MemoryRiver {
    private:  
    fstream file;  
    string file_name;  
    int sizeofT = sizeof(T); 
    public:  
    MemoryRiver() = default;
    MemoryRiver(const string& file_name) : file_name(file_name) {}

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::out);
        int tmp = 0;
        for (int i = 0; i < info_len; ++i)
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
        file.close();
    }

    //读出第n个int的值赋给tmp，1_base
    void get_info(int &tmp, int n) {};

    //将tmp写入第n个int的位置，1_base
    void write_info(int tmp, int n) {};

    //在文件合适位置写入类对象t，并返回写入的位置索引index
    int write(T &t) {};

    //用t的值更新位置索引index对应的对象
    void update(T &t, const int index) {};

    //读出位置索引index对应的T对象的值并赋值给t
    void read(T &t, const int index) {};

    //删除位置索引index对应的对象
    void Delete(int index) {};
    };
```
### BlockList类
### Account模块
```cpp
struct Account {
    char UserID[31];
    char Password[31];
    char Username[31];
    int Privilege;
}; 

class AccountSystem{
private:
    BlockList<Account> accountStorage; // 用户信息存储
    std::vector<Account> loginStack; // 登录栈
    std::map<string, string> selectedBooks; // 选中图书栈（ID ISBN）
public:
    AccountSystem(); 
    ~AccountSystem(); 

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
    void register(const string& UserID, const string& Password, const string& Username);

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
```
### Book模块
```cpp
struct Book {
    char ISBN[21];  // 除不可见字符以外 ASCII 字符
    char BookName[61];  // 除不可见字符和英文双引号以外 ASCII 字符
    char Author[61];  // 除不可见字符和英文双引号以外 ASCII 字符
    char Keyword[61];  // 内容以 | 为分隔可以出现多段信息，每段信息长度至少为 1
    int Quantity;  
    double Price;  // 图书单价
    double TotalCost;  // 交易总额
};

class BookSystem {
private:
    BlockList<Book> bookStorage; // 图书信息存储

    AccountSystem* accountSystem;
    FinanceSystem* financeSystem;

public:
    BookSystem(AccountSystem* as, FinanceSystem* fs); 
    ~BookSystem(); 

    // 无附加参数时，输出所有图书
    // 附加参数内容为空则操作失败
    // {1} 
    void show();
    
    // 按ISBN查询
    // 输出图书信息或空行（无满足条件的图书）
    void show(const string& ISBN);
   
    // 输出图书信息或空行（无满足条件的图书），[Keyword] 中出现多个关键词则操作失败
    // some:name, author...
    void show(const string& some, const string& value);

    // 购买指定数量的指定图书,减少库存，以浮点数输出购买图书所需的总金额
    // 没有符合条件的图书则操作失败；购买数量为非正整数则操作失败
    // {1}
    void buy(const string& ISBN, int Quantity);

    // 以当前帐户选中指定图书
    // 没有符合条件的图书则创建仅拥有 [ISBN] 信息的新图书；退出系统视为取消选中图书。
    // {3}
    void select(const string& ISBN);

    // 以指令中的信息更新选中图书的信息
    // 如未选中图书则操作失败；有重复附加参数为非法指令；附加参数内容为空则操作失败；不允许将 ISBN 改为原有的 ISBN；[keyword] 包含重复信息段则操作失败
    // {3}
    void modify(const string& line);

    // 以指定交易总额购入指定数量的选中图书，增加其库存数
    // 如未选中图书则操作失败；购入数量为非正整数则操作失败；交易总额为非正数则操作失败。
    // {3}
    void import(int Quantity, double TotalCost);
};
```
### Log模块
```cpp
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
    FinanceSystem();
    ~FinanceSystem();

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
```
## 数据库设计
### 存储数据
用户信息、图书信息、财务信息、员工操作信息、系统日志
### 存储形式
利用块状链表