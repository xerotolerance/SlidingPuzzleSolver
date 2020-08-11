//
// Created by thecm on 8/9/2020.
//
#define _ITERATOR_DEBUG_LEVEL 0
#include "EndPuzzleSolverSingleton.h"
#include <unordered_map>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <numeric>

#define DEBUG 0
using std::vector;
using std::cout;
using State = vector<vector<int>>;




class EndPuzzleSolverSingleton {
    class GraphNode{
        class GraphNodeMultiMapComparator{
        public:
            bool operator()(const int& depth_A, const int& depth_B) const{
                return depth_A >= 0 && (depth_B < 0 || depth_A < depth_B);
            }
        };
        GraphNode *up= nullptr, *down=nullptr, *left= nullptr, *right= nullptr;
        bool visited = false;
        State state;
        int empty_space_row_num, empty_space_col_num, depth=-1;
        std::string key;

    public:
        friend class EndPuzzleSolverSingleton;
        friend std::ostream& operator<<(std::ostream& os, const GraphNode* ptr){
            if (ptr)
                os << ptr->key;
            else os << "{NULLPTR}";
            return os;
        }
        friend std::ostream& operator<<(std::ostream& os, const GraphNode& obj){
            os << obj.key;
            return os;
        }
        static std::string stateToKey(const State& state, int indent_level=0) {
            std::string key;
            int i=0,j;
            for (const auto& row : state) {
                j=0;
                key.append(indent_level>0? indent_level-1 : 0, ' ');
                if (indent_level>0)
                    key.append("/");
                for (const auto& num : row) {
                    if (++j < row.size())
                        key.append(std::to_string(num)).append(" ");
                    else key.append(std::to_string(num));
                }
                if (++i < state.size())
                    key.append("\n");
            }
            return key;
        }

        // Complete State Constructor
        explicit GraphNode(State state, int e_row, int e_col, GraphNode* up= nullptr, GraphNode* down= nullptr, GraphNode* left= nullptr, GraphNode* right=nullptr)
                : state(std::move(state)), empty_space_row_num(e_row), empty_space_col_num(e_col), up(up), down(down), left(left), right(right){
            key = stateToKey(this->state);
        }

        // Incomplete State Constructor (Unspecified Empty Space Location)
        explicit GraphNode(State state, GraphNode* up= nullptr, GraphNode* down= nullptr, GraphNode* left= nullptr, GraphNode* right=nullptr)
                : state(std::move(state)), empty_space_row_num(-1), empty_space_col_num(-1), up(up), down(down), left(left), right(right){
            key = stateToKey(this->state);
        }

        // Unspecified State Constructor (Specified Empty Space Location)
        explicit GraphNode(int e_row, int e_col, GraphNode* up= nullptr, GraphNode* down= nullptr, GraphNode* left= nullptr, GraphNode* right=nullptr)
                : empty_space_row_num(e_row), empty_space_col_num(e_col){ updatePointers(up, down, left, right);}

        // Empty State Constructor
        explicit GraphNode(GraphNode* up= nullptr, GraphNode* down= nullptr, GraphNode* left= nullptr, GraphNode* right=nullptr)
                : empty_space_row_num(-1), empty_space_col_num(-1){ updatePointers(up, down, left, right);}


        void updatePointers(GraphNode* new_up= nullptr, GraphNode* new_down= nullptr, GraphNode* new_left= nullptr, GraphNode* new_right= nullptr){
            this->up = new_up;
            this->down = new_down;
            this->left = new_left;
            this->right = new_right;
        }
        std::multimap<int, GraphNode*, GraphNodeMultiMapComparator> getPaths() const{
            std::multimap<int, GraphNode*, GraphNodeMultiMapComparator> paths{
                    {(up ? up->depth : -1),       up},
                    {(down ? down->depth : -1),   down},
                    {(left ? left->depth : -1),   left},
                    {(right ? right->depth : -1), right}
            };
            return paths;
        }
    };
    GraphNode* win_node;
    State win_state;
    std::unordered_map<std::string, GraphNode> all_states;
    int graph_size=0, viable_state_count=0, invalid_state_count=0;
private:
    int linkStates(std::unordered_map<std::string, GraphNode>* states = nullptr){
        states = states? states : &all_states;
        int state_count = 0;
        for (auto& [key, node] : *states){
            State up, down, left, right;
            //Right
            if (node.empty_space_col_num < node.state.at(0).size()-1){
                right = node.state;
                int temp = right[node.empty_space_row_num][node.empty_space_col_num+1];
                right[node.empty_space_row_num][node.empty_space_col_num+1] = right[node.empty_space_row_num][node.empty_space_col_num];
                right[node.empty_space_row_num][node.empty_space_col_num] = temp;
                node.right = &all_states.at(GraphNode::stateToKey(right));
            }
            //Left
            if (node.empty_space_col_num > 0){
                left = node.state;
                int temp = left[node.empty_space_row_num][node.empty_space_col_num-1];
                left[node.empty_space_row_num][node.empty_space_col_num-1] = left[node.empty_space_row_num][node.empty_space_col_num];
                left[node.empty_space_row_num][node.empty_space_col_num] = temp;
                node.left = &all_states.at(GraphNode::stateToKey(left));
            }
            //Down
            if (node.empty_space_row_num < node.state.size()-1){
                down = node.state;
                int temp = down[node.empty_space_row_num+1][node.empty_space_col_num];
                down[node.empty_space_row_num+1][node.empty_space_col_num] = down[node.empty_space_row_num][node.empty_space_col_num];
                down[node.empty_space_row_num][node.empty_space_col_num] = temp;
                node.down = &all_states.at(GraphNode::stateToKey(down));
            }
            //UP
            if (node.empty_space_row_num > 0){
                up = node.state;
                int temp = up[node.empty_space_row_num-1][node.empty_space_col_num];
                up[node.empty_space_row_num-1][node.empty_space_col_num] = up[node.empty_space_row_num][node.empty_space_col_num];
                up[node.empty_space_row_num][node.empty_space_col_num] = temp;
                node.up = &all_states.at(GraphNode::stateToKey(up));
            }
            ++state_count;
        }
        return state_count;
    }
    int rankNodes_recursive(GraphNode* head= nullptr, GraphNode* prev= nullptr){
        //cout << head->key << "\n";
        int nodes_visited = 0;
        head = head ? head : win_node;
        head->depth = head->depth < 0 ? 0 : head->depth;
        if (!head->visited)
            ++nodes_visited;
        head->visited=true;

        // Rank adjacent nodes w/ shortest path to win state
        for (auto& [child_depth, child_node] : head->getPaths()) {
            if ( child_depth > head->depth || child_node && child_depth < 0) {
                child_node->depth = head->depth + 1;
            }
        }
        /* Visit node which were (re-)assigned path distances during this call
           and do the same*/
        for (auto& [directional_node_depth, directional_node] : head->getPaths()) {
            if (directional_node_depth == head->depth + 1)
                nodes_visited += rankNodes_recursive(directional_node, head);
        }
        return nodes_visited;
    }
    int rankNodes(GraphNode* head= nullptr){
        head = head ? head : win_node;
        head->depth = head->depth < 0 ? 0 : head->depth;
        int nodes_discovered = rankNodes_recursive(head);
        for (auto& [key, node] : all_states){
            if (node.depth < 0){
                for (auto& [next_depth, next_node] : node.getPaths()) {
                    if (next_depth > -1)
                        cout << '\0';
                    if (next_depth > -1 && ((node.depth < 0) || (next_depth < node.depth)))
                        node.depth = next_depth + 1;
                }
                if (node.depth > -1)
                    ++nodes_discovered;
                else ++invalid_state_count;
            }
        }
        return nodes_discovered;
    }
    vector<int> solve(GraphNode* node=nullptr){
        static vector<int> moves;
        if (node == win_node) {
            auto res = moves;
            moves.clear();
            return res;
        }
        node = node? node : win_node;
        for (const auto& [depth, next_node] : node->getPaths()){
            if (depth < 0)
                continue;
            moves.push_back(node->state[next_node->empty_space_row_num][next_node->empty_space_col_num]);
            auto res = solve(next_node);
            if (res != vector<int>{0})
                return res;
            moves.pop_back();
        }
        return {0};
    }
    explicit EndPuzzleSolverSingleton(const State& start_state): win_state(generateWinState(start_state)), all_states(generateAllStates(this)) {
        win_node = &all_states.at(GraphNode::stateToKey(win_state));
        win_node->depth=0;
        graph_size = linkStates();
        viable_state_count = rankNodes();
    }
public:
    static EndPuzzleSolverSingleton& get(){
        static auto instance = EndPuzzleSolverSingleton({{4, 5, 6}, {7, 8, 0}});
        return instance;
    }
    EndPuzzleSolverSingleton(const EndPuzzleSolverSingleton &other) = delete;
    EndPuzzleSolverSingleton(const EndPuzzleSolverSingleton &&other) = delete;
    EndPuzzleSolverSingleton operator=(const EndPuzzleSolverSingleton& other) = delete;

    static std::unordered_map<std::string, GraphNode> generateAllStates(const EndPuzzleSolverSingleton* om){
            vector<int> choices;
            std::for_each(om->win_state.begin(), om->win_state.end(), [&choices](auto &row)mutable {
                choices.insert(choices.end(), row.begin(), row.end());
            });
            std::unordered_map<std::string,GraphNode> states;
            std::sort(choices.begin(), choices.end());
            do {
                State generated_state;
                auto range_start = choices.begin(), range_end = choices.begin()+om->win_state.at(0).size();
                while (range_start != choices.end()){
                    generated_state.emplace_back(range_start, range_end);
                    range_start += om->win_state.at(0).size();
                    range_end += om->win_state.at(0).size();
                }
                auto str_rep = GraphNode::stateToKey(generated_state);
                auto empty_locataion = [&generated_state]() -> std::pair<int,int> {
                    for (int row = 0; row < generated_state.size(); row++)
                        for (int col = 0; col < generated_state.at(row).size(); col++)
                            if (generated_state.at(row).at(col) == 0)
                                return {row, col};
                    return {-1, -1};
                }();
                states[str_rep] = GraphNode(generated_state, empty_locataion.first, empty_locataion.second);
            } while (std::next_permutation(choices.begin(), choices.end()));
            return states;
    }
    static State generateEndState(const State& state, int min_num_rows=2, int min_num_cols=3){
        if (state.size() < min_num_rows || state.at(0).size() < min_num_cols)
            return {{0}};
        State end_state;
        std::for_each(state.end()-min_num_rows, state.end(), [&end_state, min_num_cols](const vector<int>& row){
            end_state.emplace_back(row.end()-min_num_cols, row.end());
        });
        return end_state;
    }
    static State generateWinState(const State& start_state, int min_num_rows=2, int min_num_cols=3){
        State state = generateEndState(start_state, min_num_rows, min_num_cols);
        if (state.size()!=min_num_rows || state.at(0).size() !=min_num_cols)
            return state;

        vector<int> choices;
        State res;
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
    vector<int> solve(State start_state){
        auto target_state = generateWinState(start_state);
        auto offset = target_state[0][0] - win_state[0][0];
        for (auto& row : start_state)
            for (auto& num : row)
                ++num = num == 0? num : num-offset;
        auto* node = &all_states.at(GraphNode::stateToKey(start_state));
        auto moveslist = solve(node);
        std::for_each(moveslist.begin(), moveslist.end(), [&offset](auto& move){move+=offset;});
        return moveslist;
    }

    friend std::ostream& operator<< (std::ostream& os, const EndPuzzleSolverSingleton& pss){
        os << "Currently set to solve " << pss.win_state.size() << "-by-" << pss.win_state.at(0).size() << " end states.\n";
        os << "Found " << pss.viable_state_count << " winnable states\n";
        os << "\tand " << pss.invalid_state_count << " non-winnable states\n";
        os << "Total nodes explored: " << pss.viable_state_count + pss.invalid_state_count << " out of " << pss.graph_size << ".\n";
        return os << std::endl;
    }

};


int main(){
    vector<State> states = {
            {{8,7,6},{5,4,0}},
            {{9,8,7},{6,5,0}},
            {{10,9,8},{7,6,0}},
    };
    auto printMoves = [](const auto &moveslist) {
        int i = 0;
        cout << "{";
        for (const auto &move : moveslist) {
            cout << move;
            if (++i < moveslist.size())
                cout << ", ";
        }
        cout << "}\n\n";
    };
    cout << EndPuzzleSolverSingleton::get();
    for (const auto& state : states) {
        auto moveslist = EndPuzzleSolverSingleton::get().solve(state);
        printMoves(moveslist);
    }
    std::cin.get();
}