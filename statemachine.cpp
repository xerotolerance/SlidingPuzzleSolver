//
// Created by thecm on 8/4/2020.
//
#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "statemachine.h"

using std::unordered_map;
using std::vector;
using std::cout;

class EndMovesTreeNode{
    EndMovesTreeNode* up = nullptr;
    EndMovesTreeNode* left = nullptr;
    EndMovesTreeNode * down = nullptr;
    EndMovesTreeNode * right = nullptr;
    EndMovesTreeNode* back;
    vector<vector<int>>state;
    int depth, empty_row, empty_col;
public:
    static int node_count;
    static std::string stateToKey(const vector<vector<int>>& state) {
        std::string key;
        int i=0,j;
        for (const auto& row : state) {
            j=0;
            for (const auto& num : row)
                if (j++ < state.at(0).size() - 1)
                    key.append(std::to_string(num)).append(" ");
                else key.append(std::to_string(num));
            if (i++ < state.size()-1)
                key.append("\n");
        }
        return key;
    }
    friend std::ostream& operator<< (std::ostream& os, EndMovesTreeNode& tn){
        return os <<  stateToKey(tn.state) << "\n";
    }
    friend class EndMovesTree;

    EndMovesTreeNode* next_state(){
        return back;
    }

    int next_move(){
        if (back && back->empty_row > -1 && back->empty_col > -1)
            return state.at(back->empty_row).at(back->empty_col);
        return 0;
    }

    explicit EndMovesTreeNode(vector<vector<int>> state, EndMovesTreeNode* back= nullptr, int empty_row=-1, int empty_col=-1, int depth=-1)
    : state(std::move(state)), back(back), empty_row(empty_row), empty_col(empty_col), depth(depth){
        ++EndMovesTreeNode::node_count;
    }
    ~EndMovesTreeNode(){
        delete(up);
        delete(left);
        delete(down);
        delete(right);
    }
};
int EndMovesTreeNode::node_count{0};

class [[maybe_unused]] EndMovesTree{
    EndMovesTreeNode* end_node;
    unordered_map<std::string, EndMovesTreeNode*> substates;
    vector<vector<int>> start_state, win_state;

    EndMovesTreeNode * buildStateTree(const vector<vector<int>>& state, int empty_row_num = -1, int empty_col_num = -1, EndMovesTreeNode *back = nullptr, int depth=0) {
        empty_row_num = empty_row_num < 0 ? state.size()-1 : empty_row_num;
        empty_col_num = empty_col_num < 0 ? state.at(0).size()-1 : empty_col_num;
        std::string key = EndMovesTreeNode::stateToKey(state);
        if (substates.count(key)){
            auto tn = substates.at(key);
            if (depth < tn->depth){
                tn->depth = depth;
                tn->back = back;
            }
            return tn;
        }
        else substates[key] = new EndMovesTreeNode{state, back, empty_row_num, empty_col_num, depth};

        if (state.at(empty_row_num).at(empty_col_num)==0){
            if (empty_row_num > 0){
                vector<vector<int>> up=state;
                up[empty_row_num][empty_col_num] = std::exchange(up[empty_row_num-1][empty_col_num],0);
                substates.at(key)->up = buildStateTree(up, empty_row_num - 1, empty_col_num, substates.at(key), depth + 1);
            }
            if (empty_col_num > 0){
                vector<vector<int>> left=state;
                left[empty_row_num][empty_col_num] = std::exchange(left[empty_row_num][empty_col_num-1],0);
                substates.at(key)->left = buildStateTree(left, empty_row_num, empty_col_num - 1, substates.at(key),depth+1);
            }
            if (empty_row_num < state.size()-1){
                vector<vector<int>> down=state;
                down[empty_row_num][empty_col_num] = std::exchange(down[empty_row_num+1][empty_col_num], 0);
                substates.at(key)->down = buildStateTree(down, empty_row_num + 1, empty_col_num, substates.at(key),depth+1);
            }
            if (empty_col_num < state.at(0).size()-1){
                vector<vector<int>> right=state;
                right[empty_row_num][empty_col_num] = std::exchange(right[empty_row_num][empty_col_num+1], 0);
                substates.at(key)->right = buildStateTree(right, empty_row_num, empty_col_num + 1, substates.at(key),depth+1);
            }
        }
        else{
            cout << '\0';
        }
        return substates.at(key);
    }
public:
    static vector<vector<int>> generate_win_state(const vector<vector<int>>& state){
        vector<int> choices;
        vector<vector<int>> res;
        res.reserve(state.size());
        for (const auto& row : state)
            choices.insert(choices.end(), row.begin(), row.end());
        std::sort(choices.begin(), choices.end(), [](int& a, int& b){return a != 0 && (!b || a < b);});
        int size = state.at(0).size();
        for (auto start_it = choices.begin(), end_it = choices.begin()+size; start_it != choices.end(); start_it+=size, end_it+=size)
            res.emplace_back(start_it, end_it);
        return res;
    }

    vector<int> generate_moveslist(){
        vector<int> moveslist;
        int num_moves = 0;
        std::string key = EndMovesTreeNode::stateToKey(start_state);
        if (substates.count(key)) {
            EndMovesTreeNode *traveller = substates.at(key);
            while (traveller) {
                cout << *traveller;
                if (traveller->next_move() > 0) {
                    cout << "Next move: {" << traveller->next_move() << "}\n\n";
                    moveslist.emplace_back(traveller->next_move());
                }
                traveller = traveller->next_state();
                ++num_moves;
            }
        }else moveslist = {0};
        cout << num_moves << " moves were made.\n";
        return moveslist;
    }

    explicit EndMovesTree(vector<vector<int>> start_state): start_state(std::move(start_state)){
        win_state = std::move(generate_win_state(this->start_state));
        end_node = buildStateTree(win_state);
    }
    ~EndMovesTree(){
        delete(end_node);
    }
};

int main() {
    vector<vector<int>> state = {{7, 4, 6},
                                 {0, 5, 8}};
    EndMovesTree endSolver(state);
    auto moveslist = endSolver.generate_moveslist();
    cout << '\0';
}