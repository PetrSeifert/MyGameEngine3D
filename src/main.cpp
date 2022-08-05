//std
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sorting_algorithms/app.hpp>

int main(){
    App app{};

    try{
        app.run();
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}