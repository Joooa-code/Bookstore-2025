#ifndef BOOKSTORE_2025_BOOK_H
#define BOOKSTORE_2025_BOOK_H
#include "Storage.h"
#include "Account.h"
#include "Log.h"
#include"MemoryRiver.h"
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
#endif //BOOKSTORE_2025_BOOK_H