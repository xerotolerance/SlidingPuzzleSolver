#include <iostream>
#include <vector>
#include <algorithm>

using std::vector;
using std::cout;
using std::endl;

class SlidingPuzzleSolver{
private:
    struct _Node{
        _Node *up=nullptr,*right=nullptr,*down=nullptr,*left=nullptr;
        int val=-1;
        friend std::ostream& operator<<(std::ostream& os, const _Node& node){
            return os << node.val;
        }
    };
private:
    const std::vector<std::vector<int>>& _arr;
    _Node* _lookup;
    uint _map_size;
private:
    void _hash_and_map(){
        _Node *node_before=nullptr, *node_above=nullptr;
        for (const auto& row_it : _arr){
            for (const int& item : row_it){
                _lookup[item].up = node_above;
                _lookup[item].left = node_before;
                _lookup[item].val = item;

                if (node_before) {
                    node_before->right = &_lookup[item];
                }
                if (node_above) {
                    node_above->down = &_lookup[item];
                    node_above = node_above->right;
                }

                node_before = &_lookup[item];
                cout << item << " ";
            }
            cout << endl;
            node_above = &_lookup[*(row_it.begin())];
            node_before = nullptr;
        }

        node_before = nullptr;
        node_above = nullptr;
        delete(node_before);
        delete(node_above);
    }
    void _swap(){

    }

    bool _swap_zero_with(int x){
        bool was_successful = true;
        if (_lookup[0].up == &_lookup[x])
            _swap();
        else if (_lookup[0].right == &_lookup[x])
            _swap();
        else if (_lookup[0].down == &_lookup[x])
            _swap();
        else if (_lookup[0].left == &_lookup[x])
            _swap();
        else was_successful = false;
        return was_successful;
    }
public:
    SlidingPuzzleSolver (const std::vector<std::vector<int>>& arr)
      : _arr(arr), _map_size(arr.size() * arr.size())
    {
      _lookup = &_lookup[_map_size];
      _hash_and_map();
    }
  
};







std::vector<int> slide_puzzle(const std::vector<std::vector<int>> &arr)
{
    SlidingPuzzleSolver sps(arr);
    return std::vector<int>();
}


int main() {
    vector<vector<int>> board_3x3 = {{4,1,3},
                                    {2,8,0},
                                    {7,6,5}};
    for (auto move : slide_puzzle(board_3x3)){
        cout << "Swapped 0 with " << move << endl;
    }
    std::cin.get();
    return 0;
}