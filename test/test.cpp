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
#define LARGE_TEST (3333333)
#define SEPERATOR "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"

int main(int argc, char* argv[]){

    ReuseEntry entry = ReuseEntry();
    ReuseDistance* r1, *r2, *r3;


    /** stride-1 test **/
    r1 = new ReuseDistance(ReuseDistance::Infinity);
    r2 = new ReuseDistance(SMALL_TEST*2);
    r3 = new ReuseDistance(SMALL_TEST/2);
    entry.id = 0;
    for (uint32_t i = 0; i < 10; i++){
        for (uint32_t j = 0; j < SMALL_TEST; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
        }
    }

    cout << "STRIDE-1 TEST" << ENDL;
    cout << SEPERATOR;
    r1->Print();
    cout << SEPERATOR;
    r2->Print();
    cout << SEPERATOR;
    r3->Print();
    cout << SEPERATOR;
    cout << SEPERATOR;

    delete r1;
    delete r2;
    delete r3;


    /** stride-4 test **/
    r1 = new ReuseDistance(ReuseDistance::Infinity);
    r2 = new ReuseDistance(SMALL_TEST*2);
    r3 = new ReuseDistance(SMALL_TEST/2);
    entry.id = 0;
    for (uint32_t i = 0; i < 10; i++){
        for (uint32_t j = 0; j < SMALL_TEST; j += 4){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
        }
    }

    cout << "STRIDE-4 TEST" << ENDL;
    cout << SEPERATOR;
    r1->Print();
    cout << SEPERATOR;
    r2->Print();
    cout << SEPERATOR;
    r3->Print();
    cout << SEPERATOR;

    delete r1;
    delete r2;
    delete r3;


    /** triangle-type pattern **/
    r1 = new ReuseDistance(ReuseDistance::Infinity);
    r2 = new ReuseDistance(SMALL_TEST*2);
    r3 = new ReuseDistance(SMALL_TEST/2);
    entry.id = 0;
    for (uint32_t i = 0; i < SMALL_TEST; i++){
        for (uint32_t j = 0; j < i; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
        }
    }

    cout << "TRIANGLE TEST" << ENDL;
    cout << SEPERATOR;
    r1->Print();
    cout << SEPERATOR;
    r2->Print();
    cout << SEPERATOR;
    r3->Print();
    cout << SEPERATOR;
    cout << SEPERATOR;

    delete r1;
    delete r2;
    delete r3;


    /** id differentiation test **/
    r1 = new ReuseDistance(ReuseDistance::Infinity);
    r2 = new ReuseDistance(SMALL_TEST*2);
    r3 = new ReuseDistance(SMALL_TEST/2);
    entry.id = 0;
    for (uint32_t i = 0; i < 10; i++){
        for (uint32_t j = 0; j < SMALL_TEST; j++){
            entry.address = j;
            entry.id = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
        }
    }

    cout << "IDDIFF TEST" << ENDL;
    cout << SEPERATOR;
    r1->Print();
    cout << SEPERATOR;
    r2->Print();
    cout << SEPERATOR;
    r3->Print();
    cout << SEPERATOR;
    cout << SEPERATOR;


    /** stride-1 medium timing test **/
    r1 = new ReuseDistance(ReuseDistance::Infinity);
    r2 = new ReuseDistance(MEDIUM_TEST*2);
    r3 = new ReuseDistance(MEDIUM_TEST/2);
    entry.id = 0;
    for (uint32_t i = 0; i < 10; i++){
        for (uint32_t j = 0; j < MEDIUM_TEST; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
        }
    }

    cout << "MEDIUM TIMING TEST" << ENDL;
    cout << SEPERATOR;
    r1->Print();
    cout << SEPERATOR;
    r2->Print();
    cout << SEPERATOR;
    r3->Print();
    cout << SEPERATOR;
    cout << SEPERATOR;

    delete r1;
    delete r2;
    delete r3;

    /** stride-1 large timing test **/
    r1 = new ReuseDistance(ReuseDistance::Infinity);
    r2 = new ReuseDistance(LARGE_TEST*2);
    r3 = new ReuseDistance(LARGE_TEST/2);
    entry.id = 0;
    for (uint32_t i = 0; i < 3; i++){
        for (uint32_t j = 0; j < LARGE_TEST; j++){
            entry.address = j;
            r1->Process(entry);
            r2->Process(entry);
            r3->Process(entry);
        }
    }

    cout << "LARGE TIMING TEST" << ENDL;
    cout << SEPERATOR;
    r1->Print();
    cout << SEPERATOR;
    r2->Print();
    cout << SEPERATOR;
    r3->Print();
    cout << SEPERATOR;
    cout << SEPERATOR;

    delete r1;
    delete r2;
    delete r3;

    return 0;
}


