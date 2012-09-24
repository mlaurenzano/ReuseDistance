#include <ReuseDistance.hpp>
#include <stdlib.h>

#define TEST_WINDOW_SIZE (100)

int main(int argc, char* argv[]){
    ReuseDistance* r = new ReuseDistance(0);
    ReuseEntry entry = ReuseEntry();
    entry.id = 0;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE; j++){
            entry.address = j;
            r->Process(entry);
        }
    }

    r->Print();

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j++){
            entry.address = j;
            r->Process(entry);
        }
    }

    r->Print();

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j += 2){
            entry.address = j;
            r->Process(entry);
        }
    }

    r->Print();

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j += 4){
            entry.address = j;
            r->Process(entry);
        }
    }

    r->Print();

    return 0;
}


