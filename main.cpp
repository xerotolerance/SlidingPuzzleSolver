#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <iterator>
#include <tuple>
#include <functional>
#include <chrono>

using std::vector;
using std::cout;
using std::endl;

static std::size_t heap_storage_in_use = 0;


class SlidingPuzzleSolver{
private:    // Private Classes, Structs, & Enums
    class GridNode{
        private:
            friend SlidingPuzzleSolver;
            friend std::ostream& operator<<(std::ostream& os, const GridNode& node){
                return os << node.val;
            };
            GridNode *up, *right, *down, *left;
            int val, row, col;
            bool movable = true, visited=false;
        public:
            explicit GridNode(int val=-1, int row=-1, int col=-1, GridNode* up= nullptr, GridNode* right= nullptr, GridNode* down= nullptr, GridNode* left= nullptr)
                : val(val), row(row), col(col), up(up), right(right), down(down), left(left) {};

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
    enum direction{NONE=-1, UP, DOWN, LEFT, RIGHT};

private:    // Private Fields
    const std::vector<std::vector<int>>& _arr;
    std::vector<std::vector<GridNode*>> _grid;
    std::unordered_map<int, GridNode> _quick_lookup;
    std::vector<int> _movelist;
    GridNode * _p_entry_point;
    const unsigned int _map_size;
    enum direction _backtrack_to = NONE;
    int _eject_me;

private:    // Private Functions
    void _print_board(const std::string& end_dec = "=============") const{
        GridNode* row_head = _p_entry_point, *item;
        while(row_head) {
            item = row_head;
            while (item){
                cout << item->val << "\t";
                item = item->right;
            }
            cout << "\n";
            row_head = row_head->down;
        }
        cout << end_dec.c_str() << endl;
    }
    void _create_map() {
        int row=0, col=0;
        GridNode *node_before=nullptr, *node_above=nullptr;
        _grid.reserve(_arr.size());
        _quick_lookup.reserve(_map_size);
        for (const auto& row_it : _arr){
            col = 0;
            _grid.emplace_back(std::vector<GridNode*>());
            _grid.back().reserve(_arr.size());
            for (const int& item : row_it){
                _quick_lookup.emplace(item, std::move(GridNode(item, row, col, node_above, nullptr, nullptr, node_before)));
                _grid[row].emplace_back(&_quick_lookup.at(item));
                if (node_before) {
                    node_before->right = &_quick_lookup.at(item);
                }
                if (node_above) {
                    node_above->down = &_quick_lookup.at(item);
                    node_above = node_above->right;
                }
                node_before = &_quick_lookup.at(item);
                ++col;
            }
            node_above = &_quick_lookup.at(*(row_it.begin()));
            node_before = nullptr;
            ++row;
        }
    }
    void _swap(const int x){ //DO NOT USE REFERENCE HERE, x will be overwritten if a {GridNode}.val is passed in!
        if (!x || !_quick_lookup.at(x).movable)
            return;

        //swap nodes' value fields
        _quick_lookup.at(0).val = x;
        _quick_lookup.at(x).val = 0;
        
        //temporarily extract nodes & swap their placements in map
        auto temp_x = _quick_lookup.extract(x), temp_0 = _quick_lookup.extract(0);
        temp_0.key() = x;
        temp_x.key() = 0;

        //put 'em back w/o copying them
        _quick_lookup.insert(std::move(temp_0));
        _quick_lookup.insert(std::move(temp_x));

    }
    void _move_zero_to_x(int x, int avoid=-1){
        if (x <= _quick_lookup.at(0).val || !_quick_lookup.at(x).movable)
            return;
        GridNode *empty_space, *node_x;

        int desired_row = _quick_lookup.at(x).row, desired_col = _quick_lookup.at(x).col;
        while (true){   // exits when 0 has been moved to spot where X was originally
            empty_space = &_quick_lookup.at(0);
            node_x = &_quick_lookup.at(x);
            if (empty_space->row == desired_row && empty_space->col == desired_col)
                break; //exit logic

            // horizontal movement logic
            if (empty_space->col != desired_col) {
                //CASE 0 is LEFT of X
                if (empty_space->col < node_x->col)
                    if (empty_space->right && empty_space->right->val != avoid && empty_space->right->movable)
                        empty_space = _swap_zero_with(empty_space->right->val);
                    else {
                        int horizontal_diff = empty_space->col - node_x->col;
                        std::tuple path1{DOWN, RIGHT, RIGHT, UP};
                        std::tuple path2{ UP,   RIGHT, RIGHT, DOWN};
                        std::tuple path3{DOWN, RIGHT};
                        std::tuple path4{UP,   RIGHT};
                        empty_space = _apply_detour(path1, path2, path3, path4);
                    }
                else    //CASE 0 is RIGHT of X
                if (empty_space->left && empty_space->left->val != avoid && empty_space->left->movable)
                    empty_space = _swap_zero_with(empty_space->left->val);
                else {
                    std::tuple path1{DOWN, LEFT, LEFT, UP};
                    std::tuple path2{ UP,   LEFT, LEFT, DOWN};
                    std::tuple path3{DOWN, LEFT};
                    std::tuple path4{UP,   LEFT};
                    empty_space = _apply_detour(path1, path2, path3, path4);
                }
            }

            // vertical movement logic
            if (empty_space->row != desired_row) {
                //CASE 0 is ABOVE X
                if (empty_space->row < node_x->row)
                    if (empty_space->down && empty_space->down->val != avoid && empty_space->down->movable)
                        empty_space = _swap_zero_with(empty_space->down->val);
                    else {
                        std::tuple path1{RIGHT, DOWN, DOWN, LEFT};
                        std::tuple path2{LEFT, DOWN, DOWN, RIGHT};
                        std::tuple path3{RIGHT, DOWN};
                        std::tuple path4{LEFT, DOWN};
                        empty_space = _apply_detour(path1, path2, path3, path4);
                    }
                else    //CASE 0 is BELOW X
                if (empty_space->up && empty_space->up->val != avoid && empty_space->up->movable)
                    empty_space = _swap_zero_with(empty_space->up->val);
                else {
                    std::tuple path1{RIGHT, UP, UP, LEFT};
                    std::tuple path2{LEFT, UP, UP, RIGHT};
                    std::tuple path3{RIGHT, UP};
                    std::tuple path4{LEFT, UP};
                    empty_space = _apply_detour(path1, path2, path3, path4);
                }
            }

        }
    }
    GridNode* _swap_zero_with(const int x) {
        if (!x || !_quick_lookup.at(x).movable)
            return &_quick_lookup.at(0);

        const auto& chk = _quick_lookup.at(0);
        if ((chk.up && chk.up->val==x) || (chk.right && chk.right->val==x)
            || (chk.down && chk.down->val==x) || (chk.left && chk.left->val==x))
            _swap(x);
        if (chk.val) {
            if (!_movelist.empty() && _movelist.back() == x)
                _movelist.pop_back();
            else
                _movelist.emplace_back(x);
            _print_board();
        }
        return &_quick_lookup.at(0);
    }
    GridNode* _move_x_in_dir(int x, enum direction direction){
        GridNode * x_adjacent;
        switch(direction){
            case RIGHT:
                x_adjacent = _quick_lookup.at(x).right;
                break;
            case DOWN:
                x_adjacent = _quick_lookup.at(x).down;
                break;
            case UP:
                x_adjacent = _quick_lookup.at(x).up;
                break;
            case LEFT:
                x_adjacent = _quick_lookup.at(x).left;
                break;
            default: return &_quick_lookup.at(0);
        }

        if (!x_adjacent)
            return &_quick_lookup.at(0);

        _move_zero_to_x(x_adjacent->val, x);
        return _swap_zero_with(_quick_lookup.at(x).val);

    }
    enum direction determine_move_dir(int start, int end, enum direction orientation){
        if (orientation <= NONE)
            return NONE;

        int start_row = _quick_lookup.at(start).row;
        int end_row = _quick_lookup.at(end).row;
        int start_col = _quick_lookup.at(start).col;
        int end_col = _quick_lookup.at(end).col;

        if (orientation < LEFT) {
            if (end_row - start_row < 0)
                return UP;
            else if (end_row - start_row > 0)
                return DOWN;
        }else {
            if (end_col - start_col < 0)
                return LEFT;
            else if (end_col - start_col > 0)
                return RIGHT;
        }
        return NONE;
    };
#if 0
    void _solve_row(int row_num){
        GridNode * empty_space;
        enum direction dir;
        unsigned int desired_row, desired_col;
        for (unsigned int i=0, num = row_num * _arr.size() + 1; i < _arr.size(); i++, num++){
            if (!_quick_lookup.at(num).movable)
                continue;
            desired_row = (num-1) / _arr.size();
            desired_col = (num-1) % _arr.size();

            // Put 2nd-to-last in row in the rightmost spot
            if (desired_col == _arr.size() - 2) {
                ++desired_col;
                if (_grid[desired_row][_arr.size()-2]->val == num) {
                    for (int j = 0; j < 2; j++)
                        empty_space = _move_x_in_dir(_quick_lookup.at(num).val, DOWN);
                }
                if (_grid[desired_row][_arr.size()-2]->val == num+1 || _grid[desired_row][_arr.size()-1]->val == num+1) {
                    for (int j = 0; j < 2; j++)
                        empty_space = _move_x_in_dir(_quick_lookup.at(num+1).val, DOWN);
                }
            }

            // Put last-in-row underneath 2nd-to-last
            if (i == _arr.size()-1) {
                desired_row++;
            }

            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, LEFT)) > NONE){
                _better_movement(num, dir);
            }
            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, UP)) > NONE) {
                _better_movement(num, dir);
            }

            //if (desired_col < _arr.size()-1)
            _quick_lookup.at(num).movable = false;

            desired_col = (num-1) % _arr.size();
            desired_row = (num-1) / _arr.size();
            if (desired_col == _arr.size() -1){
                for (int j = 0;; ++j) {
                    if (_grid[desired_row][desired_col]->val == num){
                        _quick_lookup.at(num-1).movable = false;
                        _quick_lookup.at(num).movable = false;
                        break;
                    }else{
                        _quick_lookup.at(num-1).movable = true;
                        _quick_lookup.at(num).movable = true;
                    }
                    empty_space = &_quick_lookup.at(0);
                    _better_movement(num,UP);
                    /*switch (j % 4) {
                        case 0:
                            _swap_zero_with(empty_space->up->val);
                            break;
                        case 1:
                            _swap_zero_with(empty_space->right->val);
                            break;
                        case 2:
                            _swap_zero_with(empty_space->down->val);
                            break;
                        case 3:
                            _swap_zero_with(empty_space->left->val);
                            break;
                    }*/
                }
            }
        }
    }

    void _solve_column(int col_num){
        GridNode * empty_space;
        enum direction dir;
        unsigned int desired_row, desired_col;
        for (unsigned int num = col_num + 1; num < _map_size; num+=_arr.size()){
            if (!_quick_lookup.at(num).movable)
                continue;
            desired_row = (num-1) / _arr.size();
            desired_col = (num-1) % _arr.size();

            // Put 2nd-to-last in row in bottom-most spot
            if (desired_row == _arr.size() - 2) {
                ++desired_row;
                if (_grid[_arr.size()-2][desired_col]->val == num) {
                    for (int j = 0; j < 2; j++)
                        empty_space = _move_x_in_dir(_quick_lookup.at(num).val, RIGHT);
                }
                if (_grid[_arr.size()-2][desired_col]->val == num+_arr.size() || _grid[_arr.size() - 1][desired_col]->val == num+_arr.size()) {
                    for (int j = 0; j < 2; j++)
                        empty_space = _move_x_in_dir(_quick_lookup.at(num+_arr.size()).val, RIGHT);
                }
            }
                // Put last-in-row to the right of 2nd-to-last
            if (num + _arr.size() > _map_size) {
                desired_col++;
            }

            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, DOWN)) > NONE) {
                _better_movement(num, dir);
            }
            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, RIGHT)) > NONE){
                _better_movement(num, dir);
            }


            //if (desired_row < _arr.size()-2)
            _quick_lookup.at(num).movable = false;

            desired_col = (num-1) % _arr.size();
            desired_row = (num-1) / _arr.size();
            if (desired_row == _arr.size() -1){
                for (int j = 0;; ++j) {
                    if (_grid[desired_row][desired_col]->val == num){
                        _quick_lookup.at(num-_arr.size()).movable = false;
                        _quick_lookup.at(num).movable = false;
                        break;
                    }else{
                        _quick_lookup.at(num-_arr.size()).movable = true;
                        _quick_lookup.at(num).movable = true;
                    }
                    empty_space = &_quick_lookup.at(0);
                    _better_movement(num, LEFT);
/*                    switch (j % 4) {
                        case 0:
                            _swap_zero_with(empty_space->down->val);
                            break;
                        case 1:
                            _swap_zero_with(empty_space->left->val);
                            break;
                        case 2:
                            _swap_zero_with(empty_space->up->val);
                            break;
                        case 3:
                            _swap_zero_with(empty_space->right->val);
                            break;
                    }*/
                }
            }
        }
    }
#endif

    void _solve_row(int row_num){
        for (int col_num=0, num; col_num < _arr.size(); col_num++) {
            num = row_num * _arr.size() + col_num + 1;
            if (col_num < _arr.size()-2){
                _better_movement(num, _grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num)
                    _quick_lookup.at(num).movable = false;
            }else if(col_num == _arr.size() - 2){
                if (_grid[row_num][col_num]->val == num || _grid[row_num][col_num+1]->val == num)
                    _better_movement(num,_quick_lookup.at(num).down->down->val);
                if (num + 1==_grid[row_num][col_num]->val || num + 1==_grid[row_num][col_num+1]->val)
                    _better_movement(num+1,_quick_lookup.at(num+1).down->down->val);
                _better_movement(num, _grid[row_num][col_num+1]->val, num+1, RIGHT);
                if (_grid[row_num][col_num+1]->val == num /*&& _grid[row_num][col_num]->val!= num + 1*/) {
                    _quick_lookup.at(num).movable = false;
                    if (num + 1==_grid[row_num][col_num]->val /*|| num + 1==_grid[row_num][col_num+1]->val*/)
                        _better_movement(num+1,_quick_lookup.at(num+1).down->down->val);
                }
            }else{
                _better_movement(num, _grid[row_num+1][col_num]->val);
                if (_grid[row_num+1][col_num]->val == num)
                    _quick_lookup.at(num-1).movable = true;
                _better_movement(num,_grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num && _grid[row_num][col_num-1]->val==num-1){
                    _quick_lookup.at(num).movable = false;
                    _quick_lookup.at(num-1).movable = false;
                }
            }
        }
    }
    void _solve_column(int col_num){
        for (int first_in_row = 1, num, row_num; first_in_row < _map_size; first_in_row+=_arr.size()) {
            num = first_in_row + col_num;
            row_num = first_in_row / _arr.size();
            if(first_in_row < _map_size - 2 * _arr.size()){
                _better_movement(num, _grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num)
                    _quick_lookup.at(num).movable = false;
            }else if (first_in_row < _map_size - 1 * _arr.size()){
                if (_grid[row_num][col_num]->val == num || _grid[row_num+1][col_num]->val == num)
                    _better_movement(num,_quick_lookup.at(num).right->right->val);
                if (num + _arr.size()==_grid[row_num][col_num]->val || num + _arr.size()==_grid[row_num+1][col_num]->val)
                    _better_movement(num+_arr.size(),_quick_lookup.at(num+_arr.size()).right->right->val);
                _better_movement(num, _grid[row_num+1][col_num]->val, num + _arr.size(), DOWN);
                if (_grid[row_num+1][col_num]->val == num /*&& _grid[row_num][col_num]->val != num + _arr.size()*/) {
                    _quick_lookup.at(num).movable = false;
                    if (num + _arr.size()==_grid[row_num][col_num]->val /*|| num + _arr.size()==_grid[row_num+1][col_num]->val*/)
                        _better_movement(num+_arr.size(),_quick_lookup.at(num+_arr.size()).right->right->val);
                }
            }else{
                _better_movement(num, _grid[row_num][col_num+1]->val);
                if (_grid[row_num][col_num+1]->val == num)
                    _quick_lookup.at(num-_arr.size()).movable = true;
                _better_movement(num,_grid[row_num][col_num]->val);
                if (_grid[row_num][col_num]->val == num && _grid[row_num-1][col_num]->val==num-_arr.size()){
                    _quick_lookup.at(num).movable = false;
                    _quick_lookup.at(num-_arr.size()).movable = false;
                }
            }
        }
    }


private: // Recursive Private Template Functions
    template<typename D>    //*Base Case
    bool _explore_path(GridNode* start, D direction){
        if (!start || !start->movable)
            return false;

        GridNode * explorer;
        switch(direction){
            case RIGHT:
                explorer = start->right;
                break;
            case DOWN:
                explorer = start->down;
                break;
            case UP:
                explorer = start->up;
                break;
            case LEFT:
                explorer = start->left;
                break;
            default: return false;
        } return explorer && explorer->movable;
    }

    template<typename D, typename... Ds>
    bool _explore_path(GridNode* start, D direction, Ds... directions){
        if (!start || !start->movable)
            return false;
        GridNode * explorer;
        switch(direction){
            case RIGHT:
                explorer = start->right;
                break;
            case DOWN:
                explorer = start->down;
                break;
            case UP:
                explorer = start->up;
                break;
            case LEFT:
                explorer = start->left;
                break;
            default: return false;
        } return _explore_path(explorer, directions...);
    }

    /*template<typename D> //*Base Case*/
    GridNode * _move_zero_along_path(const vector<enum direction>& path){
        GridNode * empty_space = &_quick_lookup.at(0);
        int swap_me=-1;
        for (auto dir : path){
           switch (dir) {
               case NONE:
                   break;
               case UP:
                   swap_me = empty_space->up->val;
                   break;
               case DOWN:
                   swap_me = empty_space->down->val;
                   break;
               case LEFT:
                   swap_me = empty_space->left->val;
                   break;
               case RIGHT:
                   swap_me = empty_space->right->val;
                   break;
           }
           empty_space = _swap_zero_with(swap_me);
       }
        return empty_space;
    }

    template<typename D>
    GridNode * _move_along_path(D direction){
        GridNode * explorer, *start = &_quick_lookup.at(0);
        switch(direction){
            case RIGHT:
                explorer = start->right;
                break;
            case DOWN:
                explorer = start->down;
                break;
            case UP:
                explorer = start->up;
                break;
            case LEFT:
                explorer = start->left;
                break;
            default: return start;
        }
        explorer = _swap_zero_with(explorer->val);
        return explorer;
    }

    template<typename D, typename... Ds>
    GridNode * _move_along_path(D direction, Ds... directions){
        GridNode * explorer, *start = &_quick_lookup.at(0);
        switch(direction){
            case RIGHT:
                explorer = start->right;
                break;
            case DOWN:
                explorer = start->down;
                break;
            case UP:
                explorer = start->up;
                break;
            case LEFT:
                explorer = start->left;
                break;
            default: return start;
        }
        explorer = _swap_zero_with(explorer->val);
        return _move_along_path(directions...);
    }

    template<typename T>    //*Base Case
    GridNode * _apply_detour_path(T path){
        GridNode * empty_space = &_quick_lookup.at(0);
        auto explore = [this, &empty_space](const auto ...directions){
            return _explore_path(empty_space, directions...);
        };
        auto move_along = [this](auto ...directions){
            return _move_along_path(directions...);
        };

        if (std::apply(explore, path))
            return std::apply(move_along, path);
        return empty_space;
    };

    template<typename T, typename... Ts>
    GridNode * _apply_detour_path(T path, Ts ...paths){
        GridNode * empty_space = &_quick_lookup.at(0);
        auto explore = [this, &empty_space](const auto ...directions){
            return _explore_path(empty_space, directions...);
        };
        auto move_along = [this](auto ...directions){
            return _move_along_path(directions...);
        };

        if (std::apply(explore, path))
            return std::apply(move_along, path);
        return _apply_detour_path(paths...);
    };


    template <typename... Ints>
    GridNode * _find_path_recursive(vector<enum direction>& path, GridNode * const start,
            const GridNode * const end, const enum direction came_from, Ints... Ignore) {
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
        GridNode *chk, *explorer;
        enum direction coming_from = came_from;
        for (auto dir : explore_order){
            if (/*dir == coming_from ||*/ dir == came_from) {
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

    template <typename... Ints>
    GridNode * _better_movement(int move_me, int to_me, int reject_me=-1, enum direction orientation=NONE){
        if (move_me == to_me){
            //cout << "No need to move " << move_me << ": " << to_me << " is already in place.\nSkipping...\n";
            return &_quick_lookup.at(0);
        }

        //Step One: Determine where to move the empty_space to


        GridNode * empty_space = &_quick_lookup.at(0);
        GridNode * start_space = &_quick_lookup.at(move_me);
        GridNode * end_space = &_quick_lookup.at(to_me);
        GridNode * step = start_space;

        int end_space_row = end_space->row, end_space_col = end_space->col;

        std::vector<enum direction> path_from_start_to_end;
        std::vector<enum direction> path_from_empty_to_step;
        enum direction coming_from;



        auto discovered_end = _find_path_recursive(path_from_start_to_end,start_space,end_space,NONE,0,move_me);
        if (discovered_end == end_space){
            /*cout << "Valid path from "<< start_space->val <<" to " << end_space->val << " was found:\n";
            _print_path(path_from_start_to_end);*/
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
                    /*cout << "Valid path from 0 to " << step->val << " was found:\n";
                    _print_path(path_from_empty_to_step);*/
                    GridNode * swap_me = empty_space;
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
                                        _better_movement(reject_me, _quick_lookup.at(reject_me).right->right->val);
                                        return _better_movement(move_me,_grid[end_space_row][end_space_col]->val,reject_me, orientation);
                                    }
                                    break;
                                case LEFT:
                                case RIGHT:
                                    if (reject_me == _grid[end_space_row][_arr.size()-1]->val ||
                                        reject_me == _grid[end_space_row][_arr.size()-2]->val){
                                        _better_movement(reject_me, _quick_lookup.at(reject_me).down->down->val);
                                        return _better_movement(move_me,_grid[end_space_row][end_space_col]->val,reject_me, orientation);
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
                    for (auto row : _arr){
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


private:    //Non-Recursive Template Functions
    template<typename... Ts>
    GridNode * _apply_detour(Ts... paths){
        return _apply_detour_path(paths...);
    }

private: // Static Functions
    static vector<enum direction> _get_exploration_order(GridNode *const start, const GridNode *end) {
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

public: //Constructors & Destructors
    explicit SlidingPuzzleSolver (const std::vector<std::vector<int>>& arr)
      : _arr(arr), _map_size(arr.size() * arr.size())
    {
      _create_map();
      _p_entry_point = &_quick_lookup.at(arr[0][0]);
      _eject_me = -1;
    }

public: //Public Functions
    std::vector<int> solve(){
        _print_board();


        // Methodical Solving
        //auto start = std::chrono::steady_clock::now();
        for (int num = 0; num < _arr.size()-2; ++num) {
            _solve_row(num);
            if (num < _arr.size()-3){
                _solve_column(num);
            }
        }
        //auto end = std::chrono::steady_clock::now();
        //auto elapsed = std::chrono::duration<double, std::milli>(end-start);
        //cout << _arr.size() << "-by-" << _arr.size() << " puzzle solved in: " << elapsed.count() << "ms\n";
        //_print_board();
        // TODO:Trial & Error solving for bottom right 3x2 rectangle


        return _movelist;
    }
};

std::vector<int> slide_puzzle(const std::vector<std::vector<int>> &arr)
{
    SlidingPuzzleSolver sps = SlidingPuzzleSolver(arr);
    auto res = sps.solve();
    //cout << endl << endl;
    return res;
}


vector<vector<int>> create_random_board(int size=0){
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

#if 0
void *operator new (std::size_t size){
    cout << "::new : Allocated " << size << " bytes.\n";
    heap_storage_in_use += size;
    return malloc(size);
}

void operator delete (void* ptr) noexcept {
    cout << "::delete : De-allocated item at " << ptr << ".\n";
    //heap_storage_in_use -= size;
    free(ptr);
}
#endif

int main() {
    std::srand(std::time(nullptr)); //"ok, psuedo-random," but its fine for this..."

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

    /*slide_puzzle(problem_board1);   //success
    slide_puzzle(problem_board2);   //success
    slide_puzzle(problem_board3);   //success
    slide_puzzle(problem_board4);   //success*/
    slide_puzzle(problem_board5);   //FAILED
#endif
#if 0
    int size=3, n=1000000;
    auto start = std::chrono::steady_clock::now();
    for (int i=0; i < n; i++) {
        slide_puzzle(create_random_board(size));
    }
    auto end = std::chrono::steady_clock::now();
    double seconds_elapsed = std::chrono::duration<double, std::milli>(end-start).count();
    cout << "Solved " << n << " puzzles of size " << size << "-by-" << size << "  in: " << seconds_elapsed << "ms.";
#endif
    std::cin.get();
    return 0;
}