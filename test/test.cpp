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

#define SMALL_TEST (100)
#define MEDIUM_TEST (444444)
#define LARGE_TEST (6666666)
#define SEPERATOR "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"

int main(int argc, char* argv[]){
    ReuseDistance* r1 = new ReuseDistance(ReuseDistance::Infinity);
    ReuseDistance* r2 = new ReuseDistance(SMALL_TEST/2);
    ReuseEntry entry = ReuseEntry();
    entry.id = 0;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < SMALL_TEST; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < SMALL_TEST - 1; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < SMALL_TEST - 1; j += 2){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
        }
    }

    r1->Print();
    r2->Print();
    cout << SEPERATOR;

    ReuseDistance* r3 = new ReuseDistance(ReuseDistance::Infinity);
    r1->Print();
    r3->Print();
    cout << SEPERATOR;

    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < SMALL_TEST - 1; j += 4){
            entry.address = j;
            entry.id = 0;
            r1->Process(entry);
            entry.id = j;
            r3->Process(entry);
        }
    }
    r1->Print();
    r3->Print();
    cout << SEPERATOR;

    ReuseDistance* r4 = new ReuseDistance(SMALL_TEST);
    entry.id = 0;
    for (uint32_t i = 0; i < MEDIUM_TEST; i++){
        for (uint32_t j = 0; j < SMALL_TEST - 1; j++){
            entry.address = j;
            r4->Process(entry);
        }
    }

    r4->Print();
    cout << SEPERATOR;

    ReuseDistance* r5 = new ReuseDistance(SMALL_TEST);
    ReuseDistance* r6 = new ReuseDistance(ReuseDistance::Infinity);
    entry.id = 0;
    for (uint32_t i = 0; i < 2; i++){
        for (uint32_t j = 0; j < LARGE_TEST; j++){
            entry.address = j;
            r5->Process(entry);
            r6->Process(entry);
        }
    }
    r5->Print();
    r6->Print();
    cout << SEPERATOR;

    delete r1;
    delete r2;
    delete r3;
    delete r4;
    delete r5;
    delete r6;

    return 0;
}


