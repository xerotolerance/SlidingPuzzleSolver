#include <iostream>
#include <vector>
#include <map>
#include <cassert>
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
    class _GridNode{
        private:
            friend SlidingPuzzleSolver;
            friend std::ostream& operator<<(std::ostream& os, const _GridNode& node){
                return os << node.val;
            };
            _GridNode *up, *right, *down, *left;
            int val;
        public:
            explicit _GridNode(int val=-1, _GridNode* up= nullptr, _GridNode* right= nullptr, _GridNode* down= nullptr, _GridNode* left= nullptr)
                : val(val), up(up), right(right), down(down), left(left) {};

            _GridNode (const _GridNode& orig): val(orig.val), up(orig.up), right(orig.right), down(orig.down), left(orig.left) {
                cout << orig.val << " was copied!" << endl;
            };

            _GridNode(_GridNode&& src) noexcept
                : val(std::exchange(src.val, -1)),
                  up(std::exchange(src.up, nullptr)), right(std::exchange(src.right, nullptr)),
                  down(std::exchange(src.down, nullptr)), left(std::exchange(src.left, nullptr)){
                cout << this->val << " was moved!" << endl;
            }

    };
    enum direction{UP=0, DOWN, LEFT, RIGHT};
private:
    const std::vector<std::vector<int>>& _arr;
    std::map<int, _GridNode> _lookup;
    std::vector<int> _movelist;
    _GridNode *_p_entry_point;
    const uint _map_size;
private:
    void _print_board(const std::string& end_dec = "=============") const{
        _GridNode* row_head = _p_entry_point, *item=nullptr;
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
    void _create_map() {
        _GridNode *node_before=nullptr, *node_above=nullptr;
        for (const auto& row_it : _arr){
            for (const int& item : row_it){
                _lookup.emplace( item, std::move(_GridNode(item, node_above, nullptr, nullptr, node_before)));
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
    void _swap(const int& x){
        //swap nodes' value fields
        _lookup.at(0).val = x;
        _lookup.at(x).val = 0;

        //temporarily extract nodes & swap their placements in map
        auto temp_x = _lookup.extract(x), temp_0 = _lookup.extract(0);
        temp_0.key() = x;
        temp_x.key() = 0;

        //use iterators to make node re-insertion constant time
        auto insert_before = ++_lookup.begin();
        _lookup.insert(insert_before, std::move(temp_0)); //O(1) when told where to insert before
        std::advance(insert_before, x);     // O(1) operation
        _lookup.insert(insert_before, std::move(temp_x));

    }

    bool _swap_zero_with(const int& x) {
        _swap(x);
        if (_p_entry_point == &_lookup.at(x))
            _p_entry_point = &_lookup[0];
        else if (_p_entry_point == &_lookup.at(0))
            _p_entry_point = &_lookup[x];
        _movelist.emplace_back(x);

        return true;
    }

public:
    explicit SlidingPuzzleSolver (const std::vector<std::vector<int>>& arr)
      : _arr(arr), _map_size(arr.size() * arr.size())
    {
      _create_map();
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