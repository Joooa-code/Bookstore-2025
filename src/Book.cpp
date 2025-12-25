#include "Book.h"

BookSystem::BookSystem(AccountSystem* as, LogSystem* ls)
    : accountSystem(as), logSystem(ls), selected(false) {
    std::memset(selected_ISBN, 0, sizeof(selected_ISBN));
    MemoryRiver<Book> bookStorage("book.dat");
    BlockList<21, BookIndex> ISBNIndex("ISBN_index.dat");
    BlockList<61, BookIndex> nameIndex("name_index.dat");
    BlockList<61, BookIndex> authorIndex("author_index.dat");
    BlockList<61, BookIndex> keywordIndex("keyword_index.dat");
}

BookSystem::~BookSystem() = default;

void BookSystem::show() {

}