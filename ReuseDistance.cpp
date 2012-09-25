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

ReuseDistance::ReuseDistance(){
    windowsize = 0;
    lastcleanup = 0;
    sequence = 1;

    SetCleanFrequency(windowsize > MinimumCleanFrequency ? windowsize : MinimumCleanFrequency);
}

ReuseDistance::ReuseDistance(uint64_t w){
    windowsize = w;
    lastcleanup = 0;
    sequence = 1;

    SetCleanFrequency(windowsize > MinimumCleanFrequency ? windowsize : MinimumCleanFrequency);
}

ReuseDistance::ReuseDistance(ReuseDistance& h){
    windowsize = h.GetWindowSize();
    lastcleanup = 0;
    sequence = h.GetCurrentSequence();
    cleanfreq = h.GetCleanFrequency();

    vector<uint64_t> util;
    h.GetIndices(util);
    for (vector<uint64_t>::iterator it = util.begin(); it != util.end(); it++){
        uint64_t id = *it;
        ReuseStats* r = h.GetStats(id);

        ReuseStats* rcopy = new ReuseStats(*r);        
        stats[id] = rcopy;
    }
    util.clear();

    h.GetActiveAddresses(util);
    for (vector<uint64_t>::iterator it = util.begin(); it != util.end(); it++){
        uint64_t a = *it;
        uint64_t s = h.GetSequenceValue(a);

        window[a] = s;
    }
}

void ReuseDistance::SetCleanFrequency(uint64_t c){
    cleanfreq = c;
    Cleanup();
}

ReuseDistance::~ReuseDistance(){
    for (reuse_map_type<uint64_t, ReuseStats*>::iterator it = stats.begin(); it != stats.end(); it++){
        uint64_t id = it->first;
        delete stats[id];
    }
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
    for (reuse_map_type<uint64_t, uint64_t>::iterator it = window.begin(); it != window.end(); it++){
        uint64_t addr = it->first;
        addrs.push_back(addr);
    }    
}

uint64_t ReuseDistance::GetSequenceValue(uint64_t a){
    if (window.count(a) == 0){
        return 0;
    }
    uint64_t s = window[a];
    return s;
}

void ReuseDistance::Print(){
    this->Print(cout);
}

void ReuseDistance::Print(ostream& f){
    for (reuse_map_type<uint64_t, ReuseStats*>::iterator it = stats.begin(); it != stats.end(); it++){
        uint64_t id = it->first;
        ReuseStats* r = it->second;

        f << "REUSESTATS" << TAB << dec << id
          << TAB << windowsize
          << TAB << r->GetAccessCount()
          << TAB << r->CountDistance(0)
          << endl;
        r->Print(f, id);
    }
}

void ReuseDistance::Cleanup(){
    if (windowsize == 0){
        return;
    }
    if (sequence - lastcleanup < cleanfreq){
        return;
    }

    //cout << "Cleaning! " << sequence << ENDL;

    set<uint64_t> erase;
    for (reuse_map_type<uint64_t, uint64_t>::iterator it = window.begin(); it != window.end(); it++){
        uint64_t addr = (*it).first;
        uint64_t seq = (*it).second;

        if (sequence - seq >= windowsize){
            erase.insert(addr);
        }
    }

    for (set<uint64_t>::iterator it = erase.begin(); it != erase.end(); it++){
        window.erase((*it));
    }

    lastcleanup = sequence;
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

void ReuseDistance::Process(ReuseEntry& r){
    uint64_t addr = r.address;
    ReuseStats* s = GetStats(r.id);

    Cleanup();

    uint64_t a = window[addr];
    if (a == 0){
        s->Update(0);
    } else {
        uint64_t d = sequence - a;
        if (windowsize && d >= windowsize){
            s->Update(0);
        } else {
            s->Update(d);
        }
    }

    window[addr] = sequence++;
}

ReuseStats* ReuseDistance::GetStats(uint64_t id){
    ReuseStats* s = stats[id];
    if (s == NULL){
        s = new ReuseStats();
        stats[id] = s;
    }
    //assert(s != NULL);
    return s;
}

ReuseStats::ReuseStats(ReuseStats& r){
    vector<uint64_t> dists;
    r.GetSortedDistances(dists);

    for (vector<uint64_t>::iterator it = dists.begin(); it != dists.end(); it++){
        uint64_t d = *it;
        distcounts[d] = r.CountDistance(d);
    }

    accesses = r.GetAccessCount();
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
    if (distcounts.count(dist) == 0){
        distcounts[dist] = 0;
    }
    distcounts[dist] = distcounts[dist] + 1;
    accesses++;
}

uint64_t ReuseStats::CountDistance(uint64_t d){
    if (distcounts.count(d) == 0){
        return 0;
    }
    return distcounts[d];
}

uint64_t ReuseStats::CountDistance(uint64_t l, uint64_t h){
    uint64_t t = 0;
    for (reuse_map_type<uint64_t, uint64_t>::iterator it = distcounts.begin(); it != distcounts.end(); it++){
        uint64_t d = it->first;
        if (d >= l && d < h){
            t += distcounts[d];
        }
    }
    return t;
}

void ReuseStats::GetSortedDistances(vector<uint64_t>& dkeys){
    assert(dkeys.size() == 0 && "dkeys must be an empty vector");
    for (reuse_map_type<uint64_t, uint64_t>::iterator it = distcounts.begin(); it != distcounts.end(); it++){
        uint64_t d = it->first;
        dkeys.push_back(d);
    }
    sort(dkeys.begin(), dkeys.end());    
}

void ReuseStats::Print(ostream& f, uint64_t uniqueid){
    vector<uint64_t> keys;
    GetSortedDistances(keys);

    for (vector<uint64_t>::iterator it = keys.begin(); it != keys.end(); it++){
        uint64_t d = *it;
        if (d == 0) continue;

        assert(distcounts.count(d) > 0);
        uint32_t cnt = distcounts[d];

        assert(cnt > 0);
        if (cnt > 0){
            f << TAB << dec << d
              << TAB << cnt
              << ENDL;
        }
    }

}
