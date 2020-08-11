//
// Created by thecm on 8/4/2020.
//
#define _ITERATOR_DEBUG_LEVEL 0
#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <numeric>
#include <map>
#include "statemachine.h"
#include <memory>
#include <fstream>

using std::unordered_map;
using std::vector;
using std::cout;
using std::endl;
#if 0
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
#endif

std::ofstream out_fl("EndMovesTree.txt");

class EndMovesTree{
private: // Member Classes
    class EndMovesTreeNode{
    private: //Private data members
        std::shared_ptr<EndMovesTreeNode> up = nullptr;
        std::shared_ptr<EndMovesTreeNode> left = nullptr;
        std::shared_ptr<EndMovesTreeNode> down = nullptr;
        std::shared_ptr<EndMovesTreeNode> right = nullptr;
        std::shared_ptr<EndMovesTreeNode> parent, left_child= nullptr, right_child = nullptr;
        vector<vector<int>>state;
        int depth, empty_row, empty_col;

    public: // Public Static Functions
        static std::string stateToKey(const vector<vector<int>>& state, int indent_level=0) {
            std::string key;
            int i=0,j;
            for (const auto& row : state) {
                j=0;
                for (int k = 0; k < row.size() * indent_level; k++)
                    key.append("-");
                for (const auto& num : row)
                    if (j++ < state.at(0).size() - 1)
                        key.append(std::to_string(num)).append(" ");
                    else key.append(std::to_string(num));
                if (i++ < state.size()-1)
                    key.append("\n");
            }
            return key;
        }

    public: //Public Functions
        friend class EndMovesTree;
        std::shared_ptr<EndMovesTreeNode> nextState(){
            return parent;
        }
        [[nodiscard]] int nextMove() const{
            if (parent && parent->empty_row > -1 && parent->empty_col > -1)
                return state.at(parent->empty_row).at(parent->empty_col);
            return 0;
        }

    public: //Constructors, Destructors, & Operators
        friend std::ostream& operator<< (std::ostream& os, EndMovesTreeNode& tn){
            return os <<  stateToKey(tn.state) << "\n";
        }
        explicit EndMovesTreeNode(vector<vector<int>> state, std::shared_ptr<EndMovesTreeNode> parent= nullptr, int empty_row= -1, int empty_col= -1, int depth= -1)
                : state(std::move(state)), parent(std::move(parent)), empty_row(empty_row), empty_col(empty_col), depth(depth){
            //cout << "Created State at ("<<this<<"): \n" << *this;
        }
        ~EndMovesTreeNode(){
            //cout << "deleted state at ("<<this<<"): \n" << *this << endl;
        };

        EndMovesTreeNode(const EndMovesTreeNode& node) = default;

        EndMovesTreeNode(EndMovesTreeNode&& other) = default; /*noexcept {
            cout << "Moved state to ("<<this<<"): \n" << other;
            this->up = std::exchange(other.up, nullptr);
            this->left = std::exchange(other.left, nullptr);
            this->right = std::exchange(other.right, nullptr);
            this->down = std::exchange(other.down, nullptr);
            this->parent = std::exchange(other.parent, nullptr);
            this->state = std::move(other.state);
            this->depth = std::exchange(other.depth, -1);
            this->empty_row = std::exchange(other.empty_row, -1);
            this->empty_col = std::exchange(other.empty_row, -1);
        }*/
    };

private: // Private data members
    int node_count{};
    std::shared_ptr<EndMovesTreeNode> end_node;
    unordered_map<std::string, std::shared_ptr<EndMovesTreeNode>> substates;
    vector<vector<int>> start_state, win_state;
    std::map<vector<vector<int>>, bool> verify;

private: // Private Functions
    std::shared_ptr<EndMovesTreeNode> buildStateTree(const vector<vector<int>>& state, int& node_num,
                                                     int empty_row_num = -1, int empty_col_num = -1,
                                                     const std::shared_ptr<EndMovesTreeNode>& parent = nullptr, int depth= 0) {
        bool flag=false;
        if (EndMovesTreeNode::stateToKey(state)==EndMovesTreeNode::stateToKey(generateWinState(state))){
            flag= true;
            cout << '\0';
        }

        this->node_count = node_num;
        empty_row_num = empty_row_num < 0 ? (int)state.size()-1 : empty_row_num;
        int temp = state[0].size();
        empty_col_num = empty_col_num < 0 ? (int)state[0].size()-1 : empty_col_num;
        std::string key = EndMovesTreeNode::stateToKey(state);
        if (substates.count(key)){
            std::weak_ptr<EndMovesTreeNode> tn = substates.at(key);
            auto ref_count = tn.use_count();
            auto temp = tn.lock();

            if (depth < temp->depth){
                temp->depth = depth;

                if (temp->parent) {
                    if (!temp->parent->left_child)
                        temp->parent->left_child = parent;
                    else if (!temp->parent->right_child)
                        temp->parent->right_child = parent;
                    else if (temp->parent->left_child == temp)
                        temp->parent->left_child = nullptr;
                    else if (temp->parent->right_child == temp)
                        temp->parent->right_child = nullptr;
                }else {
                    cout << '\0';
                }

                temp->parent = parent;
            }
            return substates.at(key);
        }
        else substates[key] = std::make_shared<EndMovesTreeNode>(std::move(EndMovesTreeNode{state, parent, empty_row_num, empty_col_num, depth}));
        //out_fl << EndMovesTreeNode::stateToKey(substates.at(key)->state, depth) << endl;
        if (parent.get()) {
            if (!parent->left_child)
                parent->left_child = substates.at(key);
            else if (!parent->right_child)
                parent->right_child = substates.at(key);
        }else {
            cout << '\0';
        }
        verify[state] = true;
        ++node_count;
        if (state.at(empty_row_num).at(empty_col_num)==0){
            //left
            if (empty_col_num > 0){
                vector<vector<int>> left=state;
                left[empty_row_num][empty_col_num] = std::exchange(left[empty_row_num][empty_col_num-1],0);
                substates.at(key)->left = buildStateTree(left, node_count, empty_row_num, empty_col_num - 1, substates.at(key),depth+1);

            }
            //up
            if (empty_row_num > 0){
                vector<vector<int>> up=state;
                up[empty_row_num][empty_col_num] = std::exchange(up[empty_row_num-1][empty_col_num],0);
                substates.at(key)->up = buildStateTree(up, node_count,empty_row_num - 1, empty_col_num, substates.at(key), depth + 1);

            }
            //right
            if (empty_col_num < state.at(0).size()-1){
                vector<vector<int>> right=state;
                right[empty_row_num][empty_col_num] = std::exchange(right[empty_row_num][empty_col_num+1], 0);
                substates.at(key)->right = buildStateTree(right, node_count, empty_row_num, empty_col_num + 1, substates.at(key),depth+1);

            }
            //down
            if (empty_row_num < state.size()-1){
                vector<vector<int>> down=state;
                down[empty_row_num][empty_col_num] = std::exchange(down[empty_row_num+1][empty_col_num], 0);
                substates.at(key)->down = buildStateTree(down, node_count, empty_row_num + 1, empty_col_num, substates.at(key),depth+1);

            }

        }
        else{
            cout << '\0';
        }
        return substates.at(key);
    }

public: // Public Static Functions
    static vector<vector<int>> generateWinState(const vector<vector<int>>& state){
        vector<int> choices;
        vector<vector<int>> res;
        res.reserve(state.size());
        for (const auto& row : state)
            choices.insert(choices.end(), row.begin(), row.end());
        std::sort(choices.begin(), choices.end(), [](int& a, int& b){return a != 0 && (!b || a < b);});
        int size = state.at(0).size();

        for (auto range_start = choices.begin(), range_end = choices.begin() + size; range_start != choices.end(); range_start+=size, range_end+=size) {
            res.emplace_back(range_start, range_end);
        }
        return res;
    }
    static vector<vector<int>> generateEndState(const int& size, const int& row_len = 3, const int& col_len = 2){
        vector<vector<int>> endstate;
        endstate.reserve(col_len);
        for (int row_num = size - col_len; row_num < size; row_num++){
            vector<int>row(row_len);
            std::iota(row.begin(), row.end(), size * row_num + 1);
            endstate.emplace_back(row.begin(), row.end());
        }
        endstate[col_len-1][row_len-1] = 0;
        return endstate;
    }
    static void printEndMovesTree(const EndMovesTreeNode* head, int depth=0, std::ostream& os=std::cout){
        std::weak_ptr<EndMovesTreeNode> weak_rc = head->right_child, weak_lc = head->left_child;
        if (head->left_child.get())
            printEndMovesTree(head->left_child.get(), depth+1, os);
        else
            cout << '\0';
        os << EndMovesTreeNode::stateToKey(head->state, depth) << endl << endl;
        if (head->right_child.get())
            printEndMovesTree(head->right_child.get(), depth+1, os);
        else
            cout << '\0';
    }
#if 0
    static vector<vector<int>> generateEndState(const vector<vector<std::shared_ptr<GridNode>>>& grid)  {
        std::vector<vector<int>> remaining(2);
        auto top_start_it = grid.at(grid.size() - 2).end() - 3;
        auto top_end_it = grid.at(grid.size() - 2).end();
        auto bottom_start_it = grid.at(grid.size() - 1).end() - 3;
        auto bottom_end_it = grid.at(grid.size() - 1).end();
        std::for_each(top_start_it, top_end_it, [&remaining](const std::shared_ptr<GridNode> &node) {
            remaining.at(0).emplace_back(node->val);
        });
        std::for_each(bottom_start_it, bottom_end_it, [&remaining](const std::shared_ptr<GridNode> &node) {
            remaining.at(1).emplace_back(node->val);
        });
        return remaining;
    }
#endif
public: // Public functions
    [[nodiscard]] const std::map<vector<vector<int>>, bool>& getAllStates() const{
        return static_cast<const std::map<vector<vector<int>>, bool>&>(verify);
    }
    [[nodiscard]] const vector<vector<int>>& getStartState() const{
        return static_cast<const vector<vector<int>> &>(start_state);
    }
    [[nodiscard]] const vector<vector<int>>& getWinState() const{
        return static_cast<const vector<vector<int>> &>(win_state);
    }
    [[nodiscard]] vector<int> generate_moveslist(vector<vector<int>>* state_ptr=nullptr) const{
        const auto starting_state = state_ptr ? *state_ptr : getStartState();
        vector<int> moveslist;
        int num_moves = 0;
        std::string key = EndMovesTreeNode::stateToKey(starting_state);
        if (substates.count(key)) {
            std::shared_ptr<EndMovesTreeNode>traveller = substates.at(key);
            while (traveller) {
                //cout << *traveller;
                if (traveller->nextMove() > 0) {
                    //cout << "Next move: {" << traveller->nextMove() << "}\n\n";
                    moveslist.emplace_back(traveller->nextMove());
                }
                traveller = traveller->nextState();
                ++num_moves;
            }
        }else moveslist = {0};
        //cout << num_moves << " moves were made.\n";
        return moveslist;
    }


public: //Constructors, Destructors, & Operators
    friend std::ostream& operator<< (std::ostream& os, const EndMovesTree& emt){
        printEndMovesTree(emt.end_node.get(),0,os);
        return os;
    }

    explicit EndMovesTree(vector<vector<int>> start_state): node_count(0), start_state(std::move(start_state)){
        win_state = std::move(generateWinState(this->start_state));
        verify = [this]() -> std::map<vector<vector<int>>, bool>{
            vector<int> choices;
            std::for_each(this->start_state.begin(), this->start_state.end(),[&choices](auto& row)mutable{
                choices.insert(choices.end(),row.begin(), row.end());
            });
            vector<vector<vector<int>>> permutations;
            permutations.reserve(720);
            std::sort(choices.begin(), choices.end());
            do{
                vector<vector<int>> generated_state = {
                        {choices.begin(), choices.begin() + choices.size() / 2},
                        {choices.begin() + choices.size() / 2, choices.end()}
                };
                permutations.emplace_back(std::move(generated_state));
            }while(std::next_permutation(choices.begin(), choices.end()));
            std::map<vector<vector<int>>, bool>verify;
            std::for_each(permutations.begin(), permutations.end(), [&verify](auto& state){  verify[std::move(state)] = false; });
            return verify;
        }();
        end_node = buildStateTree(win_state, node_count);
    }
    EndMovesTree(const int n): node_count(0), start_state(std::move(generateEndState(n))){
        win_state = start_state;
        end_node = buildStateTree(win_state, node_count);
    }
    explicit EndMovesTree() = default;
    ~EndMovesTree(){
        //cout << "Deleted Endstate: \n" << EndMovesTreeNode::stateToKey(start_state) << endl;
        substates.clear();
    }
};


int main() {
    vector<vector<int>> state = {{7, 4, 6},
                                 {0, 5, 8}};


    EndMovesTree endSolver(state);
    //out_fl << endSolver;
    const auto& allStates = endSolver.getAllStates();
    int i=0;
    for (const auto & allStates_pair : allStates){
        if (allStates_pair.second) {
            ++i;
            for (const auto& row : allStates_pair.first) {
                for (const auto &num : row) {
                    //cout << num << "\t";

                }
                //cout << "\n\n";
            }
        }
    }

    auto moveslist = endSolver.generate_moveslist();
    cout << '\0';
}