#include <sorting_algorithms/sorting_algorithms.hpp>

BaseSort::BaseSort(int listSize) : listSize{listSize} {RandomFillList();}

void BaseSort::RandomFillList(){
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

void SelectionSort::sortStep(){
    if (loopIndex == listSize - 1){
        sorted = true;
        return;
    }

    if (currentIndex == loopIndex) {
        minIndex = loopIndex;
        minValueIndex = loopIndex;
        currentIndex++;
    }

    if (v.at(minValueIndex) > v.at(currentIndex)){
        minValueIndex = currentIndex;
    }

    if (currentIndex == listSize - 1){
        int copy = v.at(minIndex);
        v.at(minIndex) = v.at(minValueIndex);
        v.at(minValueIndex) = copy;
        loopIndex++;
        currentIndex = loopIndex;
        return;
    }
    currentIndex++;
}

void InsertionSort::sortStep(){
    if (currentIndex == listSize){
        sorted = true;
        return;
    }

    if (backtrackIndex == 0){
        currentIndex++;
        backtrackIndex = currentIndex;
    }

    if (v.at(backtrackIndex) > v.at(backtrackIndex - 1)){
        currentIndex++;
        backtrackIndex = currentIndex;
    }
    else{
        int copy = v.at(backtrackIndex);
        v.at(backtrackIndex) = v.at(backtrackIndex - 1);
        v.at(backtrackIndex - 1) = copy;
        backtrackIndex--;
    } 
}

//Unfinished
void QuickSort::sortStep(){

    if (quickSortLow >= quickSortHigh){ 
        sorted = true;
        return;
    }

    if (!partitionDone){
        if (partitionI < partitionJ){
            while (v.at(pivotIndex) >= v.at(partitionI))
                partitionI++;
            while (v.at(pivotIndex) < v.at(partitionJ))
                partitionJ++;
            if (partitionI < partitionJ){
                int copy = v.at(partitionI);
                v.at(partitionI) = v.at(partitionJ);
                v.at(partitionJ) = copy;
                return;
            }
        }
        int copy = v.at(pivotIndex);
        v.at(pivotIndex) = v.at(partitionJ);
        v.at(partitionJ) = copy;
        pivotIndex = partitionJ;
        partitionDone = true;
        return;
    }

    partitionDone = false;
}