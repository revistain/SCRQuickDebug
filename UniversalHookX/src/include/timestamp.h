#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <vector>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include <iostream>
#include <windows.h>
#include <ctime>
#include <atomic>
#include <chrono>
#include <memory> 
#include "variable.h"

void ensureEndTimestampThread();
void analyzeTimeStamp(std::unique_ptr<Variables>& var_ptr);

void startTimeStamp(std::unique_ptr<Variables>& var_ptr);
void endTimeStamp(std::unique_ptr<Variables>& var_ptr);

typedef struct TreeNode {
    uint32_t func_number;
    uint32_t func_time;
    TreeNode* parent;
    std::vector<std::shared_ptr<TreeNode>> childs;

    TreeNode(uint32_t _func_number, uint32_t _func_time)
        : func_number(_func_number), func_time(_func_time), parent(nullptr) {}

    // �ùٸ��� child�� �߰��ϴ� �Լ�
    std::shared_ptr<TreeNode> addChild(TreeNode* _parent, uint32_t _func_number, uint32_t _func_time) {
        // std::make_shared�� �ڽ��� ����
        std::shared_ptr<TreeNode> child = std::make_shared<TreeNode>(_func_number, _func_time);
        child->parent = _parent;  // �θ� ��� ����

        childs.push_back(child);  // �ڽ� ��� �߰�
        return child;
    }

    // �ڽ��� ã�� �Լ�
    TreeNode* findChild(uint32_t func_number) {
        for (auto& child : childs) {
            if (child->func_number == func_number) {
                return child.get();  // �ڽ��� raw ������ ��ȯ
            }
        }
        return nullptr;
    }
} TreeNode;

class Tree {
public:
    std::shared_ptr<TreeNode> root;  // Ʈ���� ��Ʈ ���
    Tree() {
        root = std::make_shared<TreeNode>(0xEDACEDAC, 0);
    }

    void printTree(std::unique_ptr<Variables>& var_ptr, TreeNode* node, uint32_t depth=0) const {
        if (!node) return;

        // ���̿� ���� �鿩����
        for (int i = 0; i < depth; ++i) {
            std::cout << "  ";
        }

        // ���� ����� ������ ���
        if(node->func_number != 0xEDACEDAC) std::cout << std::dec << var_ptr->strtable.str[node->func_number+var_ptr->functrace.offset] << " / " << node->func_time << std::endl;

        // �ڽ� ���� ���
        for (const auto& child : node->childs) {
            printTree(var_ptr, child.get(), depth + 1);
        }
    }
};

#endif