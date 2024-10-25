#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <list>
#include <vector>
#include <cstdint>
#include <stack>
#include <unordered_map>
#include <iostream>
#include <windows.h>
#include <ctime>
#include <atomic>
#include <chrono>
#include <thread>
#include <memory> 
#include "variable.h"

void ensureEndTimestampThread();
void analyzeTimeStamp(std::unique_ptr<Variables>& var_ptr);

void startTimeStamp(std::unique_ptr<Variables>& var_ptr);
void endTimeStamp(std::unique_ptr<Variables>& var_ptr);

typedef struct TreeNode {
    uint32_t func_number;
    uint32_t func_time;
    uint32_t func_current_delta;
    uint32_t call_count;
    uint32_t depth;
    uint32_t padding;
    TreeNode* parent;
    std::list<TreeNode> childs;

    TreeNode(uint32_t _func_number, uint32_t _func_time, uint32_t _func_current_delta, uint32_t _depth, uint32_t _padding)
        : func_number(_func_number), func_time(_func_time), func_current_delta(_func_current_delta), call_count(0), depth(_depth), padding(_padding), parent(nullptr) {}

    TreeNode* addChild(TreeNode* _parent, uint32_t _func_number, uint32_t _func_time, uint32_t _depth, uint32_t _padding) {
        // std::make_shared로 자식을 생성
        TreeNode child = TreeNode(_func_number, _func_time, 0, _depth, _padding);
        child.parent = _parent;  // 부모 노드 설정

        childs.push_back(child);  // 자식 노드 추가
        return &childs.back();
    }

    // 자식을 찾는 함수
    TreeNode* findChild(uint32_t func_number) {
        for (auto& child : childs) {
            if (child.func_number == func_number) {
                return &child;  // 자식의 raw 포인터 반환
            }
        }
        return nullptr;
    }
} TreeNode;


class Tree {
public:
    TreeNode root;  // 트리의 루트 노드
    uint32_t starttime;
    uint32_t endtime;
    Tree(): starttime(0), endtime(0), root(TreeNode(0xEDACEDAC, 0, 0, 0, 0)){
    }

    std::list<TreeNode>& getFirstChilds() {
        return root.childs;
    }
    void rflattenTree(std::vector<TreeNode*>& nodes, TreeNode* node) {
        if (!node) return;

        if(node->func_current_delta >= 1) nodes.push_back(node);
        for (auto& child : node->childs) {
            rflattenTree(nodes, &child);
        }
    }
    std::vector<TreeNode*> flattenTree() {
        std::vector<TreeNode*> retNodes;
        rflattenTree(retNodes, &root);
        return retNodes;
    }

    void printTree(std::unique_ptr<Variables>& var_ptr, TreeNode* node, uint32_t depth=0) const {
        if (!node) return;

        // 깊이에 따라 들여쓰기
        for (int i = 0; i < depth; ++i) {
            std::cout << "  ";
        }

        // 현재 노드의 데이터 출력
        if (node->func_number != 0xEDACEDAC) {
            if (node->call_count != 0) {
                std::cout << std::dec << var_ptr->strtable.str[node->func_number] << " / input" << std::endl;
            }
            else {
                std::cout << std::dec << var_ptr->strtable.str[node->func_number] << " / " << 0 << std::endl;
            }
        }

        // 자식 노드들 출력
        for (auto& child : node->childs) {
            printTree(var_ptr, &child, depth + 1);
        }

        for (int i = 0; i < depth; ++i) {
            std::cout << "  ";
        }

        if (node->func_number != 0xEDACEDAC) {
            if (node->call_count != 0) {
                std::cout << std::dec << var_ptr->strtable.str[node->func_number] << " / " << (uint32_t)(node->func_time / node->call_count) << " / padd: " << node->padding << ",delta: " << node->func_current_delta << std::endl;
            }
            else {
                std::cout << std::dec << var_ptr->strtable.str[node->func_number] << " / " << 0 << std::endl;
            }
        }
    }
};

Tree& getTree();
#endif