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
    ReuseDistance* r1 = new ReuseDistance(0);
    ReuseDistance* r2 = new ReuseDistance(10);
    ReuseDistance* r3 = new ReuseDistance(100);
    ReuseDistance* r4 = new ReuseDistance(1000);
    ReuseEntry entry = ReuseEntry();
    entry.id = 0;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
            r4->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    r3->Print();
    r4->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
            r4->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    r3->Print();
    r4->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < TEST_WINDOW_SIZE - 1; j += 2){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
            r4->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    r3->Print();
    r4->Print();
    cout << SEPERATOR;

    ReuseDistance* r5 = new ReuseDistance(*r1);
    r1->Print();
    // TODO: r5 != r1
    r5->Print();
    cout << SEPERATOR;

    return 0;
}


