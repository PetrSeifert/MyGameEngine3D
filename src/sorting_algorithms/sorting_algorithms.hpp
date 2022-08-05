#include <random>
#include <vector>
#include <algorithm>

class BubbleSort {

    public:
    void sortStep();
    bool sorted = false;
    int listSize;
    std::vector<int> v;
    int currentIndex = 0;
    BubbleSort(int listSize = 100);

    private:
    bool swapped = false;

    void RandomFillList();
};