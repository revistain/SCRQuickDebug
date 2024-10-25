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

    // 올바르게 child를 추가하는 함수
    std::shared_ptr<TreeNode> addChild(TreeNode* _parent, uint32_t _func_number, uint32_t _func_time) {
        // std::make_shared로 자식을 생성
        std::shared_ptr<TreeNode> child = std::make_shared<TreeNode>(_func_number, _func_time);
        child->parent = _parent;  // 부모 노드 설정

        childs.push_back(child);  // 자식 노드 추가
        return child;
    }

    // 자식을 찾는 함수
    TreeNode* findChild(uint32_t func_number) {
        for (auto& child : childs) {
            if (child->func_number == func_number) {
                return child.get();  // 자식의 raw 포인터 반환
            }
        }
        return nullptr;
    }
} TreeNode;

class Tree {
public:
    std::shared_ptr<TreeNode> root;  // 트리의 루트 노드
    Tree() {
        root = std::make_shared<TreeNode>(0xEDACEDAC, 0);
    }

    void printTree(std::unique_ptr<Variables>& var_ptr, TreeNode* node, uint32_t depth=0) const {
        if (!node) return;

        // 깊이에 따라 들여쓰기
        for (int i = 0; i < depth; ++i) {
            std::cout << "  ";
        }

        // 현재 노드의 데이터 출력
        if(node->func_number != 0xEDACEDAC) std::cout << std::dec << var_ptr->strtable.str[node->func_number+var_ptr->functrace.offset] << " / " << node->func_time << std::endl;

        // 자식 노드들 출력
        for (const auto& child : node->childs) {
            printTree(var_ptr, child.get(), depth + 1);
        }
    }
};

#endif