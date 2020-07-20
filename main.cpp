#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <iterator>
#include <unordered_set>

using std::unordered_set;
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
            int val;
        public:
            explicit GridNode(int val=-1, GridNode* up= nullptr, GridNode* right= nullptr, GridNode* down= nullptr, GridNode* left= nullptr)
                : val(val), up(up), right(right), down(down), left(left) {};

            GridNode (const GridNode& orig): val(orig.val), up(orig.up), right(orig.right), down(orig.down), left(orig.left) {
                //cout << orig.val << " was copied!" << endl;
            };

            GridNode(GridNode&& src) noexcept
                : val(std::exchange(src.val, -1)),
                  up(std::exchange(src.up, nullptr)), right(std::exchange(src.right, nullptr)),
                  down(std::exchange(src.down, nullptr)), left(std::exchange(src.left, nullptr)){
                //cout << this->val << " was moved!" << endl;
            }

    };
    enum direction{UP=0, DOWN, LEFT, RIGHT};
private:
    const std::vector<std::vector<int>>& _arr;
    std::unordered_map<int, GridNode> _lookup;
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
        GridNode *node_before=nullptr, *node_above=nullptr;
        for (const auto& row_it : _arr){
            for (const int& item : row_it){
                _lookup.emplace( item, std::move(GridNode(item, node_above, nullptr, nullptr, node_before)));
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

        //put 'em back w/o copying them
        _lookup.insert(std::move(temp_0));
        _lookup.insert(std::move(temp_x));

    }

    bool _swap_zero_with(const int& x) {
        const auto& chk = _lookup.at(0);
        if ((chk.up && chk.up->val==x) || (chk.right && chk.right->val==x) \
            || (chk.down && chk.down->val==x) || (chk.left && chk.left->val==x))
            _swap(x);
        if (chk.val)
            _movelist.emplace_back(x);
        return chk.val;
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
      /*  if (!_swap_zero_with(8))
            cout << "X is not adjacent to 0.\n";
        _print_board();

        if(!_swap_zero_with(2))
            cout << "X is not adjacent to 0.\n";
        _print_board();

        if(!_swap_zero_with(4))
            cout << "X is not adjacent to 0.\n";
        _print_board();

        if (!_swap_zero_with(5))
            cout << "X is not adjacent to 0.\n";
        _print_board();*/

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

    int counter = 0, rand_position, rand_int;
    while (!validrange.empty()){
        rand_position = std::rand() % validrange.size();
        rand_int = validrange[rand_position];
        board[counter % length_of_square].emplace_back(rand_int);
        auto it = validrange.begin();
        std::advance(it, rand_position);
        validrange.erase(it);
        ++counter;
    }

    return board;
}

int main() {
    std::srand(std::time(nullptr)); //"ok, psuedo-random," but its fine for this..."
    auto board1 = create_random_board();
    auto movelist1 = slide_puzzle(board1);

    auto board2 = create_random_board();
    auto movelist2 = slide_puzzle(board2);

    auto board3 = create_random_board();
    auto movelist3 = slide_puzzle(board3);

    /*){
        cout << "Swapped 0 with " << move << endl;
    }*/
    std::cin.get();
    return 0;
}