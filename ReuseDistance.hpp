/**
 * @file
 * @author Michael Laurenzano <michaell@sdsc.edu>
 * @version 0.01
 *
 * @section LICENSE
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
 *
 * @section DESCRIPTION
 *
 * The ReuseDistanceHandler class allows for calculation and statistic tracking
 * for finding memory reuse distances given a stream of memory addresses and
 * ids.
 */

#include <assert.h>
#include <stdlib.h>
#include <tree234.h>

#include <algorithm>
#include <iostream>
#include <ostream>
#include <set>
#include <vector>

// unordered_map is a lot faster, use it where possible
#ifdef HAVE_UNORDERED_MAP
#include <unordered_map>
#define reuse_map_type std::unordered_map
#else
#include <map>
#define reuse_map_type std::map
#endif

#define TAB "\t"
#define ENDL "\n"

#define __seq id
int reusecmp (void* va, void* vb);

/**
 * @struct ReuseEntry
 *
 * ReuseEntry is used to pass memory addresses into a ReuseDistance.
 *
 * @field id  The unique id of the entity which generated the memory address.
 * Statistics are tracked seperately for each unique id.
 * @field address  A memory address.
 */
struct ReuseEntry {
    uint64_t id;
    uint64_t address;
};


class ReuseStats;

/**
 * @class ReuseDistance
 *
 * Tracks reuse distances for a memory address stream. Keep track of the addresses within
 * a specific window of history, whose size can be finite or infinite. For basic usage, see
 * the documentation at http://bit.ly/T3uXR5 for the constructors, the Process methods and
 * the Print methods. Also see the simple test file test/test.cpp included in the source package.
 */
class ReuseDistance {
private:

    // stores a list of bin boundaries
    std::vector<uint64_t> bins;
    tree234* window;
    reuse_map_type<uint64_t, uint64_t> mwindow;

    // store all stats
    // [id -> stats for this id]
    reuse_map_type<uint64_t, ReuseStats*> stats;

    // the window size
    uint64_t capacity;

    uint64_t current;
    uint64_t sequence;
    uint64_t maxbin;
    uint64_t binindividual;

    ReuseStats* GetStats(uint64_t id, bool gen);
    uint64_t GetBin(uint64_t id);
    void Init(uint64_t w, uint64_t b);

public:

    static const uint64_t DefaultBinIndividual = 32;
    static const uint64_t Infinity = 0;

    /**
     * Contructs a ReuseDistance object.
     *
     * @param w  The maximum window size, or alternatively the maximum possible reuse distance that this tool
     * will find. No window/distance limit is imposed if ReuseDistance::Infinity is used, though you could easily
     * run of of memory.
     * @param b  All distances not greater than b will be tracked individually. All distances are tracked individually
     * if b == ReuseDistance::Infinity. Beyond individual tracking, distances are tracked in bins whose boundaries
     * are the powers of two greater than b (and not exeeding w, of course).
     *
     */
    ReuseDistance(uint64_t w, uint64_t b);

    /**
     * Contructs a ReuseDistance object. Equivalent to calling the other constructor with 
     * b == ReuseDistance::DefaultBinIndividual
     *
     * @param w  The maximum window size, or alternatively the maximum possible reuse distance that this tool
     * will find. No window/distance limit if ReuseDistance::Infinity is used, though you could easily run out of
     * memory.
     */
    ReuseDistance(uint64_t w);

    /**
     * Destroys a ReuseDistance object.
     */
    ~ReuseDistance();

    /**
     * Print statistics for this ReuseDistance to an output stream.
     * The first line of the output is five tokens which are [1] the string literal
     * REUSESTATS, [2] the unique id, [3] the window size (0 == unlimited) [4] the
     * total number of accesses for that unique id and [5] the number of accesses from
     * that id which were not found within the active address window either because they
     * were evicted or because of cold misses. Each additional line of output contains
     * three tokens, which give [1] the minimum of a reuse distance range (inclusive),
     * [2] the maximum of a reuse distance range (inclusive) and [2] the number of times a
     * reusedistance in that range was observed.
     *
     * @param f  The output stream to print results to.
     *
     * @return none
     */
    void Print(std::ostream& f);

    /**
     * Print statistics for this ReuseDistance to std::cout.
     * See the other version of ReuseDistance::Print for information about output format.
     *
     * @return none
     */
    void Print();

    /**
     * Process a single memory address.
     *
     * @param addr  The structure describing the memory address to process.
     *
     * @return none
     */
    void Process(ReuseEntry& addr);

    /**
     * Process multiple memory addresses. Equivalent to calling Process on each element of the input array.
     *
     * @param addrs  An array of structures describing memory addresses to process.
     * @param count  The number of elements in addrs.
     *
     * @return none
     */
    void Process(ReuseEntry* addrs, uint64_t count);

    /**
     * Process multiple memory addresses. Equivalent to calling Process on each element of the input vector.
     *
     * @param addrs  A std::vector of memory addresses to process.
     *
     * @return none
     */
    void Process(std::vector<ReuseEntry> rs);

    /**
     * Process multiple memory addresses. Equivalent to calling Process on each element of the input vector.
     *
     * @param addrs  A std::vector of memory addresses to process.
     *
     * @return none
     */
    void Process(std::vector<ReuseEntry*> addrs);

    /**
     * Get a reuse distance for a ReuseEntry without updating any internal state.
     *
     * @param addr  The memory address to analyze.
     *
     * @return The reuse distance for the memory address given by addr.
     */
    uint64_t GetDistance(ReuseEntry& addr);

    /**
     * Get the ReuseStats object associated with some unique id.
     *
     * @param id  The unique id.
     *
     * @return The ReuseStats object associated with parameter id, or NULL if no ReuseStats is associate with id.
     */
    ReuseStats* GetStats(uint64_t id);

    /**
     * Get a std::vector containing all of the unique indices processed
     * by this ReuseDistance object.
     *
     * @param ids  A std::vector which will contain the ids. It is an error to
     * pass this vector non-empty (that is addrs.size() == 0 is enforced at runtime).
     *
     * @return none
     */
    void GetIndices(std::vector<uint64_t>& ids);

    /**
     * Get a std::vector containing all of the addresses currently in this ReuseDistance
     * object's active window.
     *
     * @param addrs  A std::vector which will contain the addresses. It is an error to
     * pass this vector non-empty (that is addrs.size() == 0 is enforced at runtime).
     *
     * @return none
     */
    void GetActiveAddresses(std::vector<uint64_t>& addrs);

};


/**
 * @class ReuseStats
 *
 * ReuseStats holds count of observed reuse distances.
 */
class ReuseStats {
private:
    reuse_map_type<uint64_t, uint64_t> distcounts;
    uint64_t accesses;
    uint64_t misscount;

public:

    /**
     * Contructs a ReuseStats object. Default constructor.
     */
    ReuseStats():accesses(0),misscount(0) {}

    /**
     * Destroys a ReuseStats object.
     */
    ~ReuseStats() {}

    /**
     * Increment the counter for some distance.
     *
     * @param dist  A reuse distance observed in the memory address stream.
     *
     * @return none
     */
    void Update(uint64_t dist);

    /**
     * Increment the number of misses. That is, addresses which were not found inside
     * the active address window. This is equivalent Update(0), but is faster.
     *
     * @return none
     */
    void Miss();

    /**
     * Get the number of misses. This is equal to the number of times Miss()
     * is called plus the number of times Update(0) is called. These two calls
     * are functionally equivalent but Miss() is faster.
     */
    uint64_t GetMissCount() { return misscount + distcounts[0]; }

    /**
     * Print a summary of the current reuse distances and counts.
     *
     * @param f  The stream to receive the output.
     * @param uniqueid  An identifier for this ReuseStats object.
     * @param binindividual  The maximum value for which bins are kept individually.
     * Helps print things prettily.
     *
     * @return none
     */
    void Print(std::ostream& f, uint64_t uniqueid, uint64_t binindividual);

    /**
     * Get a std::vector containing the distances observed, sorted in ascending order.
     *
     * @param dists  The vector which will hold the sorted distance values. It is an error
     * for dists to be passed in non-empty (that is, dists.size() == 0 is enforced).
     *
     * @return none
     */
    void GetSortedDistances(std::vector<uint64_t>& dists);

    /**
     * Get the maximum distance observed.
     *
     * @return The maximum distance observed.
     */
    uint64_t GetMaximumDistance();

    /**
     * Count the number of times some distance has been observed.
     *
     * @param dist  The distance to count.
     *
     * @return The number of times d has been observed.
     */
    uint64_t CountDistance(uint64_t dist);

    /**
     * Count the total number of distances observed.
     *
     * @return The total number of distances observed.
     */
    uint64_t GetAccessCount();
};

