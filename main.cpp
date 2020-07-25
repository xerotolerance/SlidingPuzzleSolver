#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <iterator>
#include <tuple>
#include <functional>

using std::vector;
using std::cout;
using std::endl;


#if 0
void * operator new(size_t size){
    cout << "new was called!\n";
    return malloc(size);
}

void operator delete(void * ptr){
    cout << "delete was called!\n";
    free(ptr);
}
#endif

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
            bool movable = true;
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
    const uint _map_size;

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
        cout << end_dec << endl;
    }
    void _create_map() {
        int row=0, col=0;
        GridNode *node_before=nullptr, *node_above=nullptr;
        for (const auto& row_it : _arr){
            col = 0;
            _grid.emplace_back(std::vector<GridNode*>());
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

    void _solve_row(int row_num){
        GridNode * empty_space;
        enum direction dir;
        uint desired_row, desired_col;
        for (uint i=0, num = row_num * _arr.size() + 1; i < _arr.size(); i++, num++){
            if (!_quick_lookup.at(num).movable)
                continue;
            desired_row = (num-1) / _arr.size();
            desired_col = (num-1) % _arr.size();

            // Put 2nd-to-last in row in rightmost spot
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
            else if (desired_col == _arr.size()) {
                desired_col--;
                desired_row++;
            }

            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, LEFT)) > NONE){
                _move_x_in_dir(num, dir);
            }
            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, UP)) > NONE) {
                _move_x_in_dir(num, dir);
            }

            if (desired_col < _arr.size()-2)
                _quick_lookup.at(num).movable = false;

            desired_col = (num-1) % _arr.size();
            desired_row = (num-1) / _arr.size();
            if (desired_col == _arr.size() -1){
                for (int j = 0;; ++j) {
                    if (_grid[desired_row][desired_col]->val == num){
                        _quick_lookup.at(num).movable = false;
                        _quick_lookup.at(num).left->movable = false;
                        break;
                    }
                    empty_space = &_quick_lookup.at(0);
                    switch (j % 4) {
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
                    }
                }
            }
        }
    }

    void _solve_column(int col_num){
        GridNode * empty_space;
        enum direction dir;
        uint desired_row, desired_col;
        for (uint num = col_num * _arr.size() + 1; num < _map_size; num+=_arr.size()){
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
            else if (desired_row == _arr.size()) {
                desired_col++;
                desired_row--;
            }

            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, DOWN)) > NONE) {
                _move_x_in_dir(num, dir);
            }
            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, RIGHT)) > NONE){
                _move_x_in_dir(num, dir);
            }


            if (desired_row < _arr.size()-2)
                _quick_lookup.at(num).movable = false;

            desired_col = (num-1) % _arr.size();
            desired_row = (num-1) / _arr.size();
            if (desired_row == _arr.size() -1){
                for (int j = 0;; ++j) {
                    if (_grid[desired_row][desired_col]->val == num){
                        _quick_lookup.at(num).movable = false;
                        _quick_lookup.at(num).up->movable = false;
                        break;
                    }
                    empty_space = &_quick_lookup.at(0);
                    switch (j % 4) {
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
                    }
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

    template<typename D> //*Base Case
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
        } return _swap_zero_with(explorer->val);
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

private:    //Non-Recursive Template Functions
    template<typename... Ts>
    GridNode * _apply_detour(Ts... paths){
        return _apply_detour_path(paths...);
    }


public:
    explicit SlidingPuzzleSolver (const std::vector<std::vector<int>>& arr)
      : _arr(arr), _map_size(arr.size() * arr.size())
    {
      _create_map();
      _p_entry_point = &_quick_lookup.at(arr[0][0]);
    }

    std::vector<int> solve(){
        _print_board();
        // Methodical Solving
        for (int num = 0; num < _arr.size()-2; ++num) {
            _solve_row(num);
            if (num < _arr.size()-3)
                _solve_column(num);
        }
        // TODO:Trial & Error solving for bottom right 3x2 rectangle
        return _movelist;
    }
};

std::vector<int> slide_puzzle(const std::vector<std::vector<int>> &arr)
{
    SlidingPuzzleSolver sps = SlidingPuzzleSolver(arr);
    return sps.solve();
}


vector<vector<int>> create_random_board(int size=0){
    uint length_of_square = size < 11 && size > 2? size : 3 + std::rand() % 8;
    uint num_choices = length_of_square * length_of_square;
    vector<vector<int>> board;
    vector<int> validrange;

    for (int i=0; i < length_of_square; i++)
        board.emplace_back(vector<int>());

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

int main() {
    std::srand(std::time(nullptr)); //"ok, psuedo-random," but its fine for this..."
    auto board1 = create_random_board(4);
    vector<vector<int>> board2 = {
            {14,      4,       11,      1},
            {12,      9,       8,       7},
            {10,      13,      15,      6},
            {5,       3,       2,       0}
    };
    auto movelist1 = slide_puzzle(board1);

/*    auto board2 = create_random_board();
    auto movelist2 = slide_puzzle(board2);

    auto board3 = create_random_board();
    auto movelist3 = slide_puzzle(board3);*/

    std::cin.get();
    return 0;
}