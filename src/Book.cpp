#include "Book.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iomanip>

BookSystem::BookSystem(AccountSystem* as, LogSystem* ls)
    : accountSystem(as), logSystem(ls), selected(false),
      ISBNIndex("ISBN_index.dat"),
      nameIndex("name_index.dat"),
      authorIndex("author_index.dat"),
      keywordIndex("keyword_index.dat") {
    std::memset(selected_ISBN, 0, sizeof(selected_ISBN));
    bookStorage.initialise("book_data.dat");
}

BookSystem::~BookSystem() = default;

// 检查ISBN
static bool ISBN_check(const std::string& s) {
    if (s.empty() || s.length() > 20) return false;  // 最大长度20
    for (char c : s) {
        if (c < 32 || c > 126) { // 不可见字符ASCII码小于32
            return false;
        }
    }
    return true;
}

// 检查
static bool other_check(const std::string& s) {
    if (s.empty() || s.length() > 60) return false;  // 最大长度60
    for (char c : s) {
        if (c < 32 || c > 126 || c == '\"') {
            return false;
        }
    }
    return true;
}

// 分割关键词字符串
static std::vector<std::string> split_keywords(const std::string& keyword_str) {
    std::vector<std::string> keywords;
    std::string current;

    for (char c : keyword_str) {
        if (c == '|') {
            if (!current.empty()) {
                keywords.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }

    if (!current.empty()) {
        keywords.push_back(current);
    }
    return keywords;
}

// 检查关键词是否重复
static bool keywords_repetition(const std::vector<std::string>& keywords) {
    std::vector<std::string> sorted_keywords = keywords;
    std::sort(sorted_keywords.begin(), sorted_keywords.end());

    for (size_t i = 1; i < sorted_keywords.size(); ++i) {
        if (sorted_keywords[i] == sorted_keywords[i-1]) {
            return true;
        }
    }
    return false;
}

void BookSystem::show() {
    // 权限检查
    if (accountSystem->get_curpriv() < 1) {
        std::cout << "Invalid\n";
        return;
    }

    // 获取所有图书
    auto all_books = ISBNIndex.get_all();

    if (all_books.empty()) {
        std::cout << "\n";  // 输出空行
        return;
    }

    // 输出所有图书
    for (const auto& book_idx : all_books) {
        Book book;
        bookStorage.read(book, book_idx.storage_pos);

        // 输出格式：ISBN\tBookName\tAuthor\tKeyword\tPrice\tStock
        std::cout << book.ISBN << "\t"
                  << book.BookName << "\t"
                  << book.Author << "\t"
                  << book.Keyword << "\t"
                  << std::fixed << std::setprecision(2) << book.Price << "\t"
                  << book.Stock << "\n";
    }
}

// 输出图书信息或空行（无满足条件的图书），[Keyword] 中出现多个关键词则操作失败
// some:name, author...
void BookSystem::show(const string& some, const string& value) {
    // 权限检查
    if (accountSystem->get_curpriv() < 1) {
        std::cout << "Invalid\n";
        return;
    }

    if (some == "ISBN") {
        // 参数检查
        if (!ISBN_check(value)) {
            std::cout << "Invalid\n";
            return;
        }
        // 查找图书
        BookIndex target;
        strcpy(target.ISBN, value.c_str());
        auto result = ISBNIndex.find(value.c_str());
        if (result.empty()) {
            std::cout << "\n";  // 输出空行
            return;
        }
        // 输出图书信息
        Book book;
        bookStorage.read(book, result[0].storage_pos);
        std::cout << book.ISBN << "\t"
              << book.BookName << "\t"
              << book.Author << "\t"
              << book.Keyword << "\t"
              << std::fixed << std::setprecision(2) << book.Price << "\t"
              << book.Stock << "\n";
        return;
    }
    else {
        std::vector<BookIndex> results;
        // name
        if (some == "name") {
            if (!other_check(value)) {
                std::cout << "Invalid\n";
                return;
            }

            results = nameIndex.find(value.c_str());
        }

        // author
        else if (some == "author") {
            if (!other_check(value)) {
                std::cout << "Invalid\n";
                return;
            }

            results = authorIndex.find(value.c_str());
        }

        // keyword
        else if (some == "keyword") {
            if (!other_check(value)) {
                std::cout << "Invalid\n";
                return;
            }

            // 检查是否包含多个关键词
            std::vector<std::string> keywords = split_keywords(value);
            if (keywords.size() != 1 || keywords.empty()) {
                std::cout << "Invalid\n";
                return;
            }

            results = keywordIndex.find(keywords[0].c_str());
        }

        else {
            std::cout << "Invalid\n";
            return;  // some不合法
        }

        if (results.empty()) {
            std::cout << "\n";  // 输出空行
            return;
        }
        // 按ISBN排序
        std::sort(results.begin(), results.end());
        // 输出图书信息
        for (const auto& book_idx : results) {
            Book book;
            bookStorage.read(book, book_idx.storage_pos);

            std::cout << book.ISBN << "\t"
                      << book.BookName << "\t"
                      << book.Author << "\t"
                      << book.Keyword << "\t"
                      << std::fixed << std::setprecision(2) << book.Price << "\t"
                      << book.Stock << "\n";
        }
    }
}

// 购买指定数量的指定图书,减少库存，以浮点数输出购买图书所需的总金额
void BookSystem::buy(const string& ISBN, int Quantity) {
    // 权限检查
    if (accountSystem->get_curpriv() < 1) {
        std::cout << "Invalid\n";
        return;
    }

    // 参数检查
    if (!ISBN_check(ISBN) || Quantity <= 0) {
        std::cout << "Invalid\n";
        return;
    }

    // 查找图书
    auto result = ISBNIndex.find(ISBN.c_str());
    if (result.empty()) {
        std::cout << "Invalid\n";
        return;
    }
    // 检查库存
    Book book;
    bookStorage.read(book, result[0].storage_pos);
    if (book.Stock < Quantity) {
        std::cout << "Invalid\n";
        return;
    }

    // 计算总价
    double total_price = book.Price * Quantity;
    // 减少库存
    book.Stock -= Quantity;
    book.TotalCost += total_price; // 每本书的交易总额

    // 更新图书信息
    bookStorage.update(book, result[0].storage_pos);

    // 输出总金额
    std::cout << std::fixed << std::setprecision(2) << total_price << "\n";
    logSystem->recordFinance(total_price);
}

// 以当前帐户选中指定图书
void BookSystem::select(const string& ISBN) {
    // 权限检查
    if (accountSystem->get_curpriv() < 3) {
        std::cout << "Invalid\n";
        return;
    }
    // 参数检查
    if (!ISBN_check(ISBN)) {
        std::cout << "Invalid\n";
        return;
    }
    // 查找图书
    auto result = ISBNIndex.find(ISBN.c_str());
    if (result.empty()) {
        // 图书不存在，创建新图书
        Book new_book;
        strcpy(new_book.ISBN, ISBN.c_str());
        strcpy(new_book.BookName, "");
        strcpy(new_book.Author, "");
        strcpy(new_book.Keyword, "");
        new_book.Stock = 0;
        new_book.Price = 0;
        new_book.TotalCost = 0;
        int pos = bookStorage.write(new_book);
        // 创建索引
        BookIndex idx;
        strcpy(idx.ISBN, ISBN.c_str());
        idx.storage_pos = pos;
        ISBNIndex.insert(ISBN.c_str(), idx);
    }
    // 设置选中状态
    selected = true;
    strcpy(selected_ISBN, ISBN.c_str());

    // 更新账户系统中的选中信息
    accountSystem->set_selected_ISBN(ISBN);
}

// 修改选中图书
void BookSystem::modify(const std::string& line) {
    // 检查权限
    if (accountSystem->get_curpriv() < 3) {
        std::cout << "Invalid\n";
        return;
    }

    // 检查有没有选中书
    if (!selected) {
        std::cout << "Invalid\n";
        return;
    }

    // 从书库里找到选中的书
    auto result = ISBNIndex.find(selected_ISBN);
    if (result.empty()) {
        std::cout << "Invalid\n";
        return;
    }

    Book book;
    int pos = result[0].storage_pos;
    bookStorage.read(book, pos);

    std::string new_ISBN, new_name, new_author, new_keywords;
    bool have_ISBN = false, have_name = false, have_author = false, have_keyword = false;
    double new_price = book.Price;

    size_t start = 0;
    while (start < line.length()) {
        // 找到下一个空格
        size_t end = line.find(' ', start);
        if (end == std::string::npos) {
            end = line.length();
        }

        // 取出一个参数
        std::string p = line.substr(start, end - start);

        // 判断参数类型
        if (p.substr(0, 6) == "-ISBN=") {
            if (have_ISBN) {
                std::cout << "Invalid\n";  // 重复的ISBN参数
                return;
            }
            new_ISBN = p.substr(6);
            have_ISBN = true;
        }
        else if (p.substr(0, 6) == "-name=") {
            if (have_name) {
                std::cout << "Invalid\n";  // 重复的书名参数
                return;
            }
            new_name = p.substr(6);
            have_name = true;
        }
        else if (p.substr(0, 8) == "-author=") {
            if (have_author) {
                std::cout << "Invalid\n";  // 重复的作者参数
                return;
            }
            new_author = p.substr(8);
            have_author = true;
        }
        else if (p.substr(0, 9) == "-keyword=") {
            if (have_keyword) {
                std::cout << "Invalid\n";  // 重复的关键词参数
                return;
            }
            new_keywords = p.substr(9);
            have_keyword = true;
        }
        else if (p.substr(0, 7) == "-price=") {
            std::string price_str = p.substr(7);
            if (price_str.length() > 13) {
                cout << "Invalid\n";
                return;  // 长度不能超过13
            }
            bool valid = true;
            bool dot_found = false;
            for (char c : price_str) {
                if (c == '.') {
                    if (dot_found) {
                        valid = false;
                        break;
                    }
                    dot_found = true;
                }
                else if (!isdigit(c)) {
                    valid = false;
                    break;
                }
            }
            if (!valid || price_str.empty()) {
                cout << "Invalid\n";
                return;
            }
            double price = stod(price_str);
            book.Price = price;
        }
        else {
            std::cout << "Invalid\n";  // 没有对应参数
            return;
        }

        start = end + 1;  // 处理下一个参数
    }

    // 检查参数是否有效
    if (have_ISBN) {
        if (!ISBN_check(new_ISBN)) {
            std::cout << "Invalid\n";
            return;
        }
        // 检查新ISBN是否和其他书重复
        if (strcmp(book.ISBN, new_ISBN.c_str()) != 0) {
            auto exist = ISBNIndex.find(new_ISBN.c_str());
            if (!exist.empty()) {
                std::cout << "Invalid\n";  // ISBN已存在
                return;
            }
        }
        // 不能修改为原来的ISBN
        else {
            std::cout << "Invalid\n";
            return;
        }
    }

    if (have_name) {
        if (!other_check(new_name)) {
            std::cout << "Invalid\n";
            return;
        }
    }

    if (have_author) {
        if (!other_check(new_author)) {
            std::cout << "Invalid\n";
            return;
        }
    }

    if (have_keyword) {
        if (!other_check(new_keywords)) {
            std::cout << "Invalid\n";
            return;
        }
        // 检查关键词格式
        std::vector<std::string> keywords = split_keywords(new_keywords);
        if (keywords.empty()) {
            std::cout << "Invalid\n";
            return;
        }
        // 检查关键词是否重复
        if (keywords_repetition(keywords)) {
            std::cout << "Invalid\n";
            return;
        }
    }

    // 修改图书信息
    // 改ISBN
    if (have_ISBN) {
        // 从索引中删除旧的ISBN
        ISBNIndex.remove(book.ISBN, result[0]);
        // 添加新的ISBN到索引
        BookIndex new_idx;
        strcpy(new_idx.ISBN, new_ISBN.c_str());
        new_idx.storage_pos = pos;
        ISBNIndex.insert(new_ISBN.c_str(), new_idx);
        // 更新书里的ISBN
        strcpy(book.ISBN, new_ISBN.c_str());
        strcpy(selected_ISBN, new_ISBN.c_str());  // 更新选中的ISBN
    }
    // 改书名
    if (have_name) {
        // 删除旧的书名索引
        if (book.BookName[0] != '\0') {
            nameIndex.remove(book.BookName, result[0]);
        }
        // 添加新的书名索引
        BookIndex name_idx;
        strcpy(name_idx.ISBN, book.ISBN);
        name_idx.storage_pos = pos;
        nameIndex.insert(new_name.c_str(), name_idx);
        // 更新书里的书名
        strcpy(book.BookName, new_name.c_str());
    }
    // 改作者
    if (have_author) {
        // 删除旧的作者索引
        if (book.Author[0] != '\0') {
            authorIndex.remove(book.Author, result[0]);
        }
        // 添加新的作者索引
        BookIndex author_idx;
        strcpy(author_idx.ISBN, book.ISBN);
        author_idx.storage_pos = pos;
        authorIndex.insert(new_author.c_str(), author_idx);
        // 更新书里的作者
        strcpy(book.Author, new_author.c_str());
    }
    // 改关键词
    if (have_keyword) {
        // 删除旧的关键词索引
        if (book.Keyword[0] != '\0') {
            std::vector<std::string> old_keywords = split_keywords(book.Keyword);
            for (const auto& keyword : old_keywords) {
                keywordIndex.remove(keyword.c_str(), result[0]);
            }
        }
        // 添加新的关键词索引
        std::vector<std::string> keywords = split_keywords(new_keywords);
        for (const auto& keyword : keywords) {
            BookIndex keyword_idx;
            strcpy(keyword_idx.ISBN, book.ISBN);
            keyword_idx.storage_pos = pos;
            keywordIndex.insert(keyword.c_str(), keyword_idx);
        }
        // 更新书里的关键词
        strcpy(book.Keyword, new_keywords.c_str());
    }

    // 修改存储中的图书信息
    bookStorage.update(book, pos);
}

// 以指定交易总额购入指定数量的选中图书，增加其库存数
void BookSystem::import(int Quantity, double TotalCost) {
    // 权限检查
    if (accountSystem->get_curpriv() < 3) {
        std::cout << "Invalid\n";
        return;
    }
    // 数值检查
    if (Quantity <= 0 || TotalCost <= 0) {
        std::cout << "Invalid\n";
        return;
    }
    // 检查是否有选中图书
    if (!selected) {
        std::cout << "Invalid\n";
        return;
    }
    // 获取选中图书
    auto result = ISBNIndex.find(selected_ISBN);
    if (result.empty()) {
        std::cout << "Invalid\n";
        return;
    }
    Book book;
    int pos = result[0].storage_pos;
    bookStorage.read(book, pos);
    // 增加库存
    book.Stock += Quantity;

    // 更新图书信息
    bookStorage.update(book, pos);
    logSystem->recordFinance(-TotalCost);
}