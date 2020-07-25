#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <iterator>


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
private:
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
private:
    const std::vector<std::vector<int>>& _arr;
    std::vector<std::vector<GridNode*>> _grid;
    std::unordered_map<int, GridNode> _quick_lookup;
    std::vector<int> _movelist;
    GridNode * _p_entry_point;
    const uint _map_size;
private:
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
        
/*        //swap nodes' row & col fields
        int temp_col = _quick_lookup.at(0).col, temp_row = _quick_lookup.at(0).row;  
        _quick_lookup.at(0).row = _quick_lookup.at(x).row;
        _quick_lookup.at(0).col = _quick_lookup.at(x).col;
        _quick_lookup.at(x).row = temp_row;
        _quick_lookup.at(x).row = temp_col;*/
        
        //temporarily extract nodes & swap their placements in map
        auto temp_x = _quick_lookup.extract(x), temp_0 = _quick_lookup.extract(0);
        temp_0.key() = x;
        temp_x.key() = 0;

        //put 'em back w/o copying them
        _quick_lookup.insert(std::move(temp_0));
        _quick_lookup.insert(std::move(temp_x));

    }

    GridNode* _swap_zero_with(const int x) {
        if (!x || !_quick_lookup.at(x).movable)
            return &_quick_lookup.at(0);

        const auto& chk = _quick_lookup.at(0);
        if ((chk.up && chk.up->val==x) || (chk.right && chk.right->val==x)
            || (chk.down && chk.down->val==x) || (chk.left && chk.left->val==x))
            _swap(x);
        if (chk.val) {
            _movelist.emplace_back(x);
            _print_board();
        }
        return &_quick_lookup.at(0);
    }

    int _get_distance_between(const GridNode* start, const GridNode* end) const{
        int col_diff = std::abs(start->col - end->col);
        int row_diff = std::abs(start->row - end->row);
        return std::abs(row_diff) + std::abs(col_diff);
    }


    template<typename D>
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

    void _move_zero_to_x(int x, int avoid=-1){
        // TODO: CLEAN UP THIS FUNCTION
        //  - Make helper function that can make a series of moves given
        //      a set of enum directions
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
                        if (_explore_path(empty_space, DOWN, RIGHT, RIGHT, UP)) {
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                        }
                        else if (_explore_path(empty_space, UP, RIGHT, RIGHT, DOWN)) {
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                        }
                        else if (_explore_path(empty_space, DOWN, RIGHT)){
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                        }
                        else if (_explore_path(empty_space, UP, RIGHT)){
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                        }
                    }
                else    //CASE 0 is RIGHT of X
                    if (empty_space->left && empty_space->left->val != avoid && empty_space->left->movable)
                        empty_space = _swap_zero_with(empty_space->left->val);
                    else {
                        if (_explore_path(empty_space, DOWN, LEFT, LEFT, UP)) {
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                        }
                        else if (_explore_path(empty_space, UP, LEFT, LEFT, DOWN)) {
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                        }
                        else if (_explore_path(empty_space, DOWN, LEFT)){
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                        }
                        else if (_explore_path(empty_space, UP, LEFT)){
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                        }
                    }
            }

            // vertical movement logic
            if (empty_space->row != desired_row) {
                //CASE 0 is ABOVE X
                if (empty_space->row < node_x->row)
                    if (empty_space->down && empty_space->down->val != avoid && empty_space->down->movable)
                        empty_space = _swap_zero_with(empty_space->down->val);
                    else {
                        if (_explore_path(empty_space, RIGHT, DOWN, DOWN, LEFT)) {
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                        }
                        else if (_explore_path(empty_space, LEFT, DOWN, DOWN, RIGHT)) {
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                        }
                        else if (_explore_path(empty_space, RIGHT, DOWN)){
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                        }
                        else if (_explore_path(empty_space, LEFT, DOWN)){
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->down->val);
                        }
                    }
                else    //CASE 0 is BELOW X
                    if (empty_space->up && empty_space->up->val != avoid && empty_space->up->movable)
                        empty_space = _swap_zero_with(empty_space->up->val);
                    else {
                        if (_explore_path(empty_space, RIGHT, UP, UP, LEFT)) {
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->left->val);
                        }
                        else if (_explore_path(empty_space, LEFT, UP, UP, RIGHT)) {
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                            empty_space = _swap_zero_with(empty_space->right->val);
                        }
                        else if (_explore_path(empty_space, RIGHT, UP)){
                            empty_space = _swap_zero_with(empty_space->right->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                        }
                        else if (_explore_path(empty_space, LEFT, UP)){
                            empty_space = _swap_zero_with(empty_space->left->val);
                            empty_space = _swap_zero_with(empty_space->up->val);
                        }
                    }
            }
            
        }

/*
        //TODO: fix collision logic blocks to move 0 *around* avoid

        // Row switching logic
        if (_quick_lookup.at(0).row < _quick_lookup.at(x).row)
            do { //collision logic
                if (_quick_lookup.at(0).down && _quick_lookup.at(0).down->val == avoid){
                    if (_quick_lookup.at(0).right)
                        _swap_zero_with(_quick_lookup.at(0).right->val);
                    else
                        _swap_zero_with(_quick_lookup.at(0).left->val);
                }
                _swap_zero_with(_quick_lookup.at(0).down->val);
            } while (_quick_lookup.at(0).row < _quick_lookup.at(x).row && _quick_lookup.at(0).down);
        else
            do { //collision logic
                if (_quick_lookup.at(0).up && _quick_lookup.at(0).up->val == avoid){
                    if (_quick_lookup.at(0).right)
                        _swap_zero_with(_quick_lookup.at(0).right->val);
                    else
                        _swap_zero_with(_quick_lookup.at(0).left->val);
                }
                _swap_zero_with(_quick_lookup.at(0).up->val);
            } while (_quick_lookup.at(x).row < _quick_lookup.at(0).row);

        // Column switching logic
        if (_quick_lookup.at(0).col < _quick_lookup.at(x).col)
            do { //collision logic
                if (_quick_lookup.at(0).right && _quick_lookup.at(0).right->val == avoid){
                    if (_quick_lookup.at(0).down)
                        _swap_zero_with(_quick_lookup.at(0).down->val);
                    else
                        _swap_zero_with(_quick_lookup.at(0).up->val);
                }
                _swap_zero_with(_quick_lookup.at(0).right->val);
            } while (_quick_lookup.at(0).col < _quick_lookup.at(x).col && _quick_lookup.at(0).right);
        else
            do { //collision logic
                if (_quick_lookup.at(0).left && _quick_lookup.at(0).left->val == avoid){
                    if (_quick_lookup.at(0).down)
                        _swap_zero_with(_quick_lookup.at(0).down->val);
                    else
                        _swap_zero_with(_quick_lookup.at(0).up->val);
                }
                _swap_zero_with(_quick_lookup.at(0).up->val);
            } while (_quick_lookup.at(x).col < _quick_lookup.at(0).col);
*/

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

public:
    explicit SlidingPuzzleSolver (const std::vector<std::vector<int>>& arr)
      : _arr(arr), _map_size(arr.size() * arr.size())
    {
      _create_map();
      _p_entry_point = &_quick_lookup.at(arr[0][0]);
    }

    std::vector<int> solve(){
        _print_board();
        GridNode * s_entry_point = _p_entry_point;
        enum direction dir;
        uint desired_row, desired_col;
        for (int num = 1; num < _map_size; ++num) {
            desired_row = (num-1) / _arr.size();
            desired_col = (num-1) % _arr.size();
            if (desired_col == _arr.size() - 2)
                ++desired_col;

            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, LEFT)) > NONE){
                _move_x_in_dir(num, dir);
            }
            while ((dir = determine_move_dir(num, _grid[desired_row][desired_col]->val, UP)) > NONE) {
                _move_x_in_dir(num, dir);
            }
            if (desired_col != _arr.size()-1)
                _quick_lookup.at(num).movable = false;
        }


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
    //auto board1 = create_random_board(4);
    vector<vector<int>> board2 = {
            {14,      4,       11,      1},
            {12,      9,       8,       7},
            {10,      13,      15,      6},
            {5,       3,       2,       0}
    };
    auto movelist1 = slide_puzzle(board2);

/*    auto board2 = create_random_board();
    auto movelist2 = slide_puzzle(board2);

    auto board3 = create_random_board();
    auto movelist3 = slide_puzzle(board3);*/

    /*){
        cout << "Swapped 0 with " << move << endl;
    }*/
    std::cin.get();
    return 0;
}