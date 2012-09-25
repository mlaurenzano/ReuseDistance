/* 
 * This file is part of the ReuseDistance tool.
 * 
 * Copyright (c) 2012, University of California Regents
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ReuseDistance.hpp>

using namespace std;

#define TEST_WINDOW_SIZE (100)
#define SEPERATOR "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"

int main(int argc, char* argv[]){
    ReuseDistance* r1 = new ReuseDistance();
    ReuseDistance* r2 = new ReuseDistance(10);
    ReuseEntry entry = ReuseEntry();
    entry.id = 0;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j += 2){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    cout << SEPERATOR;

    ReuseDistance* r3 = new ReuseDistance(*r1);
    r1->Print();
    r3->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j += 4){
            entry.address = j;
            r1->Process(entry);
            r3->Process(entry);
        }
    }
    r1->Print();
    r3->Print();
    cout << SEPERATOR;

    ReuseDistance* r4 = new ReuseDistance(TEST_WINDOW_SIZE);
    for (uint32_t i = 0; i < 444444; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j++){
            entry.address = j;
            r4->Process(entry);
        }
    }

    r4->Print();
    cout << SEPERATOR;

    ReuseDistance* r5 = new ReuseDistance();
    for (uint32_t i = 0; i < 444444; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j++){
            entry.address = j;
            entry.id = j;
            r5->Process(entry);
        }
    }

    r5->Print();
    cout << SEPERATOR;

    ReuseDistance* r6 = new ReuseDistance();
    for (uint32_t i = 0; i < 2; i++){
        for (uint32_t j = 0; j < 22222222; j++){
            entry.address = j;
            r6->Process(entry);
        }
    }
    
    r6->Print();
    cout << SEPERATOR;

    return 0;
}


