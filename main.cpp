#include <iostream>
#include <vector>
#include <map>
#include <cassert>

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
    class _Node{
        private:
            friend SlidingPuzzleSolver;
            friend std::ostream& operator<<(std::ostream& os, const _Node& node){
                return os << node.val;
            };
            _Node *up, *right, *down, *left;
            int val;
        public:
            explicit _Node(int val=-1, _Node* up= nullptr, _Node* right= nullptr, _Node* down= nullptr, _Node* left= nullptr)
                : val(val), up(up), right(right), down(down), left(left) {};

    };
    enum direction{UP=0, DOWN, LEFT, RIGHT};
private:
    const std::vector<std::vector<int>>& _arr;
    std::map<int, _Node> _lookup;
    std::vector<int> _movelist;
    _Node *_p_entry_point;
    const uint _map_size;
private:
    void _print_board(const std::string& end_dec = "=============") const{
        _Node* row_head = _p_entry_point, *item=nullptr;
        while(row_head) {
            item = row_head;
            while (item){
                cout << item->val << " ";
                item = item->right;
            }
            cout << endl;
            row_head = row_head->down;
        }
        cout << end_dec << endl;
    }
    void _hash_and_map() {
        _Node *node_before=nullptr, *node_above=nullptr;
        for (const auto& row_it : _arr){
            for (const int& item : row_it){
                _lookup.emplace( item, _Node(item, node_above, nullptr, nullptr, node_before));
                if (node_before) {
                    node_before->right = &_lookup.at(item);
                }
                if (node_above) {
                    node_above->down = &_lookup.at(item);
                    node_above = node_above->right;
                }
                node_before = &_lookup.at(item);
                cout << '\0';
            }
            node_above = &_lookup.at(*(row_it.begin()));
            node_before = nullptr;
        }
        cout << '\0';
    }
    void _swap(const int& x, const direction& x_moves){
        _Node temp;
        if (x_moves < LEFT){
            if (_lookup[0].left)
                _lookup[0].left->right = &_lookup[x];
            if(_lookup[0].right)
                _lookup[0].right->left = &_lookup[x];
            if (_lookup[x].left)
                _lookup[x].left->right = &_lookup[0];
            if (_lookup[x].right)
                _lookup[x].right->left = &_lookup[0];
        }
        else{
            if(_lookup[0].up)
                _lookup[0].up->down = &_lookup[x];
            if(_lookup[0].down)
                _lookup[0].down->up = &_lookup[x];
            if(_lookup[x].up)
                _lookup[x].up->down = &_lookup[0];
            if(_lookup[x].down)
                _lookup[x].down->up = &_lookup[0];
        }

        temp.up = _lookup[0].up;
        temp.down = _lookup[0].down;
        temp.left = _lookup[0].left;
        temp.right = _lookup[0].right;

        _lookup[0].up = _lookup[x].up;
        _lookup[0].down = _lookup[x].down;
        _lookup[0].left = _lookup[x].left;
        _lookup[0].right = _lookup[x].right;

        _lookup[x].up = temp.up;
        _lookup[x].down = temp.down;
        _lookup[x].left = temp.left;
        _lookup[x].right = temp.right;

        switch (x_moves){
            case UP:
                _lookup[x].down = &_lookup[0];
                _lookup[0].up = &_lookup[x];
                break;
            case DOWN:
                _lookup[x].up = &_lookup[0];
                _lookup[0].down = &_lookup[x];
                break;
            case LEFT:
                _lookup[x].right = &_lookup[0];
                _lookup[0].left = &_lookup[x];
                break;
            case RIGHT:
                _lookup[x].left = &_lookup[0];
                _lookup[0].right = &_lookup[x];
                break;
        }
    }

    bool _swap_zero_with(const int& x) {
        direction x_moves;
        if (_lookup[0].up == &_lookup[x])
            x_moves = DOWN;
        else if (_lookup[0].right == &_lookup[x])
            x_moves = LEFT;
        else if (_lookup[0].down == &_lookup[x])
            x_moves = UP;
        else if (_lookup[0].left == &_lookup[x])
            x_moves = RIGHT;
        else
            return false;

        _swap(x, x_moves);
        _movelist.emplace_back(x);
        return true;
    }

public:
    explicit SlidingPuzzleSolver (const std::vector<std::vector<int>>& arr)
      : _arr(arr), _map_size(arr.size() * arr.size())
    {
      _hash_and_map();
      _p_entry_point = &_lookup[arr[0][0]];
    }

    std::vector<int> solve(){
        _print_board();

        assert(_swap_zero_with(8) && "X is not adjacent to 0.\n");
        _print_board();

        assert(_swap_zero_with(2)&& "X is not adjacent to 0.\n");
        _print_board();

        assert(_swap_zero_with(4)&& "X is not adjacent to 0.\n");
        _print_board();

        assert(_swap_zero_with(1)&& "X is not adjacent to 0.\n");
        _print_board();

        return _movelist;
    }
};







std::vector<int> slide_puzzle(const std::vector<std::vector<int>> &arr)
{
    SlidingPuzzleSolver sps = SlidingPuzzleSolver(arr);
    return sps.solve();
}


int main() {
    vector<vector<int>> board_3x3 = {{4,1,3},
                                    {2,8,0},
                                    {7,6,5}};
    auto movelist = slide_puzzle(board_3x3);
    for (auto move : movelist){
        cout << "Swapped 0 with " << move << endl;
    }
    std::cin.get();
    return 0;
}