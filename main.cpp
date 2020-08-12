#define _ITERATOR_DEBUG_LEVEL 0
#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <iterator>
#include <functional>
#include <memory>
#include <chrono>
#include <string>
#include <numeric>
#include <algorithm>
#include <random>


using std::unordered_map;
using std::vector;
using std::cout;
using std::endl;


class SlidingPuzzleSolver{
private:    //Local Datatypes
    using State = vector<vector<int>>;
    class GridNode{
    private:
        friend SlidingPuzzleSolver;
        friend std::ostream& operator<<(std::ostream& os, const GridNode& node){
            return os << node.val;
        };
        std::shared_ptr<GridNode>up, right, down, left;
        int val, row, col;
        bool movable = true, visited=false;
    public:
        explicit GridNode(int val=-1, int row=-1, int col=-1, std::shared_ptr<GridNode> up= nullptr, std::shared_ptr<GridNode> right= nullptr, std::shared_ptr<GridNode> down= nullptr, std::shared_ptr<GridNode> left= nullptr)
                : val(val), row(row), col(col), up(std::move(up)), right(std::move(right)), down(std::move(down)), left(std::move(left)) {};
        GridNode (const GridNode& orig): val(orig.val), row(orig.row), col(orig.col), up(orig.up), right(orig.right), down(orig.down), left(orig.left) {};
        GridNode(GridNode&& src) noexcept
                : val(std::exchange(src.val, -1)), row(std::exchange(src.row, -1)), col(std::exchange(src.col, -1)),
                  up(std::exchange(src.up, nullptr)), right(std::exchange(src.right, nullptr)),
                  down(std::exchange(src.down, nullptr)), left(std::exchange(src.left, nullptr)){}
    };
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
            friend class SlidingPuzzleSolver::EndPuzzleSolverSingleton;
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
            [[nodiscard]] std::multimap<int, GraphNode*, GraphNodeMultiMapComparator> getPaths() const{
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
        SlidingPuzzleSolver::State win_state;
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
        vector<int> solve(const State& start_state){
            auto end_state = generateEndState(start_state);
            auto target_state = generateWinState(end_state);

            /* Transpose end state into base_end_state
             (do this to prevent having to hold multiple
              state maps (720 gridnodes each) in memory at once). */
            for (auto& row : end_state){
                for (auto& num : row) {
                    if (!num)
                        continue;
                    else if (num < (target_state.end() - 1)->at(0))
                        num -= (target_state.end() - 2)->at(0) - (this->win_state.end() - 2)->at(0);
                    else
                        num -= (target_state.end() - 1)->at(0) - (this->win_state.end() - 1)->at(0);
                }
            }

            auto* node = &all_states.at(GraphNode::stateToKey(end_state));
            auto moveslist = solve(node);

            // Transpose moves back to their original numbers
            std::for_each(moveslist.begin(), moveslist.end(), [&target_state, this](auto& move){
                if (move < (this->win_state.end()-1)->at(0) )
                    move += (target_state.end()-2)->at(0) - (this->win_state.end()-2)->at(0);
                else
                    move += (target_state.end()-1)->at(0) - (this->win_state.end()-1)->at(0);
            });
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
    enum direction{NONE=-1, UP, DOWN, LEFT, RIGHT};
    using Grid = std::vector<std::vector<std::shared_ptr<GridNode>>>;

private:    // Private data members
    const State& _board;
    Grid _gridnodes_by_position;
    std::unordered_map<int, std::shared_ptr<GridNode>> _gridnodes_by_value;
    std::vector<int> _movelist;
    std::shared_ptr<GridNode> _p_entry_point;
    const unsigned int _map_size;
    enum direction _backtrack_to = NONE;
    int _eject_me;
    bool printAll = false;

private:    // Private Functions
    void _print_board(std::ostream& os=std::cout, const std::string& end_dec = "==========================") const{
        std::shared_ptr<GridNode> row_head = _p_entry_point, item;
        while(row_head) {
            item = row_head;
            while (item){
                os << item->val << "\t";
                item = item->right;
            }
            os << "\n";
            row_head = row_head->down;
        }
        os << end_dec.c_str() << endl;
    }
    void _create_map() {
        int row=0, col=0;
        std::shared_ptr<GridNode>node_before=nullptr, node_above=nullptr;
        _gridnodes_by_position.reserve(_board.size());
        _gridnodes_by_value.reserve(_map_size);
        for (const auto& row_it : _board){
            col = 0;
            _gridnodes_by_position.emplace_back(std::vector<std::shared_ptr<GridNode>>());
            _gridnodes_by_position.back().reserve(_board.size());
            for (const int& item : row_it){
                _gridnodes_by_value.emplace(item, std::make_shared<GridNode>(std::move(GridNode(item, row, col, node_above, nullptr, nullptr, node_before))));
                _gridnodes_by_position[row].emplace_back(_gridnodes_by_value.at(item));
                if (node_before) {
                    node_before->right = _gridnodes_by_value.at(item);
                }
                if (node_above) {
                    node_above->down = _gridnodes_by_value.at(item);
                    node_above = node_above->right;
                }
                node_before = _gridnodes_by_value.at(item);
                ++col;
            }
            node_above = _gridnodes_by_value.at(*(row_it.begin()));
            node_before = nullptr;
            ++row;
        }
    }
    void _solve_row(int row_num){
        for (int col_num=0, num; col_num < _board.size(); col_num++) {
            num = row_num * (int)_board.size() + col_num + 1;
            // Steps for solving up to (but not including) last 2 elms of in a row:
            if (col_num < _board.size() - 2){
                _move_recursive(num, _gridnodes_by_position[row_num][col_num]->val);
                if (_gridnodes_by_position[row_num][col_num]->val == num)
                    _gridnodes_by_value.at(num)->movable = false;
            }
                // STEPS FOR SOLVING LAST 2 ELMS OF A ROW:
            else{
                /* Step 0: Check if row is solved. If it is, we're done. If not...
                 *
                 * Step 1: If last two elements of row are in each others' spot then move each out
                 *          of the Top-Right quadrant.
                 *      NOTE: It's okay if *one* of the elms remain in the quadrant
                 *           after both "ejects" have taken place. This step is meant to provide "room" to move
                 *           each of these elements independently of each other in the following steps.
                 *
                 * Step 2: If the number belonging in the 2nd-to-last spot of the row
                 *          is in the second-to-last position BEFORE it's been put in the last spot in the row,
                 *          then move that number outside of the Top-Right quadrant.
                 *          (Necessary in order to prepare for positioning the number belonging in LAST spot of row)
                 *
                 * Step 3: If the num belonging in the last spot of the row is already there,
                 *          check if it's possible to immediately move the number preceding it to the 2nd-to-last
                 *          spot in the row. This will solve the row and exit the function.
                 *             (Take this action whenever possible!).
                 *
                 *          If this is not possible, then eject num from the Top-Right quadrant.
                 *          (Again, necessary to set up number belonging in LAST spot of row)
                 *
                 * Step 4:  Put 2nd-to-last elm in the last spot in the row.
                 *
                 * Step 5:  Check if the board is deadlocked due to the empty space becoming trapped between
                 *          the 3rd-to-last in the row, the last solved row, the last-in-row,
                 *          & the 2nd-to-last in the row  (L,U,R,D respectively).
                 *
                 *          To break this deadlock, we mark 2nd-to-last in row as movable & eject last-in-row
                 *          from Top-Right quadrant. (Done efficiently by moving last-in-row one down from the center).
                 *          Then we can finally move 2nd-to-last back to the last spot of the row.
                 *
                 * Step 6: Put 2nd-to-last elm BACK in the last spot in the row &
                 *            Put last elm underneath 2nd-to-last elm
                 *
                 * Step 7-8: Rotate 2nd-to-last & last elms into their correct spots in the row.
                 *
                 *  >> Row Solve Complete <<
                */

                // STEPS 0-5
                if (col_num == _board.size() - 2){
                    // STEP 0:
                    if (num == _gridnodes_by_position[row_num][col_num]->val && num + 1 == _gridnodes_by_position[row_num][col_num + 1]->val){
                        _gridnodes_by_position[row_num][col_num]->movable = false;
                        _gridnodes_by_position[row_num][col_num + 1]->movable = false;
                        return;
                    }
                    // STEPS 1-3
                    else {
                        //STEP 1:
                        if (_gridnodes_by_value.at(num)->val == _gridnodes_by_position[row_num][col_num + 1]->val
                            && _gridnodes_by_value.at(num + 1)->val == _gridnodes_by_position[row_num][col_num]->val){
                            _move_recursive(num+1, _gridnodes_by_value.at(num + 1)->down->down->val, num);
                            _move_recursive(num, _gridnodes_by_value.at(num)->down->down->val, num);
                            if (num == _gridnodes_by_position[row_num][col_num]->val && num + 1 == _gridnodes_by_position[row_num][col_num + 1]->val){
                                _gridnodes_by_position[row_num][col_num]->movable = false;
                                _gridnodes_by_position[row_num][col_num + 1]->movable = false;
                                return;
                            }
                        }

                        bool second_to_last_in_row_is_correct = _gridnodes_by_position[row_num][col_num]->val == num;
                        bool last_in_row_is_correct = num + 1 == _gridnodes_by_position[row_num][col_num + 1]->val;

                        //STEP 2:
                        if (second_to_last_in_row_is_correct){
                            //Eject row's 2nd to last element
                            _move_recursive(num, _gridnodes_by_value.at(num)->down->down->val);
                        }

                        //STEP 3:
                        if (last_in_row_is_correct) {
                            if (_gridnodes_by_value.at(num + 1)->left->val == 0
                                && _gridnodes_by_value.at(0)->down->val == num) {
                                _swap_zero_with(num);
                                _gridnodes_by_position[row_num][col_num]->movable = false;
                                _gridnodes_by_position[row_num][col_num + 1]->movable = false;
                                return;
                            }
                                //Eject row's last element
                            else _move_recursive(num + 1, _gridnodes_by_value.at(num + 1)->down->down->val);
                        }
                    }

                    //STEP 4
                    _move_recursive(num, _gridnodes_by_position[row_num][col_num + 1]->val, num + 1, RIGHT);

                    //STEP 5
                    if (_gridnodes_by_value.at(num)->val == _gridnodes_by_position[row_num][col_num + 1]->val && _gridnodes_by_position[row_num + 1][col_num]->val == num + 1) {
                        _gridnodes_by_value.at(num)->movable = true;
                        _move_recursive(num+1, _gridnodes_by_value.at(num + 1)->down->val);
                        _move_recursive(num, _gridnodes_by_position[row_num][col_num + 1]->val);
                    }
                    if (_gridnodes_by_position[row_num][col_num + 1]->val == num) {
                        _gridnodes_by_value.at(num)->movable = false;
                    }
                }
                // STEP 6-8
                else{
                    //Step 6
                    _move_recursive(num, _gridnodes_by_position[row_num + 1][col_num]->val);
                    if (_gridnodes_by_position[row_num + 1][col_num]->val == num)
                        _gridnodes_by_value.at(num - 1)->movable = true;

                    //Step 7-8
                    _move_recursive(num, _gridnodes_by_position[row_num][col_num]->val);
                    if (_gridnodes_by_position[row_num][col_num]->val == num && _gridnodes_by_position[row_num][col_num - 1]->val == num - 1){
                        _gridnodes_by_value.at(num)->movable = false;
                        _gridnodes_by_value.at(num - 1)->movable = false;
                    }
                }
            }
        }
    }
    void _solve_column(int col_num){
        for (int row_num=0, num; row_num < _board.size(); row_num++) {
            num = row_num * (int)_board.size() + col_num + 1;
            // Steps for solving up to (but not including) last 2 elms of in a column:
            if (row_num < _board.size() - 2){
                _move_recursive(num, _gridnodes_by_position[row_num][col_num]->val);
                if (_gridnodes_by_position[row_num][col_num]->val == num)
                    _gridnodes_by_value.at(num)->movable = false;
            }
                // STEPS FOR SOLVING LAST 2 ELMS OF A COLUMN:
            else{
                /* Step 0: Check if column is solved. If it is, we're done. If not...
                 *
                 * Step 1: If last two elements of column are in each others' spot then move each out
                 *          of the Bottom-Left quadrant.
                 *      NOTE: It's okay if *one* of the elms remain in the quadrant
                 *           after both "ejects" have taken place. This step is meant to provide "room" to move
                 *           each of these elements independently of each other in the following steps.
                 *
                 * Step 2: If the number belonging in the 2nd-to-last spot of the column
                 *          is in the second-to-last position BEFORE it's been put in the last spot in the column,
                 *          then move that number outside of the Bottom-Left quadrant.
                 *          (Necessary in order to prepare for positioning the number belonging in LAST spot of column)
                 *
                 * Step 3: If the num belonging in the last spot of the column is already there,
                 *          check if it's possible to immediately move the number preceding it (in the column) to the 2nd-to-last
                 *          spot in the column. This will solve the column and exit the function.
                 *             (Take this action whenever possible!).
                 *
                 *          If this is not possible, then eject num from the Bottom-Left quadrant.
                 *          (Again, necessary to set up number belonging in LAST spot of column)
                 *
                 * Step 4:  Put 2nd-to-last elm in the last spot in the column.
                 *
                 * Step 5:  Check if the board is deadlocked due to the empty space becoming trapped between
                 *          the last solved column, the 3rd-to-last in the column, the 2nd-to-last in the column,
                 *          & the last-in-column,   (L,U,R,D respectively).
                 *
                 *          To break this deadlock, we mark 2nd-to-last in column as movable & eject last-in-column
                 *          from Bottom-Left quadrant. (Done efficiently by moving last-in-column one right from the center).
                 *          Then we can finally move 2nd-to-last back to the last spot of the column.
                 *
                 * Step 6:  Put 2nd-to-last elm in the last spot in the column &
                 *            Put last elm to the right of 2nd-to-last elm
                 *
                 * Step 7-8: Rotate 2nd-to-last & last elms into their correct spots in the column.
                 *
                 *  >> Column Solve Complete <<
                */

                // STEPS 0-5
                if (row_num == _board.size() - 2){
                    // STEP 0:
                    if (num == _gridnodes_by_position[row_num][col_num]->val && num + static_cast<int>(_board.size()) == _gridnodes_by_position[row_num + 1][col_num]->val){
                        _gridnodes_by_position[row_num][col_num]->movable = false;
                        _gridnodes_by_position[row_num + 1][col_num]->movable = false;
                        return;
                    }
                    // STEPS 1-3
                    else {
                        //STEP 1:
                        if (_gridnodes_by_value.at(num)->val == _gridnodes_by_position[row_num + 1][col_num]->val
                            && _gridnodes_by_value.at(num + static_cast<int>(_board.size()))->val == _gridnodes_by_position[row_num][col_num]->val){
                            _move_recursive(num+static_cast<int>(_board.size()), _gridnodes_by_value.at(num + static_cast<int>(_board.size()))->right->right->val, num);
                            _move_recursive(num, _gridnodes_by_value.at(num)->right->right->val, num);
                            if (num == _gridnodes_by_position[row_num][col_num]->val && num + static_cast<int>(_board.size()) == _gridnodes_by_position[row_num + 1][col_num]->val){
                                _gridnodes_by_position[row_num][col_num]->movable = false;
                                _gridnodes_by_position[row_num + 1][col_num]->movable = false;
                                return;
                            }
                        }

                        bool second_to_last_in_col_is_correct = _gridnodes_by_position[row_num][col_num]->val == num;
                        bool last_in_col_is_correct = num + static_cast<int>(_board.size()) == _gridnodes_by_position[row_num + 1][col_num]->val;

                        //STEP 2:
                        if (second_to_last_in_col_is_correct){
                            //Eject column's 2nd to last element
                            _move_recursive(num, _gridnodes_by_value.at(num)->right->right->val);
                        }

                        //STEP 3:
                        if (last_in_col_is_correct) {
                            if (_gridnodes_by_value.at(num + static_cast<int>(_board.size()))->up->val == 0
                                && _gridnodes_by_value.at(0)->right->val == num) {
                                _swap_zero_with(num);
                                _gridnodes_by_position[row_num][col_num]->movable = false;
                                _gridnodes_by_position[row_num + 1][col_num]->movable = false;
                                return;
                            }
                                //Eject column's last element
                            else _move_recursive(num + static_cast<int>(_board.size()), _gridnodes_by_value.at(num + static_cast<int>(_board.size()))->right->right->val);
                        }
                    }

                    //STEP 4
                    _move_recursive(num, _gridnodes_by_position[row_num + 1][col_num]->val, num + static_cast<int>(_board.size()), DOWN);

                    //STEP 5
                    if (_gridnodes_by_value.at(num)->val == _gridnodes_by_position[row_num + 1][col_num]->val && _gridnodes_by_position[row_num][col_num + 1]->val == num + static_cast<int>(_board.size())) {
                        _gridnodes_by_value.at(num)->movable = true;
                        _move_recursive(num+static_cast<int>(_board.size()), _gridnodes_by_value.at(num + static_cast<int>(_board.size()))->right->val);
                        _move_recursive(num, _gridnodes_by_position[row_num + 1][col_num]->val);
                    }
                    if (_gridnodes_by_position[row_num + 1][col_num]->val == num) {
                        _gridnodes_by_value.at(num)->movable = false;
                    }
                }
                // STEP 6-8
                else{
                    //Step 6
                    _move_recursive(num, _gridnodes_by_position[row_num][col_num + 1]->val);
                    if (_gridnodes_by_position[row_num][col_num + 1]->val == num)
                        _gridnodes_by_value.at(num - static_cast<int>(_board.size()))->movable = true;

                    //Step 7-8
                    _move_recursive(num, _gridnodes_by_position[row_num][col_num]->val);
                    if (_gridnodes_by_position[row_num][col_num]->val == num && _gridnodes_by_position[row_num - 1][col_num]->val == num - static_cast<int>(_board.size())){
                        _gridnodes_by_value.at(num)->movable = false;
                        _gridnodes_by_value.at(num - static_cast<int>(_board.size()))->movable = false;
                    }
                }
            }
        }
    }
    void _solve_remaining(){
        auto end_state = gridToState(_gridnodes_by_position);
        const auto moves = EndPuzzleSolverSingleton::get().solve(end_state);
        if (!moves.empty() && moves.at(0)){
            std::for_each(moves.begin(), moves.end(), [this](int move){_swap_zero_with(move);});
            for (const auto& row : end_state){
                for (const auto& num : row)
                    _gridnodes_by_value.at(num)->movable=false;
            }
        }
        else _movelist = moves;
    }
    
    void _swap(const int x){ //DO NOT USE REFERENCE HERE, x will be overwritten if a {GridNode}.val is passed in!
        if (!x || !_gridnodes_by_value.at(x)->movable)
            return;

        //swap nodes' value fields
        _gridnodes_by_value.at(0)->val = x;
        _gridnodes_by_value.at(x)->val = 0;
        
        //temporarily extract nodes & swap their placements in map
        auto temp_x = _gridnodes_by_value.extract(x), temp_0 = _gridnodes_by_value.extract(0);
        temp_0.key() = x;
        temp_x.key() = 0;

        //put 'em parent w/o copying them
        _gridnodes_by_value.insert(std::move(temp_0));
        _gridnodes_by_value.insert(std::move(temp_x));

    }
    std::shared_ptr<GridNode> _swap_zero_with(const int x) {
        if (!x || !_gridnodes_by_value.at(x)->movable)
            return _gridnodes_by_value.at(0);

        const auto& chk = _gridnodes_by_value.at(0);
        if ((chk->up && chk->up->val==x) || (chk->right && chk->right->val==x)
            || (chk->down && chk->down->val==x) || (chk->left && chk->left->val==x))
            _swap(x);
        if (chk->val) {
            if (!_movelist.empty() && _movelist.back() == x)
                _movelist.pop_back();
            else
                _movelist.emplace_back(x);
            if (printAll)
                _print_board();
        }
        return _gridnodes_by_value.at(0);
    }

private:    // Private Recursive Functions
    template <typename... Ints>
    std::shared_ptr<GridNode> _find_path_recursive(vector<enum direction>& path, std::shared_ptr<GridNode> const& start,
            std::shared_ptr<GridNode>const& end, const enum direction came_from, Ints... Ignore) {
        if (start == end)
            return start;
        else if (!start || !end || start->visited || !start->movable || !end->movable ||
                (_eject_me > 0 && start->val == _eject_me) || (came_from != NONE && ((start->val == Ignore)||...))) {
            if (!path.empty())
                path.pop_back();
            if (start&& start->visited)
                _backtrack_to = came_from;
            return nullptr;
        }
        start->visited = true;
        vector<enum direction> explore_order = _get_exploration_order(start, end);
        std::shared_ptr<GridNode>chk, explorer;
        enum direction coming_from = came_from;
        for (auto dir : explore_order){
            if (dir == came_from) {
                coming_from = NONE;
                continue;
            }
            switch (dir) {
                case NONE:
                    explorer = nullptr;
                    coming_from = NONE;
                    break;
                case UP:
                    explorer = start->up;
                    coming_from = DOWN;
                    break;
                case DOWN:
                    explorer = start->down;
                    coming_from = UP;
                    break;
                case LEFT:
                    explorer = start->left;
                    coming_from = RIGHT;
                    break;
                case RIGHT:
                    explorer = start->right;
                    coming_from = LEFT;
                    break;
            }
            path.emplace_back(dir);
            auto discovered_end = _find_path_recursive(path, explorer, end, coming_from, Ignore...);
            if (discovered_end == end){
                start->visited = false;
                return discovered_end;
            }else if (_backtrack_to != NONE){
                if (_backtrack_to == dir){
                    _backtrack_to = NONE;
                    continue;
                }
                break;
            }
        }
        start->visited = false;
        path.pop_back();
        return nullptr;
    }
    std::shared_ptr<GridNode> _move_recursive(int move_me, int to_me, int reject_me= -1, enum direction orientation= NONE){
        if (move_me == to_me){
            return _gridnodes_by_value.at(0);
        }

        //Step One: Determine where to move the empty_space to
        std::shared_ptr<GridNode> empty_space = _gridnodes_by_value.at(0);
        std::shared_ptr<GridNode> start_space = _gridnodes_by_value.at(move_me);
        std::shared_ptr<GridNode> end_space = _gridnodes_by_value.at(to_me);
        std::shared_ptr<GridNode> step = start_space;

        int end_space_row = end_space->row, end_space_col = end_space->col;

        std::vector<enum direction> path_from_start_to_end;
        std::vector<enum direction> path_from_empty_to_step;
        enum direction coming_from;

        auto discovered_end = _find_path_recursive(path_from_start_to_end,start_space,end_space,NONE,0,move_me);
        if (discovered_end == end_space){
            for (auto dir : path_from_start_to_end) {
                switch (dir) {
                    case NONE:
                        step = nullptr;
                        coming_from = NONE;
                        break;
                    case UP:
                        step = step->up;
                        coming_from = DOWN;
                        break;
                    case DOWN:
                        step = step->down;
                        coming_from = UP;
                        break;
                    case LEFT:
                        step = step->left;
                        coming_from = RIGHT;
                        break;
                    case RIGHT:
                        step = step->right;
                        coming_from = LEFT;
                        break;
                }
                auto discovered_step = _find_path_recursive(path_from_empty_to_step, empty_space,
                                                            step, NONE, 0, move_me);
                if (discovered_step == step) {
                    std::shared_ptr<GridNode> swap_me = empty_space;
                    for (auto move_dir : path_from_empty_to_step){
                        switch (move_dir) {
                            case NONE:
                                swap_me = nullptr;
                                coming_from = NONE;
                                break;
                            case UP:
                                swap_me = swap_me->up;
                                coming_from = DOWN;
                                break;
                            case DOWN:
                                swap_me = swap_me->down;
                                coming_from = UP;
                                break;
                            case LEFT:
                                swap_me = swap_me->left;
                                coming_from = RIGHT;
                                break;
                            case RIGHT:
                                swap_me = swap_me->right;
                                coming_from = LEFT;
                                break;
                        }
                        empty_space = _swap_zero_with(swap_me->val);
                        if (reject_me > 0){
                            switch (orientation) {
                                case NONE:
                                    break;
                                case UP:
                                case DOWN:
                                    if (reject_me == _gridnodes_by_position[_board.size() - 1][end_space_col]->val ||
                                        reject_me == _gridnodes_by_position[_board.size() - 2][end_space_col]->val){
                                        _move_recursive(reject_me, _gridnodes_by_value.at(reject_me)->right->right->val);
                                        return _move_recursive(move_me, _gridnodes_by_position[end_space_row][end_space_col]->val,
                                                               reject_me, orientation);
                                    }
                                    break;
                                case LEFT:
                                case RIGHT:
                                    if (reject_me == _gridnodes_by_position[end_space_row][_board.size() - 1]->val ||
                                        reject_me == _gridnodes_by_position[end_space_row][_board.size() - 2]->val){
                                        _move_recursive(reject_me, _gridnodes_by_value.at(reject_me)->down->down->val);
                                        return _move_recursive(move_me, _gridnodes_by_position[end_space_row][end_space_col]->val,
                                                               reject_me, orientation);
                                    }
                                    break;
                            }
                        }
                    }
                    empty_space = _swap_zero_with(move_me); // move move_me to next step
                }
                else {
                    if (printAll) {
                        cout << "Couldn't find valid path to place " << move_me << " in correct spot...\n";
                        _print_board();
                        cout << "\nChecking for deadlock...\n";
                    }
                    if (!(empty_space->up && empty_space->up->movable && empty_space->up->val != move_me) &&
                        !(empty_space->down && empty_space->down->movable && empty_space->down->val != move_me) &&
                        !(empty_space->left && empty_space->left->movable && empty_space->left->val != move_me) &&
                        !(empty_space->right && empty_space->right->movable && empty_space->right->val != move_me)
                    ) {
                        if (printAll)
                            cout << "Deadlock detected.\n\nMarking adjacent node with the highest number != " << move_me << " as movable...\n";
                        int mark_me_movable = std::max({
                                empty_space->up?empty_space->up->val:-1,
                                empty_space->down?empty_space->down->val:-1,
                                empty_space->left?empty_space->left->val:-1,
                                empty_space->right?empty_space->right->val:-1,
                            }, [&move_me](const int& a, const int& b) -> bool{ return a == move_me || a < b;}
                        );
                        if (mark_me_movable > 0) {
                            int mmm_orig_row = _gridnodes_by_value.at(mark_me_movable)->row;
                            int mmm_orig_col = _gridnodes_by_value.at(mark_me_movable)->col;
                            if (printAll)
                                cout << "Marked " << mark_me_movable << " as movable.\n";
                            _gridnodes_by_value.at(mark_me_movable)->movable = true;
                            int intermediate_step;
                            if (empty_space->right->val == mark_me_movable)
                                intermediate_step = _gridnodes_by_value.at(move_me)->down->val;
                            else
                                intermediate_step = _gridnodes_by_value.at(move_me)->right->val;
                            if (printAll)
                                cout << "Moving " << move_me << " to intermediate step: " << intermediate_step
                                     << " and putting " << mark_me_movable << " back...\n";
                            _move_recursive(move_me, intermediate_step);
                            _move_recursive(mark_me_movable, _gridnodes_by_position[mmm_orig_row][mmm_orig_col]->val);
                            if (printAll)
                                cout << "Re-running _move_recursive("<<move_me<<", "<<end_space->val<<", "<<reject_me<<", "<< orientation<<")...\n";
                            return _move_recursive(move_me, end_space->val, reject_me, orientation);
                        } else if (printAll) cout << "No adjacent node matching criteria was found...\n";
                    }else if (printAll) cout << "Could not detect gridlock condition.\n";

                    cout << "\nOrig:\n";
                    for (const auto& row : _board){
                        for (auto num : row)
                            cout << num << "\t";
                        cout << "\n";
                    }
                    std::cin.get();
                }
                path_from_empty_to_step.clear();
            }
        }
        return empty_space;
    }

private:    // Private Static Functions
    static bool is_solved(Grid& grid){
        for (int i=0; i < grid.size(); i++)
            for (int j=0; j < grid.at(i).size(); j++)
                if (i==grid.size() - 1 && j==grid.at(i).size()-1)
                    return grid.at(i).at(j)->val == 0;
                else if (grid.at(i).at(j)->val != i*grid.at(i).size() + j + 1)
                    return false;
        return true;
    }
    static vector<enum direction> _get_exploration_order(std::shared_ptr<GridNode>const& start, const std::shared_ptr<GridNode>&end) {
        std::vector<enum direction> explore_order;
        explore_order.reserve(4);
        enum direction move_vert = NONE;
        enum direction move_horz = NONE;
        if (start->row != end->row) {
            move_vert = start->row < end->row ? DOWN : UP;
            explore_order.push_back(move_vert);
        }
        if (start->col != end->col) {
            move_horz = start->col < end->col ? RIGHT : LEFT;
            explore_order.push_back(move_horz);
        }
        if (move_vert != NONE && move_horz!= NONE){
            explore_order.push_back(move_vert == UP? DOWN : UP);
            explore_order.push_back(move_horz == LEFT? RIGHT : LEFT);
        }else {
            if (move_horz == NONE){
                explore_order.push_back(LEFT);
                explore_order.push_back(RIGHT);
                explore_order.push_back(move_vert == UP? DOWN : UP);
            } else{
                explore_order.push_back(UP);
                explore_order.push_back(DOWN);
                explore_order.push_back(move_horz == LEFT? RIGHT : LEFT);
            }
        }
        return explore_order;
    }
    static void _print_path(const vector<enum direction>& path) {
        cout << "[";
        int i = 0;
        for (auto dir : path) {
            switch (dir) {
                case NONE:
                    cout << "?";
                    break;
                case UP:
                    cout << "UP";
                    break;
                case DOWN:
                    cout << "DOWN";
                    break;
                case LEFT:
                    cout << "LEFT";
                    break;
                case RIGHT:
                    cout << "RIGHT";
                    break;
            }
            if ((++i) < path.size())
                cout << ", ";
        }
        cout << "]\n";
    }

public:     // Public Static Functions
    static State create_random_board(int size=0, int min=3, int max=10){
        static std::random_device rd;
        static std::default_random_engine re(rd());
        static std::uniform_int_distribution<int> sizeGen(min,max);
        if (min > max){
            int temp = max;
            max = min;
            min = temp;
        }

        if (size < min || size > max)
            size = sizeGen (re);
        vector<int> possibilities(size*size);
        std::iota(possibilities.begin(), possibilities.end(), 0);
        std::shuffle(possibilities.begin(), possibilities.end(), re);

        State board;
        board.reserve(size);
        for (auto range_start = possibilities.begin(), range_end = possibilities.begin()+size; range_start != possibilities.end(); range_start+=size, range_end+=size ) {
            board.emplace_back(range_start, range_end);
        }
        return board;
    }
    static State gridToState(const Grid& grid){
        State state(grid.size(), vector<int>(grid.at(0).size()));
        for (int i=0; i < grid.size(); i++)
            for (int j=0; j < grid.at(i).size(); j++)
                state[i][j] = grid[i][j]->val;
        return state;
    }
    static bool verifySolution(const vector<int>& movelist, SlidingPuzzleSolver& sps, bool show_steps= false){
        if (show_steps)
            sps._print_board();
        for (const auto& move: movelist){
            sps._swap_zero_with(move);
            if (show_steps)
                sps._print_board();
        }
        auto res = is_solved(sps._gridnodes_by_position);
        if (show_steps) {
            if (res)
                cout << "Puzzle verified solved!\n";
            else cout << "ERR Puzzle NOT in Solved State!\n";
        }
        return res;
    }

public:     //Constructors, Destructors, & Operators
    explicit SlidingPuzzleSolver (const State& arr, bool show_details=false)
      : _board(arr), _map_size(arr.size() * arr.size()), printAll(show_details)
    {
      _create_map();
      _p_entry_point = _gridnodes_by_value.at(arr[0][0]);
      _eject_me = -1;
    }
    friend std::ostream& operator<<(std::ostream& os, SlidingPuzzleSolver& sps){
        sps._print_board(os);
        return os;
    }
    
public:     //Public Functions
    std::vector<int> solve(){
        // Methodical Solving
        for (int num = 0; num < _board.size() - 2; ++num) {
            _solve_row(num);
            if (num < _board.size() - 3){
                _solve_column(num);
            }
        }
        _solve_remaining();
        return is_solved(_gridnodes_by_position) ? _movelist : vector<int>{0};
    }
};

std::vector<int> slide_puzzle(const vector<vector<int>> &arr){
    SlidingPuzzleSolver sps = SlidingPuzzleSolver(arr);
    auto res = sps.solve();
    return res;
}

#if 0
void *operator new (std::size_t size){
    auto* ptr = malloc(size);
    cout << "::new : Allocated " << size << " bytes at " << ptr << ".\n";
    return ptr;
}

void operator delete (void* ptr) noexcept {
    cout << "::delete : De-allocated item at " << ptr << ".\n";
    free(ptr);
}
#endif

int main() {
    // Kata Author provided test cases
#if 1
    std::vector<std::vector<std::vector<int>>> puzzles = {{
      	{4,1,3},
      	{2,8,0},
      	{7,6,5}},
       {{10, 3, 6, 4},
      	{ 1, 5, 8, 0},
      	{ 2,13, 7,15},
      	{14, 9,12,11}},
       {{ 3, 7,14,15,10},
      	{ 1, 0, 5, 9, 4},
      	{16, 2,11,12, 8},
      	{17, 6,13,18,20},
      	{21,22,23,19,24}}};

for (const auto& puzzle : puzzles ) {
    auto sln = slide_puzzle(puzzle);
    auto p = SlidingPuzzleSolver(puzzle);
    int i = 0;
    for (const auto &move : sln) {
        if (i == 0)
            cout << "{";
        cout << move;
        if (i++ < sln.size() - 1)
            cout << ", ";
        else cout << "}";
    }
    cout << "\n(" << sln.size() << " moves to solve state.)\n\n";
    if (SlidingPuzzleSolver::verifySolution(sln, p))
        cout << "Solution verified!\n";
    std::cin.get();
}
#endif

    // Problem Boards
#if 0
    vector<vector<int>> board2 = {
            {14,      4,       11,      1},
            {12,      9,       8,       7},
            {10,      13,      15,      6},
            {5,       3,       2,       0}
    };

    vector<vector<int>> problem_board1 = {
            {10,      1,       15,      8},
            {12,      3,       4,       7},
            {13,      5,       0,       9},
            {14,      2,       11,      6}
    };

    // FIXED: This board causes infinite loop in apply_detour method
    //  - something with the path trial order
    vector<vector<int>> problem_board2 = {
            {9,       14,      11,      8},
            {4,       5,       10,      0},
            {15,      3,       2,       12},
            {7,       1,       13,      6}
    };

    //FIXED: Program fails to solve last two spots in column 4.
    //  - Algorithm breaks down while trying to find a valid
    //  - path to get 53 / 61 in place...
    //  - empty_space gets trapped between 52, 43, 61, & 53 (L,U,R,D respectively).
    vector<vector<int>> problem_board3 = {
            {63,   36,   9,   41,   12,   46,   11,   39},
            {8,   14,   4,   54,   51,   37,   21,   1},
            {6,   22,   15,   55,   45,   2,   32,   28},
            {43,   35,   50,   38,   57,   24,   52,   61},
            {47,   34,   60,   0,   40,   58,   10,   31},
            {3,   7,   59,   18,   62,   16,   30,   17},
            {13,   5,   27,   29,   23,   19,   33,   26},
            {42,   20,   53,   49,   56,   48,   25,   44}
    };


    vector<vector<int>> problem_board4 = {
            {1,4,0},
            {2,5,3},
            {8,7,6}
    };

    vector<vector<int>> problem_board5 = {
            {1,7,0},
            {4,2,3},
            {8,6,5}
    };

    vector<vector<int>> problem_board6 = {
            {1,      2,      3,      4,      5,      6},
            {7,      8,      9,      10,     11,     12},
            {13,     14,     15,     16,     17,     18},
            {19,     20,     21,     22,     23,     24},
            {25,     26,     27,     34,     29,     0},
            {31,     32,     33,     35,     30,     28}
    };

    vector<vector<int>> problem_board7 = {
            {1,      2,      3,      4,      5,      6,      7,      8},
            {9,      10,     11,     12,     13,     14,     15,     16},
            {17,     18,     19,     20,     21,     22,     23,     24},
            {25,     26,     27,     28,     29,     30,     31,     32},
            {33,     34,     35,     36,     37,     38,     39,     40},
            {41,     42,     43,     44,     45,     46,     47,     48},
            {49,     50,     51,     52,     53,     63,     62,     0},
            {57,     58,     59,     60,     61,     54,     55,     56}
    };

    // FIXME: Couldn't find valid path to place 80 in correct spot...
    vector<vector<int>> problem_board8 = {
            {51,     95,     89,     91,     20,     84,     4,      45,     80,     16},
            {0,      68,     72,     76,     39,     12,     40,     17,     64,     66},
            {77,     25,     88,     67,     9,      24,     3,      21,     46,     27},
            {18,     50,     48,     30,     54,     86,     73,     41,     75,     34},
            {94,     37,     15,     26,     10,     38,     2,      5,      43,     14},
            {74,     85,     97,     42,     93,     31,     63,     65,     8,      32},
            {90,     56,     23,     11,     96,     52,     36,     69,     57,     1},
            {33,     81,     49,     59,     60,     58,     79,     61,     7,      87},
            {44,     99,     28,     47,     53,     70,     6,      29,     78,     55},
            {22,     62,     35,     19,     71,     13,     92,     82,     98,     83}
    };

    // FIXME: Couldn't find valid path to place 3 in correct spot...
    vector<vector<int>> problem_board9 = {
            {1,0,2},
            {8,3,7},
            {4,6,5}
    };


    vector<vector<int>> problem_board10 = {
            {1,0,2},
            {5,3,6},
            {7,8,4}
    };


    vector<vector<int>> problem_board11 = {
            {1,3,2},
            {6,4,0},
            {5,7,8}
    };



    vector<vector<int>> problem_board12{
        {1,      0,      4},
        {7,      5,      3},
        {8,      2,      6}
    };


    vector<vector<int>> problem_board13{
        {1,	5,	0},
        {8,	2,	3},
        {4,	7,	6}
    };


    vector<vector<int>> problem_board14{
        {1, 3, 0},
        {6, 5, 2},
        {8, 7, 4}
    };

    //auto sln = slide_puzzle(problem_board7);
    //cout << sln;
    //int j=0;
    /*cout << "{";
    for (auto num : sln) {
        cout << num;
        if (j++ < sln.size()-1)
            cout << ", ";
    }
    cout << "}\n";*/

    //slide_puzzle(problem_board1);   //FAIL
    //slide_puzzle(problem_board2);   //FAIL
    const auto& board = problem_board14;
    auto start = std::chrono::steady_clock::now();
    auto sln = slide_puzzle(board);   //SUCCESS
    auto end = std::chrono::steady_clock::now();
    double seconds_elapsed = std::chrono::duration<double, std::milli>(end-start).count();
    cout << "Evaluated puzzle of size " << board.size() << "-by-" << board.size() << "  in: " << seconds_elapsed/1000.f << "s.\n";
    //cout << "Avg. time per "<< size << "-by-" << size <<" puzzle: " << seconds_elapsed/n << "ms.\n";
/*    int i = 0;
    cout << "{";
    for (const auto& move : sln) {
        cout << move;
        if (i++ < sln.size() -1)
            cout << ", ";
    }
    cout << "}\n";*/
    cout<< "(" << (sln.size() > 1 && sln.at(0) ? sln.size() : 0) << " moves to solve state.)\n\n";
    //slide_puzzle(problem_board4);   //SUCCESS
    //slide_puzzle(problem_board5);   //SUCCESS
    //slide_puzzle(problem_board6);   //FAIL
    //slide_puzzle(problem_board7);   //FAIL
#endif

    // Random Tests
#if 0
    int size=10, n=1000;
    auto start = std::chrono::steady_clock::now();
    vector<int> sln;
    vector<vector<int>> rand_board;
    for (int i=0, j; i < n; i++) {
        rand_board = std::move(SlidingPuzzleSolver::create_random_board(size));
        sln = slide_puzzle(rand_board);
/*        j=0;
        for (const auto& move : sln) {
            if (j==0)
                cout << "{";
            cout << move;
            if (j++ < sln.size()-1)
                cout << ", ";
            else cout << "}";
        }*/
        //cout<< "\n(" << (sln.size() > 1 ? sln.size() : 0) << " moves to solve state.)\n\n";
    }
    auto end = std::chrono::steady_clock::now();
    auto seconds_elapsed = std::chrono::duration<double>(end-start); //in floating-point seconds
    if (size)
        cout << "Evaluated " << n << " puzzles of size " << size << "-by-" << size;

    else
        cout << "Evaluated " << n << " random puzzles with sizes between [3,11) \n";

    cout << "  in: " << seconds_elapsed.count() << "s.\n";
    cout << "Avg. time per puzzle: " << std::chrono::duration<double, std::milli>(seconds_elapsed).count() / n << "ms.\n";
#endif



    std::cin.get();
    return 0;
}