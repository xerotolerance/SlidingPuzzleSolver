#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <iterator>
#include <tuple>
#include <functional>
#include <memory>
#include <chrono>

using std::unordered_map;
using std::vector;
using std::cout;
using std::endl;

class SlidingPuzzleSolver{
private:    // Private Classes, Structs, & Enums
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
            GridNode (const GridNode& orig): val(orig.val), row(orig.row), col(orig.col), up(orig.up), right(orig.right), down(orig.down), left(orig.left) {
                //cout << orig.val << " was copied!" << endl;
            };
            GridNode(GridNode&& src) noexcept
                : val(std::exchange(src.val, -1)), row(std::exchange(src.row, -1)), col(std::exchange(src.col, -1)),
                  up(std::exchange(src.up, nullptr)), right(std::exchange(src.right, nullptr)),
                  down(std::exchange(src.down, nullptr)), left(std::exchange(src.left, nullptr)){
                //cout << this->val << " was moved!" << endl;
            }
    };
    class EndMovesTree{
    private: // Member Classes
        class EndMovesTreeNode{
        private: //Private data members
             std::shared_ptr<EndMovesTreeNode> up = nullptr;
             std::shared_ptr<EndMovesTreeNode> left = nullptr;
             std::shared_ptr<EndMovesTreeNode> down = nullptr;
             std::shared_ptr<EndMovesTreeNode> right = nullptr;
            std::shared_ptr<EndMovesTreeNode> parent;
            vector<vector<int>>state;
            int depth, empty_row, empty_col;

        public: // Public Static Functions
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
                    : state(std::move(state)), parent(std::move(parent)), empty_row(empty_row), empty_col(empty_col), depth(depth){}
            ~EndMovesTreeNode() = default;
        };

    private: // Private data members
         int node_count;
         std::shared_ptr<EndMovesTreeNode> end_node;
        unordered_map<std::string, std::shared_ptr<EndMovesTreeNode>> substates;
        vector<vector<int>> start_state, win_state;

    private: // Private Functions
        std::shared_ptr<EndMovesTreeNode> buildStateTree(const vector<vector<int>>& state,
                                                         int empty_row_num = -1, int empty_col_num = -1,
                                                         const std::shared_ptr<EndMovesTreeNode>& parent = nullptr, int depth= 0) {
            empty_row_num = empty_row_num < 0 ? (int)state.size()-1 : empty_row_num;
            empty_col_num = empty_col_num < 0 ? (int)state.at(0).size()-1 : empty_col_num;
            std::string key = EndMovesTreeNode::stateToKey(state);
            if (substates.count(key)){
                std::weak_ptr<EndMovesTreeNode> tn = substates.at(key);
                auto ref_count = tn.use_count();
                auto temp = tn.lock();
                if (depth < temp->depth){
                    temp->depth = depth;
                    temp->parent = parent;
                }
                return substates.at(key);
            }
            else substates[key] = std::make_shared<EndMovesTreeNode>(std::move(EndMovesTreeNode{state, parent, empty_row_num, empty_col_num, depth}));

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

    public: // Public Static Functions
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

    public: // Public functions
        const vector<vector<int>>& getStartState() const{
            return static_cast<const vector<vector<int>> &>(start_state);
        }
        const vector<vector<int>>& getWinState() const{
            return static_cast<const vector<vector<int>> &>(win_state);
        }
        vector<int> generate_moveslist() const{
            vector<int> moveslist;
            int num_moves = 0;
            std::string key = EndMovesTreeNode::stateToKey(getStartState());
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
        explicit EndMovesTree(vector<vector<int>> start_state): start_state(std::move(start_state)){
            win_state = std::move(generate_win_state(this->start_state));
            node_count = 0;
            end_node = buildStateTree(win_state);
        }
        ~EndMovesTree(){
            substates.clear();
        }
    };
    enum direction{NONE=-1, UP, DOWN, LEFT, RIGHT};

private:    // Private data members
    const std::vector<std::vector<int>>& _arr;
    std::vector<std::vector<std::shared_ptr<GridNode>>> _grid;
    std::unordered_map<int, std::shared_ptr<GridNode>> _quick_lookup;
    std::vector<int> _movelist;
    std::shared_ptr<GridNode> _p_entry_point;
    const unsigned int _map_size;
    enum direction _backtrack_to = NONE;
    int _eject_me;
    bool printAll = false;

private:    // Private Functions
    void _print_board(std::ostream& os=cout, const std::string& end_dec = "==========================") const{
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
        _grid.reserve(_arr.size());
        _quick_lookup.reserve(_map_size);
        for (const auto& row_it : _arr){
            col = 0;
            _grid.emplace_back(std::vector<std::shared_ptr<GridNode>>());
            _grid.back().reserve(_arr.size());
            for (const int& item : row_it){
                _quick_lookup.emplace(item, std::make_shared<GridNode>(std::move(GridNode(item, row, col, node_above, nullptr, nullptr, node_before))));
                _grid[row].emplace_back(_quick_lookup.at(item));
                if (node_before) {
                    node_before->right = _quick_lookup.at(item);
                }
                if (node_above) {
                    node_above->down = _quick_lookup.at(item);
                    node_above = node_above->right;
                }
                node_before = _quick_lookup.at(item);
                ++col;
            }
            node_above = _quick_lookup.at(*(row_it.begin()));
            node_before = nullptr;
            ++row;
        }
    }
    void _solve_row(int row_num){
        for (int col_num=0, num; col_num < _arr.size(); col_num++) {
            num = row_num * (int)_arr.size() + col_num + 1;
            if (col_num < _arr.size()-2){
                _move_recursive(num, _grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num)
                    _quick_lookup.at(num)->movable = false;
            }
            else if(col_num == _arr.size() - 2){
                if (_grid[row_num][col_num]->val == num || _grid[row_num][col_num+1]->val == num)
                    _move_recursive(num, _quick_lookup.at(num)->down->down->val);
                if (num + 1==_grid[row_num][col_num]->val || num + 1==_grid[row_num][col_num+1]->val)
                    _move_recursive(num + 1, _quick_lookup.at(num + 1)->down->down->val);

                if (_quick_lookup.at(num)->val == _grid[row_num+1][col_num]->val && _grid[row_num+1][col_num+1]->val == num + 1)
                    _move_recursive(num, _grid[row_num + 1][col_num - 1]->val);

                _move_recursive(num, _grid[row_num][col_num + 1]->val, num + 1, RIGHT);
                if (_grid[row_num][col_num+1]->val == num) {
                    _quick_lookup.at(num)->movable = false;
                    if (num + 1==_grid[row_num][col_num]->val)
                        _move_recursive(num + 1, _quick_lookup.at(num + 1)->down->down->val);
                }
            }
            else{
                _move_recursive(num, _grid[row_num + 1][col_num]->val);
                if (_grid[row_num+1][col_num]->val == num)
                    _quick_lookup.at(num-1)->movable = true;
                _move_recursive(num, _grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num && _grid[row_num][col_num-1]->val==num-1){
                    _quick_lookup.at(num)->movable = false;
                    _quick_lookup.at(num-1)->movable = false;
                }
            }
        }
    }
    void _solve_column(int col_num){
        for (int first_in_row = 1, num, row_num; first_in_row < _map_size; first_in_row+=_arr.size()) {
            num = first_in_row + col_num;
            row_num = first_in_row / (int)_arr.size();
            if(first_in_row < _map_size - 2 * _arr.size()){
                _move_recursive(num, _grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num)
                    _quick_lookup.at(num)->movable = false;
            }
            else if (first_in_row < _map_size - 1 * _arr.size()){
                if (_grid[row_num][col_num]->val == num || _grid[row_num+1][col_num]->val == num)
                    _move_recursive(num, _quick_lookup.at(num)->right->right->val);
                if (num + _arr.size()==_grid[row_num][col_num]->val || num + _arr.size()==_grid[row_num+1][col_num]->val)
                    _move_recursive(num + (int) _arr.size(),
                                    _quick_lookup.at(num + (int) _arr.size())->right->right->val);
                _move_recursive(num, _grid[row_num + 1][col_num]->val, num + (int) _arr.size(), DOWN);
                if (_grid[row_num+1][col_num]->val == num) {
                    _quick_lookup.at(num)->movable = false;
                    if (num + _arr.size()==_grid[row_num][col_num]->val)
                        _move_recursive(num + (int) _arr.size(),
                                        _quick_lookup.at(num + (int) _arr.size())->right->right->val);
                }
            }
            else{
                _move_recursive(num, _grid[row_num][col_num + 1]->val);
                if (_grid[row_num][col_num+1]->val == num)
                    _quick_lookup.at(num-_arr.size())->movable = true;
                _move_recursive(num, _grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num && _grid[row_num-1][col_num]->val==num-_arr.size()){
                    _quick_lookup.at(num)->movable = false;
                    _quick_lookup.at(num-_arr.size())->movable = false;
                }
            }
        }
    }
    void _solve_remaining(){
        vector<vector<int>> remaining(2);
        auto top_start_it = _grid.at(_arr.size()-2).end()-3;
        auto top_end_it = _grid.at(_arr.size()-2).end();
        auto bottom_start_it = _grid.at(_arr.size()-1).end()-3;
        auto bottom_end_it = _grid.at(_arr.size()-1).end();
        std::for_each(top_start_it, top_end_it, [&remaining](const std::shared_ptr<GridNode>& node){
            remaining.at(0).emplace_back(node->val);
        });
        std::for_each(bottom_start_it, bottom_end_it, [&remaining](const std::shared_ptr<GridNode>& node){
            remaining.at(1).emplace_back(node->val);
        });

        EndMovesTree end_solver(remaining);
        auto moves = end_solver.generate_moveslist();
        if (!moves.empty() && moves.at(0)){
            _movelist.insert(_movelist.end(), moves.begin(), moves.end());
            std::for_each(moves.begin(), moves.end(), [this](int move){_swap_zero_with(move);});
            for (const auto& row : end_solver.getWinState()){
                for (const auto& num : row)
                    _quick_lookup.at(num)->movable=false;
            }
        }
        else _movelist = moves;
    }
    void _swap(const int x){ //DO NOT USE REFERENCE HERE, x will be overwritten if a {GridNode}.val is passed in!
        if (!x || !_quick_lookup.at(x)->movable)
            return;

        //swap nodes' value fields
        _quick_lookup.at(0)->val = x;
        _quick_lookup.at(x)->val = 0;
        
        //temporarily extract nodes & swap their placements in map
        auto temp_x = _quick_lookup.extract(x), temp_0 = _quick_lookup.extract(0);
        temp_0.key() = x;
        temp_x.key() = 0;

        //put 'em parent w/o copying them
        _quick_lookup.insert(std::move(temp_0));
        _quick_lookup.insert(std::move(temp_x));

    }
    std::shared_ptr<GridNode> _swap_zero_with(const int x) {
        if (!x || !_quick_lookup.at(x)->movable)
            return _quick_lookup.at(0);

        const auto& chk = _quick_lookup.at(0);
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
        return _quick_lookup.at(0);
    }

private: // Private Recursive Functions
    template <typename... Ints>
    std::shared_ptr<GridNode> _find_path_recursive(vector<enum direction>& path, std::shared_ptr<GridNode> const& start,
            const std::shared_ptr<GridNode> end, const enum direction came_from, Ints... Ignore) {
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
            //cout << "No need to move " << move_me << ": " << to_me << " is already in place.\nSkipping...\n";
            return _quick_lookup.at(0);
        }

        //Step One: Determine where to move the empty_space to
        std::shared_ptr<GridNode> empty_space = _quick_lookup.at(0);
        std::shared_ptr<GridNode> start_space = _quick_lookup.at(move_me);
        std::shared_ptr<GridNode> end_space = _quick_lookup.at(to_me);
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
                                    if (reject_me == _grid[_arr.size()-1][end_space_col]->val ||
                                        reject_me == _grid[_arr.size()-2][end_space_col]->val){
                                        _move_recursive(reject_me, _quick_lookup.at(reject_me)->right->right->val);
                                        return _move_recursive(move_me, _grid[end_space_row][end_space_col]->val,
                                                               reject_me, orientation);
                                    }
                                    break;
                                case LEFT:
                                case RIGHT:
                                    if (reject_me == _grid[end_space_row][_arr.size()-1]->val ||
                                        reject_me == _grid[end_space_row][_arr.size()-2]->val){
                                        _move_recursive(reject_me, _quick_lookup.at(reject_me)->down->down->val);
                                        return _move_recursive(move_me, _grid[end_space_row][end_space_col]->val,
                                                               reject_me, orientation);
                                    }
                                    break;
                            }
                        }
                    }
                    empty_space = _swap_zero_with(move_me); // move move_me to next step
                }else {
                    cout << "Couldn't find valid path to place "<< move_me <<" in correct spot...\n";
                    _print_board();
                    cout << "\nOrig:\n";
                    for (const auto& row : _arr){
                        for (auto num : row)
                            cout << num << "\t";
                        cout << "\n";
                    }
                    std::cin.get();
                }
                path_from_empty_to_step.clear();
            }
        }
        cout << '\0';
        return empty_space;
    }

private: // Private Static Functions
    static bool is_solved(vector<vector<std::shared_ptr<GridNode>>>& grid){
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

public: //Constructors, Destructors, & Operators
    explicit SlidingPuzzleSolver (const std::vector<std::vector<int>>& arr)
      : _arr(arr), _map_size(arr.size() * arr.size())
    {
      _create_map();
      _p_entry_point = _quick_lookup.at(arr[0][0]);
      _eject_me = -1;
    }
    friend std::ostream& operator<<(std::ostream& os, SlidingPuzzleSolver& sps){
        sps._print_board(os);
        return os;
    }

public: //Public Functions
    static vector<vector<int>> create_random_board(int size=0){
        std::srand(std::time(nullptr)); //"ok, psuedo-random," but its fine for this..."
        unsigned int length_of_square = size < 11 && size > 2? size : 3 + std::rand() % 8;
        unsigned int num_choices = length_of_square * length_of_square;
        vector<vector<int>> board;
        board.reserve(length_of_square);
        vector<int> validrange;
        validrange.reserve(num_choices);

        for (int i=0; i < length_of_square; i++) {
            board.emplace_back(vector<int>());
            board.at(i).reserve(length_of_square);
        }

        for (int i=0; i < num_choices; i++)
            validrange.emplace_back(i);

        int row = 0, rand_position, rand_int;
        while (!validrange.empty()){
            rand_position = std::rand() % validrange.size();
            rand_int = validrange[rand_position];

            if (board[row].size() >= length_of_square)
                ++row;
            board[row].emplace_back(rand_int);

            auto it = validrange.begin();
            std::advance(it, rand_position);
            validrange.erase(it);
        }
        return board;
    }
    std::vector<int> solve(){
        // Methodical Solving
        /*auto start = std::chrono::steady_clock::now();*/
        for (int num = 0; num < _arr.size()-2; ++num) {
            _solve_row(num);
            if (num < _arr.size()-3){
                _solve_column(num);
            }
        }
        _solve_remaining();
/*
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double, std::milli>(end-start);
        _print_board();
        cout << _arr.size() << "-by-" << _arr.size() << " puzzle solved in: " << elapsed.count() << "ms\n\n";
*/
        return is_solved(_grid)? _movelist : vector<int>{0};
    }
};

std::vector<int> slide_puzzle(const std::vector<std::vector<int>> &arr)
{
    SlidingPuzzleSolver sps = SlidingPuzzleSolver(arr);
    //cout << sps << endl;
    auto res = sps.solve();
    //cout << sps << endl;
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
#if 1
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
    auto start = std::chrono::steady_clock::now();
    auto sln = slide_puzzle(problem_board3);   //SUCCESS
    auto end = std::chrono::steady_clock::now();
    double seconds_elapsed = std::chrono::duration<double, std::milli>(end-start).count();
    cout << "Evaluated puzzle of size " << problem_board3.size() << "-by-" << problem_board3.size() << "  in: " << seconds_elapsed/1000.f << "s.\n";
    //cout << "Avg. time per "<< size << "-by-" << size <<" puzzle: " << seconds_elapsed/n << "ms.\n";
    int i = 0;
    cout << "{";
    for (const auto& move : sln) {
        cout << move;
        if (i++ < sln.size() -1)
            cout << ", ";
    }
    cout << "}\n(" << sln.size() << " moves to solve state.)\n\n";
    //slide_puzzle(problem_board4);   //SUCCESS
    //slide_puzzle(problem_board5);   //SUCCESS
    //slide_puzzle(problem_board6);   //FAIL
    //slide_puzzle(problem_board7);   //FAIL
#endif
#if 0
    int size=10, n=1000;
    auto start = std::chrono::steady_clock::now();
    for (int i=0, j; i < n; i++) {
        auto sln = slide_puzzle(SlidingPuzzleSolver::create_random_board(size));
    }
    auto end = std::chrono::steady_clock::now();
    double seconds_elapsed = std::chrono::duration<double, std::milli>(end-start).count();
    cout << "Evaluated " << n << " puzzles of size " << size << "-by-" << size << "  in: " << seconds_elapsed/1000.f << "s.\n";
    cout << "Avg. time per "<< size << "-by-" << size <<" puzzle: " << seconds_elapsed/n << "ms.\n";
#endif
    std::cin.get();
    return 0;
}