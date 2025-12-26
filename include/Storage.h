#ifndef BOOKSTORE_2025_STORAGE_H
#define BOOKSTORE_2025_STORAGE_H
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// 常量定义
const int BLOCK_SIZE = 512;         // 默认块大小
const int MIN_BLOCK_SIZE = 64;      // 块合并阈值
const int MAX_HEAD_RESERVE = 480000; // 为NodeHead预留空间

// 文件头结构 (32字节)
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
    void read_file_header() {
        data_file.seekg(0);
        data_file.read(reinterpret_cast<char*>(&file_header), sizeof(FileHeader));
    }

    // 写入文件头
    void write_file_header() {
        data_file.seekp(0);
        data_file.write(reinterpret_cast<const char*>(&file_header), sizeof(FileHeader));
        data_file.flush();
    }

    // 读取NodeHead
    void read_head(NodeHead<INDEX_LEN>& head, int offset) {
        if (offset < 0) return;
        data_file.seekg(offset);
        data_file.read(reinterpret_cast<char*>(&head), sizeof(NodeHead<INDEX_LEN>));
    }

    // 写入NodeHead
    void write_head(const NodeHead<INDEX_LEN>& head, int offset) {
        if (offset < 0) return;
        data_file.seekp(offset);
        data_file.write(reinterpret_cast<const char*>(&head), sizeof(NodeHead<INDEX_LEN>));
        data_file.flush();
    }

    // 读取NodeBody
    void read_body(NodeBody<INDEX_LEN, TypeName>& body, int offset) {
        if (offset < 0) return;
        data_file.seekg(offset);
        data_file.read(reinterpret_cast<char*>(&body), sizeof(NodeBody<INDEX_LEN, TypeName>));
    }

    // 写入NodeBody
    void write_body(const NodeBody<INDEX_LEN, TypeName>& body, int offset) {
        if (offset < 0) return;
        data_file.seekp(offset);
        data_file.write(reinterpret_cast<const char*>(&body), sizeof(NodeBody<INDEX_LEN, TypeName>));
        data_file.flush();
    }

    // 在预留区域分配NodeHead
    int allocate_head() {
        int offset;

        if (file_header.free_head_offset != -1) {
            // 从空闲链表分配
            offset = file_header.free_head_offset;
            NodeHead<INDEX_LEN> free_head;
            read_head(free_head, offset);
            file_header.free_head_offset = free_head.next_offset;
        }
        else {
            // 从预留区域分配
            offset = head_start + file_header.count * head_size;
            file_header.count++;
        }

        write_file_header();  // 写回文件头
        return offset;
    }

    // 分配NodeBody
    int allocate_body() {
        int offset;

        if (file_header.free_body_offset != -1) {
            // 从空闲链表分配
            offset = file_header.free_body_offset;
            NodeBody<INDEX_LEN, TypeName> free_body;
            read_body(free_body, offset);
            file_header.free_body_offset = free_body.next_free;
        }
        else {
            // 从数据区域分配
            offset = data_start + file_header.count * body_size;
        }
        write_file_header();  // 写回文件头
        return offset;
    }

    // 释放NodeHead到空闲链表
    void free_head(int offset) {
        NodeHead<INDEX_LEN> freed_head;
        memset(&freed_head, 0, sizeof(NodeHead<INDEX_LEN>));
        freed_head.next_offset = file_header.free_head_offset;
        file_header.free_head_offset = offset;
        write_head(freed_head, offset);
        write_file_header();  // 写回文件头
    }

    // 释放NodeBody到空闲链表
    void free_body(int offset) {
        NodeBody<INDEX_LEN, TypeName> freed_body;
        memset(&freed_body, 0, sizeof(NodeBody<INDEX_LEN, TypeName>));
        freed_body.next_free = file_header.free_body_offset;
        file_header.free_body_offset = offset;
        write_body(freed_body, offset);
        write_file_header();  // 写回文件头
    }

    // 查找合适的插入块
    int find_suitable_block(const char* index) {
    if (file_header.first_head_offset == -1) {
        return -1;
    }

    int current_offset = file_header.first_head_offset;
    int prev_offset = -1;

    while (current_offset != -1) {
        NodeHead<INDEX_LEN> current_head;
        read_head(current_head, current_offset);

        int cmp_max = strcmp(index, current_head.max_index);

        if (cmp_max < 0) {
            // index < 当前块max_index
            return current_offset;
        }
        else if (cmp_max == 0) {
            // index == 当前块max_index
            // 先检查当前块是否已满
            if (current_head.pair_count < BLOCK_SIZE) {
                return current_offset;
            }

            // 当前块已满，检查下一个块
            if (current_head.next_offset == -1) {
                return current_offset;  // 没有下一个块
            }

            NodeHead<INDEX_LEN> next_head;
            read_head(next_head, current_head.next_offset);
            int cmp_next_min = strcmp(index, next_head.min_index);

            if (cmp_next_min == 0) {
                // index也等于下一个块的min_index
                if (next_head.pair_count < BLOCK_SIZE) {
                    return current_head.next_offset;
                } else {
                    return current_offset;  // 两个块都满
                }
            }
            else if (cmp_next_min < 0) {
                // index < 下一个块的min_index
                return current_offset;
            }
            else {
                // index > 下一个块的min_index，不应该发生
                // 因为当前块max_index不可能大于下一个块min_index
                prev_offset = current_offset;
                current_offset = current_head.next_offset;
            }
        }
        else {
            // index > 当前块max_index
            prev_offset = current_offset;
            current_offset = current_head.next_offset;
        }
    }

    // 遍历完所有块，index大于所有块的max_index
    return prev_offset;  // 插入到最后一个块
}

    // 创建新块
    int create_new_block(int insert_after) {
        // 分配NodeHead
        int new_head_offset = allocate_head();

        // 分配NodeBody
        int new_body_offset = allocate_body();

        // 创建新的NodeHead
        NodeHead<INDEX_LEN> new_head;
        memset(&new_head, 0, sizeof(NodeHead<INDEX_LEN>));
        new_head.body_offset = new_body_offset;
        new_head.pair_count = 0;
        new_head.min_index[0] = '\0';
        new_head.max_index[0] = '\0';

        if (insert_after == -1) {
            // 插入到链表头部
            if (file_header.first_head_offset != -1) {
                NodeHead<INDEX_LEN> prev_head;
                read_head(prev_head, file_header.first_head_offset);
                prev_head.prev_offset = new_head_offset;
                write_head(prev_head, file_header.first_head_offset);
            }
            new_head.prev_offset = -1;
            new_head.next_offset = file_header.first_head_offset;
            file_header.first_head_offset = new_head_offset;

            if (new_head.next_offset == -1) {
                file_header.last_head_offset = new_head_offset;
            }
        }
        else {
            // 插入到insert_after之后
            NodeHead<INDEX_LEN> after_head;
            read_head(after_head, insert_after);

            new_head.prev_offset = insert_after;
            new_head.next_offset = after_head.next_offset;
            after_head.next_offset = new_head_offset;

            write_head(after_head, insert_after);

            if (new_head.next_offset != -1) {
                NodeHead<INDEX_LEN> next_head;
                read_head(next_head, new_head.next_offset);
                next_head.prev_offset = new_head_offset;
                write_head(next_head, new_head.next_offset);
            }
            else {
                // 更新尾部
                file_header.last_head_offset = new_head_offset;
            }
        }

        // 初始化NodeBody
        NodeBody<INDEX_LEN, TypeName> new_body;
        memset(&new_body, 0, sizeof(NodeBody<INDEX_LEN, TypeName>));
        new_body.next_free = -1;

        // 写入新块
        write_head(new_head, new_head_offset);
        write_body(new_body, new_body_offset);
        write_file_header();

        return new_head_offset;
    }

     // 在块中插入条目
    bool insert_to_block(int head_offset, const char* index, TypeName value) {
        NodeHead<INDEX_LEN> head;
        read_head(head, head_offset);

        // 安全检查
        if (head.pair_count >= BLOCK_SIZE) {
            return false;  // 块已满，需要分裂
        }

        NodeBody<INDEX_LEN, TypeName> body;
        read_body(body, head.body_offset);

        // 二分查找插入位置
        int left = 0, right = head.pair_count - 1;
        int insert_pos = head.pair_count;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            int cmp = strcmp(body.pairs[mid].index, index);
            if (cmp < 0) {
                left = mid + 1;
            }
            else if (cmp > 0) {
                right = mid - 1;
                insert_pos = mid;
            }
            else {
                // 索引相同，比较值
                if (body.pairs[mid].value < value) {
                    left = mid + 1;
                }
                else if (body.pairs[mid].value > value) {
                    right = mid - 1;
                    insert_pos = mid;
                }
                else {
                    return true;  // 已存在，不插入
                }
            }
        }

        // 移动元素
        for (int i = head.pair_count; i > insert_pos; i--) {
            body.pairs[i] = body.pairs[i - 1];
        }

        // 插入新元素
        strncpy(body.pairs[insert_pos].index, index, INDEX_LEN - 1);
        body.pairs[insert_pos].index[INDEX_LEN - 1] = '\0';
        body.pairs[insert_pos].value = value;

        head.pair_count++;

        // 更新索引范围
        if (head.pair_count == 1) {
            strncpy(head.min_index, index, INDEX_LEN - 1);
            strncpy(head.max_index, index, INDEX_LEN - 1);
            head.min_index[INDEX_LEN - 1] = '\0';
            head.max_index[INDEX_LEN - 1] = '\0';
        }
        else {
            if (strcmp(index, head.min_index) < 0) {
                strncpy(head.min_index, index, INDEX_LEN - 1);
                head.min_index[INDEX_LEN - 1] = '\0';
            }
            if (strcmp(index, head.max_index) > 0) {
                strncpy(head.max_index, index, INDEX_LEN - 1);
                head.max_index[INDEX_LEN - 1] = '\0';
            }
        }

        // 写回
        write_head(head, head_offset);
        write_body(body, head.body_offset);

        return true;
    }

    // 查找包含特定index的第一个块
    int find_first_block_by_index(const char* index) {
        int current_offset = file_header.first_head_offset;

        while (current_offset != -1) {
            NodeHead<INDEX_LEN> current_head;
            read_head(current_head, current_offset);

            if (strcmp(index, current_head.max_index) <= 0) {
                if (strcmp(index, current_head.min_index) >= 0 || current_head.prev_offset == -1) {
                    return current_offset;
                }
            }

            if (current_head.next_offset == -1 || strcmp(index, current_head.min_index) < 0) {
                break;
            }

            current_offset = current_head.next_offset;
        }

        return -1;
    }

     // 分裂块
    void split_block(int head_offset) {
        NodeHead<INDEX_LEN> old_head;
        read_head(old_head, head_offset);

        NodeBody<INDEX_LEN, TypeName> old_body;
        read_body(old_body, old_head.body_offset);

        // 创建新块
        int new_head_offset = create_new_block(head_offset);
        if (new_head_offset == -1) return;

        NodeHead<INDEX_LEN> new_head;
        read_head(new_head, new_head_offset);
        NodeBody<INDEX_LEN, TypeName> new_body;
        read_body(new_body, new_head.body_offset);

        // 计算分裂点（大致一半）
        int split_point = old_head.pair_count / 2;

        // 将后半部分数据移动到新块
        new_head.pair_count = old_head.pair_count - split_point;
        for (int i = 0; i < new_head.pair_count; i++) {
            new_body.pairs[i] = old_body.pairs[split_point + i];

            // 更新新块的索引范围
            if (i == 0) {
                strncpy(new_head.min_index, new_body.pairs[i].index, INDEX_LEN - 1);
                strncpy(new_head.max_index, new_body.pairs[i].index, INDEX_LEN - 1);
            }
            else {
                if (strcmp(new_body.pairs[i].index, new_head.min_index) < 0) {
                    strncpy(new_head.min_index, new_body.pairs[i].index, INDEX_LEN - 1);
                }
                if (strcmp(new_body.pairs[i].index, new_head.max_index) > 0) {
                    strncpy(new_head.max_index, new_body.pairs[i].index, INDEX_LEN - 1);
                }
            }
        }
        new_head.min_index[INDEX_LEN - 1] = '\0';
        new_head.max_index[INDEX_LEN - 1] = '\0';

        // 更新旧块的条目数和索引范围
        old_head.pair_count = split_point;
        if (old_head.pair_count > 0) {
            strncpy(old_head.min_index, old_body.pairs[0].index, INDEX_LEN - 1);
            strncpy(old_head.max_index, old_body.pairs[old_head.pair_count - 1].index, INDEX_LEN - 1);
            old_head.min_index[INDEX_LEN - 1] = '\0';
            old_head.max_index[INDEX_LEN - 1] = '\0';
        }

        // 更新链表连接
        old_head.next_offset = new_head_offset;

        if (new_head.next_offset != -1) {
            NodeHead<INDEX_LEN> next_head;
            read_head(next_head, new_head.next_offset);
            next_head.prev_offset = new_head_offset;
        }
        else {
            file_header.last_head_offset = new_head_offset;
        }

        // 清空旧块中已移动的数据
        for (int i = split_point; i < BLOCK_SIZE; i++) {
            memset(&old_body.pairs[i], 0, sizeof(KeyValue<INDEX_LEN, TypeName>));
        }

        // 写入所有更新
        write_head(old_head, head_offset);
        write_body(old_body, old_head.body_offset);

        write_head(new_head, new_head_offset);
        write_body(new_body, new_head.body_offset);
        write_file_header();
    }

    // 合并两个块
    void merge_blocks(NodeHead<INDEX_LEN>& left_head, NodeHead<INDEX_LEN>& right_head, int left_offset, int right_offset) {
        NodeBody<INDEX_LEN, TypeName> left_body, right_body;
        read_body(left_body, left_head.body_offset);
        read_body(right_body, right_head.body_offset);

        // 将右块数据复制到左块
        for (int i = 0; i < right_head.pair_count; i++) {
            left_body.pairs[left_head.pair_count + i] = right_body.pairs[i];
        }
        left_head.pair_count += right_head.pair_count;
        left_head.next_offset = right_head.next_offset;

        // 更新左块的max_index
        if (right_head.pair_count > 0) {
            strncpy(left_head.max_index, right_body.pairs[right_head.pair_count - 1].index, INDEX_LEN - 1);
            left_head.max_index[INDEX_LEN - 1] = '\0';
        }

        // 更新链表
        if (right_head.next_offset != -1) {
            NodeHead<INDEX_LEN> next_head;
            read_head(next_head, right_head.next_offset);
            next_head.prev_offset = left_offset;
            write_head(next_head, right_head.next_offset);
        }
        else {
            file_header.last_head_offset = left_offset;
        }
        // 写入左块
        write_head(left_head, left_offset);
        write_body(left_body, left_head.body_offset);

        // 释放右块的head和body
        free_head(right_offset);
        free_body(right_head.body_offset);
    }

    // 尝试合并块
    void try_merge_blocks(int head_offset) {
        NodeHead<INDEX_LEN> head;
        read_head(head, head_offset);

        if (head.pair_count >= MIN_BLOCK_SIZE) {
            return;  // 不需要合并
        }

        // 尝试与前面的块合并
        if (head.prev_offset != -1) {
            NodeHead<INDEX_LEN> prev_head;
            read_head(prev_head, head.prev_offset);
            if (prev_head.pair_count + head.pair_count <= BLOCK_SIZE) {
                merge_blocks(prev_head, head, head.prev_offset, head_offset);
                return;
            }
        }
        // 尝试与后面的块合并
        if (head.next_offset != -1) {
            NodeHead<INDEX_LEN> next_head;
            read_head(next_head, head.next_offset);
            if (head.pair_count + next_head.pair_count <= BLOCK_SIZE) {
                merge_blocks(head, next_head, head_offset, head.next_offset);
                return;
            }
        }
    }

    // 在块中删除条目
    bool delete_from_block(int head_offset, const char* index, TypeName value) {
        NodeHead<INDEX_LEN> head;
        read_head(head, head_offset);

        NodeBody<INDEX_LEN, TypeName> body;
        read_body(body, head.body_offset);

        // 二分查找删除位置
        int left = 0, right = head.pair_count - 1;
        int delete_pos = -1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            int cmp = strcmp(body.pairs[mid].index, index);
            if (cmp < 0) {
                left = mid + 1;
            }
            else if (cmp > 0) {
                right = mid - 1;
            }
            else {
                // 索引相同，查找值
                int value_cmp = body.pairs[mid].value - value;
                if (value_cmp < 0) {
                    left = mid + 1;
                }
                else if (value_cmp > 0) {
                    right = mid - 1;
                }
                else {
                    delete_pos = mid;
                    break;
                }
            }
        }

        if (delete_pos == -1) {
            return false;  // 未找到
        }

        // 移动元素覆盖要删除的元素
        for (int i = delete_pos; i < head.pair_count - 1; i++) {
            body.pairs[i] = body.pairs[i + 1];
        }
        head.pair_count--;
        memset(&body.pairs[head.pair_count], 0, sizeof(KeyValue<INDEX_LEN, TypeName>));

        // 更新索引范围
        if (head.pair_count > 0) {
            strncpy(head.min_index, body.pairs[0].index, INDEX_LEN - 1);
            strncpy(head.max_index, body.pairs[head.pair_count - 1].index, INDEX_LEN - 1);
            head.min_index[INDEX_LEN - 1] = '\0';
            head.max_index[INDEX_LEN - 1] = '\0';
        }
        else {
            // 块为空
            head.min_index[0] = '\0';
            head.max_index[0] = '\0';
        }

        // 写回
        write_head(head, head_offset);
        write_body(body, head.body_offset);

        return true;
    }

    // 初始化新文件
    void init_new_file() {
        // 初始化文件头
        memset(&file_header, 0, sizeof(FileHeader));
        file_header.first_head_offset = -1;
        file_header.last_head_offset = -1;
        file_header.free_head_offset = -1;
        file_header.free_body_offset = -1;
        file_header.count = 0;
        // 写入文件头
        write_file_header();
    }

public:
    BlockList() = default;
    explicit BlockList(const string& filename) {
        header_size = sizeof(FileHeader);
        head_size = sizeof(NodeHead<INDEX_LEN>);
        body_size = sizeof(int32_t) + BLOCK_SIZE * sizeof(KeyValue<INDEX_LEN, TypeName>);

        // 计算各个区域的起始偏移
        head_start = header_size;
        data_start = head_start + MAX_HEAD_RESERVE;

        this->filename = filename;

        // 打开或创建文件
        data_file.open(filename, ios::in | ios::out | ios::binary);
        if (!data_file.is_open() || data_file.peek() == EOF) {
            // 文件不存在，创建新文件
            data_file.close();
            data_file.open(filename, ios::out | ios::binary);
            data_file.close();
            data_file.open(filename, ios::in | ios::out | ios::binary);
            // 初始化新文件
            init_new_file();
        }
        else {
            // 读取现有文件头
            read_file_header();
        }
    }

    ~BlockList() {
        if (data_file.is_open()) {
            data_file.close();
        }
    }

    // 插入操作
    void insert(const char* index, TypeName value) {
        // 查找合适的块
        int target_offset = find_suitable_block(index);

        // 处理数据库为空的情况
        if (target_offset == -1) {
            // 需要插入在第一个块之前
            target_offset = create_new_block(-1);
            insert_to_block(target_offset, index, value);
            write_file_header();
            return;
        }

        // 读取目标块的信息
        NodeHead<INDEX_LEN> target_head;
        read_head(target_head, target_offset);

        // 检查是否需要分裂
        if (target_head.pair_count >= BLOCK_SIZE) {
            split_block(target_offset);
            // 分裂后重新查找合适的块
            target_offset = find_suitable_block(index);
            if (target_offset == -1) {
                target_offset = create_new_block(-1);
            }
        }

        bool success = insert_to_block(target_offset, index, value);
        if (!success) {  // 有相同项不插入
                return;
        }

        write_file_header();
    }

    // 删除操作
    void remove(const char* index, TypeName value) {
        int current_offset = find_first_block_by_index(index);

        while (current_offset != -1) {
            NodeHead<INDEX_LEN> current_head;
            read_head(current_head,current_offset);

            // 如果index超出当前块范围，停止查找
            if (strcmp(index, current_head.min_index) < 0) {
                break;
            }

            if (strcmp(index, current_head.min_index) >= 0 && strcmp(index, current_head.max_index) <= 0) {
                // 尝试在当前块中删除
                bool deleted = delete_from_block(current_offset, index, value);
                if (deleted) {

                        try_merge_blocks(current_offset);

                    write_file_header();
                    return;
                }
            }
            // 继续查找下一个块
            current_offset = current_head.next_offset;
        }
        write_file_header();
    }

    // 查找操作
    vector<TypeName> find(const char* index) {
        vector<TypeName> result;
        // 查找第一个可能包含该index的块
        int current_offset = find_first_block_by_index(index);

        while (current_offset != -1) {
            NodeHead<INDEX_LEN> current_head;
            read_head(current_head, current_offset);
            // 如果index超出范围，停止查找
            if (strcmp(index, current_head.min_index) < 0) {
                break;
            }

            if (strcmp(index, current_head.min_index) >= 0 && strcmp(index, current_head.max_index) <= 0) {
                // 在这个块中查找
                NodeBody<INDEX_LEN, TypeName> current_body;
                read_body(current_body, current_head.body_offset);
                for (int i = 0; i < current_head.pair_count; i++) {
                    if (strcmp(current_body.pairs[i].index, index) == 0) {
                        result.push_back(current_body.pairs[i].value);
                    }
                    else if (strcmp(current_body.pairs[i].index, index) > 0) {
                        // 块内是有序的，可以提前结束
                        break;
                    }
                }
            }
            // 继续下一个块
            current_offset = current_head.next_offset;
        }

        // 排序结果
        sort(result.begin(), result.end());
        return result;
    }

    // 新增：获取全部元素
    std::vector<TypeName> get_all() {
        std::vector<TypeName> result;

        // 从头节点开始遍历
        int current_offset = file_header.first_head_offset;

        while (current_offset != -1) {
            NodeHead<INDEX_LEN> current_head;
            read_head(current_head, current_offset);

            if (current_head.pair_count > 0) {
                NodeBody<INDEX_LEN, TypeName> current_body;
                read_body(current_body, current_head.body_offset);

                // 将当前块中的所有元素添加到结果中
                for (int i = 0; i < current_head.pair_count; i++) {
                    result.push_back(current_body.pairs[i].value);
                }
            }

            // 移动到下一个块
            current_offset = current_head.next_offset;
        }

        // 按索引排序
        return result;
    }
};
#endif //BOOKSTORE_2025_STORAGE_H