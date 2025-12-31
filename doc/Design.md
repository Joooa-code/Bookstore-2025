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
├── Account.hpp         # 用户账户管理  
├── Book.hpp            # 图书管理系统   
└── Log.hpp             # 日志记录系统   
**src/**   
├── Account.cpp         # 用户账户管理  
├── Book.cpp            # 图书管理系统  
├── Log.cpp             # 日志记录系统   
└── main.cpp       
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
```cpp
struct FileHeader {
    int first_head_offset;    // 第一个NodeHead的偏移量
    int last_head_offset;     // 最后一个NodeHead的偏移量
    int free_head_offset;     // 空闲NodeHead链表头
    int count;                // 使用的NodeHead和NodeBody数量
    int free_body_offset;     // 空闲NodeBody链表头
    int padding[3];          // 填充
};

// 数据条目结构
template<int INDEX_LEN, typename TypeName>
struct KeyValue {
    char index[INDEX_LEN];        // 索引字符串
    TypeName value;                // 值
};

// NodeBody结构
template<int INDEX_LEN, typename TypeName>
struct NodeBody {
    int next_free;            // 空闲链表指针
    KeyValue<INDEX_LEN, TypeName> pairs[BLOCK_SIZE];       // 数据条目数组
};

// NodeHead结构
template<int INDEX_LEN>
struct NodeHead {
    int prev_offset;          // 前一个NodeHead的偏移量
    int next_offset;          // 下一个NodeHead的偏移量
    int body_offset;          // 对应NodeBody在文件中的偏移量
    int pair_count;           // 当前块中存储的数据数量
    char min_index[INDEX_LEN];    // 当前块中最小index
    char max_index[INDEX_LEN];    // 当前块中最大index
};

template<int INDEX_LEN, typename TypeName>
class BlockList {
private:
    fstream data_file;            // 数据文件
    string filename;              // 文件名

    FileHeader file_header;       // 文件头缓存
    int header_size;          // 文件头大小
    int head_size;            // NodeHead大小
    int body_size;            // NodeBody大小
    int head_start;           // NodeHead区域起始偏移
    int data_start;           // 数据区域起始偏移

    // 读取文件头
    void read_file_header() {}

    // 写入文件头
    void write_file_header() {}

    // 读取NodeHead
    void read_head(NodeHead<INDEX_LEN>& head, int offset) {}

    // 写入NodeHead
    void write_head(const NodeHead<INDEX_LEN>& head, int offset) {}

    // 读取NodeBody
    void read_body(NodeBody<INDEX_LEN, TypeName>& body, int offset) {}

    // 写入NodeBody
    void write_body(const NodeBody<INDEX_LEN, TypeName>& body, int offset) {}

    // 在预留区域分配NodeHead
    int allocate_head() {}

    // 分配NodeBody
    int allocate_body() {}

    // 释放NodeHead到空闲链表
    void free_head(int offset) {}

    // 释放NodeBody到空闲链表
    void free_body(int offset) {}

    // 查找合适的插入块
    int find_suitable_block(const char* index) {}

    // 创建新块
    int create_new_block(int insert_after) {}

     // 在块中插入条目
    bool insert_to_block(int head_offset, const char* index, TypeName value) {}

    // 查找包含特定index的第一个块
    int find_first_block_by_index(const char* index) {}

     // 分裂块
    void split_block(int head_offset) {}

    // 合并两个块
    void merge_blocks(NodeHead<INDEX_LEN>& left_head, NodeHead<INDEX_LEN>& right_head, int left_offset, int right_offset) {}

    // 尝试合并块
    void try_merge_blocks(int head_offset) {}

    // 在块中删除条目
    bool delete_from_block(int head_offset, const char* index, TypeName value) {}

    // 初始化新文件
    void init_new_file() {}

public:
    BlockList() = default;
    explicit BlockList(const string& filename) {}

    ~BlockList() {}

    // 插入操作
    void insert(const char* index, TypeName value) {}

    // 删除操作
    void remove(const char* index, TypeName value) {}

    // 查找操作
    vector<TypeName> find(const char* index) {}

    // 新增：获取全部元素
    std::vector<TypeName> get_all() {}
};
```
### Account模块
```cpp
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

    bool ID_pw_check(const std::string& s) const;
    static bool name_check(const std::string& name);
    bool priv_check(int priv) const;
public:
    AccountSystem();
    ~AccountSystem();

    // 获取当前登录用户信息

    int get_curpriv() const;
    std::string get_selected_ISBN() const;
    void set_selected_ISBN(const std::string& ISBN);

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
```
### Book模块
```cpp
struct Book {
    char ISBN[21];  // 除不可见字符以外 ASCII 字符
    char BookName[61];  // 除不可见字符和英文双引号以外 ASCII 字符
    char Author[61];  // 除不可见字符和英文双引号以外 ASCII 字符
    char Keyword[61];  // 内容以 | 为分隔可以出现多段信息，每段信息长度至少为 1，除不可见字符和英文双引号以外 ASCII 字符
    int Stock;  // 库存
    double Price;  // 图书单价
    double TotalCost;  // 交易总额

    Book(): Stock(0), Price(0), TotalCost(0) {
        std::memset(ISBN, 0, sizeof(ISBN));
        std::memset(BookName, 0, sizeof(BookName));
        std::memset(Author, 0, sizeof(Author));
        std::memset(Keyword, 0, sizeof(Keyword));
    }

    // 任何两本图书不应该有相同的ISBN信息
    bool operator==(const Book& other) const {
        return std::strcmp(ISBN, other.ISBN) == 0;
    }
};

class BookSystem {
private:
    MemoryRiver<Book> bookStorage; // 图书信息完整存储(使用MemoryRiver)

    struct BookIndex {
        char ISBN[21];  //ISBN
        int storage_pos;  // position in MemoryRiver

        bool operator <(const BookIndex& other) const {
            return strcmp(ISBN, other.ISBN) < 0;
        }
        bool operator >(const BookIndex& other) const {
            return strcmp(ISBN, other.ISBN) > 0;
        }
    };

    BlockList<21, BookIndex> ISBNIndex;  // ISBN索引
    BlockList<61, BookIndex> nameIndex;  // 书名索引
    BlockList<61, BookIndex> authorIndex;  // 作者名索引
    BlockList<61, BookIndex> keywordIndex;  // 关键词索引

    AccountSystem* accountSystem;
    LogSystem* logSystem;

    bool selected;  // 当前是否选中图书
    char selected_ISBN[21];  // 当前选中图书的ISBN
public:
    BookSystem(AccountSystem* as, LogSystem* ls);
    ~BookSystem();

    static bool ISBN_check(const std::string& s);
    static bool other_check(const std::string& s);
    static bool keywords_repetition(const std::vector<std::string>& keywords);


    // 无附加参数时，输出所有图书
    // 附加参数内容为空则操作失败
    // {1}
    void show();

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
```
## 数据库设计
### 存储数据
用户信息、图书信息、财务信息、员工操作信息、系统日志
### 存储形式
利用块状链表