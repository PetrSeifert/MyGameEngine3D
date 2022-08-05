#include <sorting_algorithms/sorting_algorithms.hpp>

BubbleSort::BubbleSort(int listSize) : listSize{listSize} {RandomFillList();}

void BubbleSort::RandomFillList(){
    static std::random_device rd;  
    static std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib1(1, listSize);
    for (int i = 0; i < listSize; i++)
    {
        int number;
        do {
            number = distrib1(gen);
        } while (std::find(v.begin(), v.end(), number) != v.end());
        v.push_back(number);
    }
}

void BubbleSort::sortStep(){
    if (currentIndex == listSize - 1){
        if (!swapped)
        {
            sorted = true;
            return;
        }
        currentIndex = 0;
        swapped = false;
        return;
    }
    
    if (v.at(currentIndex) > v.at(currentIndex + 1)){
        int copy = v.at(currentIndex);
        v.at(currentIndex) = v.at(currentIndex + 1);
        v.at(currentIndex + 1) = copy;
        swapped = true;
    }
    currentIndex++;
}