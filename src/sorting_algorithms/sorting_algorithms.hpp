#include <random>
#include <vector>
#include <algorithm>

class BaseSort {
    public:
    void sortStep();
    bool sorted = false;
    int listSize;
    std::vector<int> v;
    int currentIndex = 0;
    BaseSort(int listSize = 100);

    protected:
    void RandomFillList();
};

class BubbleSort : public BaseSort {
    public:
    using BaseSort::BaseSort;
    virtual void sortStep();

    private:
    bool swapped = false;
};

class SelectionSort : public BaseSort {

    public:
    using BaseSort::BaseSort;
    int loopIndex = 0;
    int minValueIndex = 0;
    virtual void sortStep();

    private:
    bool swapped = false;
    int minIndex = 0;
};

class InsertionSort : public BaseSort {
    public:
    using BaseSort::BaseSort;
    virtual void sortStep();
    int currentIndex = 1;

    private:
    int backtrackIndex = 1;
};