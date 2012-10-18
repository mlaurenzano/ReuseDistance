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

//#define REUSE_DEBUG
#ifdef REUSE_DEBUG
#define debug_assert(...) assert(__VA_ARGS__)
#else
#define debug_assert(...)
#endif

#define __seq id
int reusecmp (void* va, void* vb) {
    ReuseEntry a = (ReuseEntry)(*((ReuseEntry*)va));
    ReuseEntry b = (ReuseEntry)(*((ReuseEntry*)vb));
    return (a.__seq - b.__seq);
}

void ReuseDistance::Init(uint64_t w, uint64_t b){
    capacity = w;
    binindividual = b;

    current = 0;
    sequence = 1;

    window = newtree234(reusecmp);
    assert(window);

    mwindow.clear();

    assert(ReuseDistance::Infinity == NULL && "NULL is non-zero!?");
}

ReuseDistance::ReuseDistance(uint64_t w, uint64_t b){
    Init(w, b);
}

ReuseDistance::ReuseDistance(uint64_t w){
    Init(w, DefaultBinIndividual);
}

ReuseDistance::~ReuseDistance(){
    for (reuse_map_type<uint64_t, ReuseStats*>::iterator it = stats.begin(); it != stats.end(); it++){
        uint64_t id = it->first;
        delete stats[id];
    }

    debug_assert(current == count234(window));
    while (current){
        delete (ReuseEntry*)delpos234(window, 0);
        current--;
    }
    freetree234(window);
}

void ReuseDistance::GetIndices(std::vector<uint64_t>& ids){
    assert(ids.size() == 0);
    for (reuse_map_type<uint64_t, ReuseStats*>::iterator it = stats.begin(); it != stats.end(); it++){
        uint64_t id = it->first;
        ids.push_back(id);
    }
}

void ReuseDistance::GetActiveAddresses(std::vector<uint64_t>& addrs){
    assert(addrs.size() == 0);
    debug_assert(current == count234(window));

    for (int i = 0; i < current; i++){
        ReuseEntry* r = (ReuseEntry*)index234(window, i);
        addrs.push_back(r->address);
    }
}

void ReuseDistance::Print(){
    Print(cout);
}

void ReuseDistance::Print(ostream& f){
    vector<uint64_t> keys;
    for (reuse_map_type<uint64_t, ReuseStats*>::iterator it = stats.begin(); it != stats.end(); it++){
        keys.push_back(it->first);
    }
    sort(keys.begin(), keys.end());

    for (vector<uint64_t>::const_iterator it = keys.begin(); it != keys.end(); it++){
        uint64_t id = (*it);
        ReuseStats* r = stats[id];

        f << "REUSESTATS"
          << TAB << dec << id
          << TAB << dec << capacity
          << TAB << r->GetAccessCount()
          << TAB << r->GetMissCount()
          << endl;

        r->Print(f, id, binindividual);
    }
}

void ReuseDistance::Process(ReuseEntry* rs, uint64_t count){
    for (uint32_t i = 0; i < count; i++){
        Process(rs[i]);
    }
}

void ReuseDistance::Process(vector<ReuseEntry> rs){
    for (vector<ReuseEntry>::iterator it = rs.begin(); it != rs.end(); it++){
        ReuseEntry r = *it;
        Process(r);
    }
}

void ReuseDistance::Process(vector<ReuseEntry*> rs){
    for (vector<ReuseEntry*>::iterator it = rs.begin(); it != rs.end(); it++){
        ReuseEntry* r = *it;
        Process((*r));
    }
}

uint64_t ReuseDistance::GetDistance(ReuseEntry& r){
    uint64_t addr = r.address;
    uint64_t mres = mwindow[addr];

    if (mres){
        int dist = 0;
        ReuseEntry key;
        key.__seq = sequence;

        void* result = findrelpos234(window, (void*)(&key), reusecmp, REL234_EQ, &dist);
        debug_assert(result);
        return current - dist;
    } else {
        return ReuseDistance::Infinity;
    }

}


// this should be fast as possible. This code is from http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
static const uint64_t b[] = {0x2L, 0xCL, 0xF0L, 0xFF00L, 0xFFFF0000L, 0xFFFFFFFF00000000L};
static const uint32_t S[] = {1, 2, 4, 8, 16, 32};
inline uint64_t ShaveBitsPwr2(uint64_t val){
    val -= 1;
    register uint64_t r = 0; // result of log2(v) will go here
    for (int32_t i = 5; i >= 0; i--){
        if (val & b[i]){
            val = val >> S[i];
            r |= S[i];
        }
    }
    return (2 << r);
}

uint64_t ReuseDistance::GetBin(uint64_t id){
    if (binindividual && id > binindividual){
        return ShaveBitsPwr2(id);
    }
    return id;
}

void ReuseDistance::Process(ReuseEntry& r){
    uint64_t addr = r.address;
    uint64_t id = r.id;
    uint64_t mres = mwindow.count(addr);

    ReuseStats* stats = GetStats(id, true);

    int dist = 0;
    void* result;
    if (mres){
        mres = mwindow[addr];
        ReuseEntry key;
        key.address = addr;
        key.__seq = mres;

        result = findrelpos234(window, (void*)(&key), reusecmp, REL234_EQ, &dist);
        debug_assert(result);

        if (capacity != ReuseDistance::Infinity){
            debug_assert(current - dist <= capacity);
        }
        stats->Update(GetBin(current - dist));
    } else {
        stats->Update(ReuseDistance::Infinity);
    }

    // recycle a slot when possible
    ReuseEntry* slot = NULL;
    if (mres || (capacity != ReuseDistance::Infinity && current >= capacity)){
        slot = (ReuseEntry*)delpos234(window, dist);
        debug_assert(mwindow[slot->address]);
        mwindow.erase(slot->address);
        debug_assert(count234(window) == mwindow.size());
    } else {
        slot = new ReuseEntry();
        current++;
    }
    
    mwindow[addr] = sequence;

    slot->__seq = sequence;
    slot->address = addr;
    add234(window, slot);

    debug_assert(count234(window) == mwindow.size());
    debug_assert(mwindow.size() <= current);

    sequence++;
}

ReuseStats* ReuseDistance::GetStats(uint64_t id, bool gen){
    ReuseStats* s = stats[id];
    if (s == NULL && gen){
        s = new ReuseStats();
        stats[id] = s;
    }
    return s;
}

ReuseStats* ReuseDistance::GetStats(uint64_t id){
    return GetStats(id, false);
}

uint64_t ReuseStats::GetAccessCount(){
    return accesses;
}

uint64_t ReuseStats::GetMaximumDistance(){
    uint64_t max = 0;
    for (reuse_map_type<uint64_t, uint64_t>::iterator it = distcounts.begin(); it != distcounts.end(); it++){
        uint64_t d = it->first;
        if (d > max){
            max = d;
        }
    }
    return max;
}

void ReuseStats::Update(uint64_t dist){
    distcounts[dist] += 1;
    accesses++;
}

void ReuseStats::Miss(){
    misscount++;
    accesses++;
}

uint64_t ReuseStats::CountDistance(uint64_t d){
    if (distcounts.count(d) == 0){
        return 0;
    }
    return distcounts[d];
}

void ReuseStats::GetSortedDistances(vector<uint64_t>& dkeys){
    assert(dkeys.size() == 0 && "dkeys must be an empty vector");
    for (reuse_map_type<uint64_t, uint64_t>::iterator it = distcounts.begin(); it != distcounts.end(); it++){
        uint64_t d = it->first;
        dkeys.push_back(d);
    }
    sort(dkeys.begin(), dkeys.end());    
}

void ReuseStats::Print(ostream& f, uint64_t uniqueid, uint64_t binindividual){
    vector<uint64_t> keys;
    GetSortedDistances(keys);

    for (vector<uint64_t>::iterator it = keys.begin(); it != keys.end(); it++){
        uint64_t d = *it;
        if (d == 0) continue;

        debug_assert(distcounts.count(d) > 0);
        uint32_t cnt = distcounts[d];

        debug_assert(cnt > 0);
        if (cnt > 0){
            uint64_t p = d / 2 + 1;
            if (binindividual == ReuseDistance::Infinity || d <= binindividual){
                p = d;
            }
            f << TAB << dec << p
              << TAB << d
              << TAB << cnt
              << ENDL;
        }
    }
}

