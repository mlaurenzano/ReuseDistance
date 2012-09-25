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

/**
 * @class ReuseStats
 *
 * ReuseStats holds a count of the number of times each reuse distance is observed.
 */
class ReuseStats {
private:
    reuse_map_type<uint64_t, uint64_t> distcounts;
    uint64_t accesses;

public:

    /**
     * Contructs a ReuseStats object. Default constructor.
     */
    ReuseStats():accesses(0) {}

    /**
     * Contructs a ReuseStats object. Copy constructor. Performs a deep copy.
     *
     * @param r  A ReuseStats object from which to copy all state.
     */
    ReuseStats(ReuseStats& r);

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
     * Print a summary of the current reuse distances and counts.
     *
     * @param f  The stream to receive the output.
     * @param uniqueid  An identifier for this ReuseStats object.
     * @param scale  A vector holding the boundaries of bins used
     * to aggregate the reuse distances.
     *
     * @return none
     */
    void Print(std::ostream& f, uint64_t uniqueid);

    /**
     * Get a std::vector containing the distances observed, sorted in ascending order.
     * The first line of the output is five tokens which are [1] the string literal
     * REUSESTATS, [2] the unique id, [3] the window size (0 == unlimited) [4] the
     * total number of accesses for that unique id and [5] the number of accesses from
     * that id which were not found within the active address window either because they
     * were evicted or because of cold misses. Each additional line of output contains
     * two tokens, which give [1] a reuse distance and [2] the number of times that reuse
     * distance was observed.
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
     * Count the number of times any distance within some range [low, high) has been observed.
     *
     * @param low  The lower bound (inclusive) of the distance range to count.
     * @param high  The upper bound (exclusive) of the distance range to count.
     *
     * @return The number of times any distance within the range [low, high) has been observed.
     */
    uint64_t CountDistance(uint64_t low, uint64_t high);

    /**
     * Count the total number of distances observed.
     *
     * @return The total number of distances observed.
     */
    uint64_t GetAccessCount();
};


/**
 * @class ReuseDistance
 *
 * Tracks reuse distances for a memory address stream. Keep track of the addresses within
 * a specific window of history, whose size can be finite or infinite. See constructor
 * documentation for more details.
 */
class ReuseDistance {
private:

    // stores all addresses seen since the last cleanup
    // [address -> the sequence id of when we last saw this address]
    reuse_map_type<uint64_t, uint64_t> window;

    // store all stats
    // [id -> stats for this id]
    reuse_map_type<uint64_t, ReuseStats*> stats;

    // a sequence to enumerate memory accesses
    uint64_t sequence;

    // at which memory access did we last clean up?
    uint64_t lastcleanup;
    uint64_t cleanfreq;

    uint64_t windowsize;

public:

    /**
     * Minimum value for the cleanup frequency. The cleanup frequency is set by the constructor
     * to the maximum of this value and the window size.
     */
    //static const uint64_t MinimumCleanFrequency = 1000000;

    /**
     * Contructs a ReuseDistance object. Default constructor. Uses an unlimited-size window.
     *
     */
    ReuseDistance();

    /**
     * Contructs a ReuseDistance object.
     *
     * @param w  The maximum size of the window of addresses that will be examined. 
     * Use 0 for no window, but be aware that this will use a potetially unlimited 
     * amount of memory that will be proportional to the number of unique addresses
     * processed by this object.
     */
    ReuseDistance(uint64_t w);

    /**
     * Contructs a ReuseDistance object. Copy constructor. Performs a deep copy.
     *
     * @param h  A reference to another ReuseDistance object. All state from this parameter is copied
     * to the new ReuseDistance object, including window size, current addresses in that window and all tracked
     * statistics.
     */
    ReuseDistance(ReuseDistance& h);

    /**
     * Destroys a ReuseDistance object.
     */
    ~ReuseDistance();

    /**
     * Print statistics for this ReuseDistance to std::cout.
     * See ReuseStats::Print for information about output format.
     *
     * @return none
     */
    void Print();

    /**
     * Print statistics for this ReuseDistance to an output stream.
     * See ReuseStats::Print for information about output format.
     *
     * @param f  The output stream to print results to.
     *
     * @return none
     */
    void Print(std::ostream& f);

    /**
     * Process a single memory address.
     *
     * @param addr  The structure describing the memory address to process.
     *
     * @return none
     */
    void Process(ReuseEntry& addr);

    /**
     * Process multiple memory addresses.
     *
     * @param addrs  An array of structures describing memory addresses to process.
     * @param count  The number of elements in addrs.
     *
     * @return none
     */
    void Process(ReuseEntry* addrs, uint64_t count);

    /**
     * Process multiple memory addresses.
     *
     * @param addrs  A std::vector of memory addresses to process.
     *
     * @return none
     */
    void Process(std::vector<ReuseEntry> rs);

    /**
     * Process multiple memory addresses.
     *
     * @param addrs  A std::vector of memory addresses to process.
     *
     * @return none
     */
    void Process(std::vector<ReuseEntry*> addrs);

    /**
     * Get a reuse distance for a memory address without tracking statistics for it.
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
     * @return The ReuseStats object associated with parameter id.
     */
    ReuseStats* GetStats(uint64_t id);

    /**
     * Get the size of the window for this ReuseDistance object.
     *
     * @return The size of the window for this ReuseDistance object.
     */
    uint64_t GetWindowSize() { return windowsize; }

    /**
     * Increment the internal sequence count for this ReuseDistance object.
     * This has the effect of fast forwarding in the memory address stream.
     * Possibly useful if you are using sampling on your memory address stream.
     *
     * @param count  The amount of the increment.
     *
     * @return none
     */
    void IncrementSequence(uint64_t count) { sequence += count; }

    /**
     * Get a std::vector containing all of the unique indices processed
     * by this ReuseDistance object.
     *
     * @param ids  A std::vector which will contain the ids. It is an error to
     * pass this vector non-empty (that is addrs.size() == 0 is enforced).
     *
     * @return none
     */
    void GetIndices(std::vector<uint64_t>& ids);

    /**
     * Get a std::vector containing all of the addresses currently in this ReuseDistance
     * object's active window.
     *
     * @param addrs  A std::vector which will contain the addresses. It is an error to
     * pass this vector non-empty (that is addrs.size() == 0 is enforced).
     *
     * @return none
     */
    void GetActiveAddresses(std::vector<uint64_t>& addrs);

    /**
     * Get the sequence value for an address currently in this ReuseDistance object's
     * active window.
     *
     * @param addr  An address. Addresses not in this object's active window will generate
     * a return value of 0.
     *
     * @return The sequence value for addr, or 0 if addr is not in this object's active window.
     */
    uint64_t GetSequenceValue(uint64_t addr);
    

    /**
     * Get this ReuseDistance object's current sequence.
     *
     * @return This ReuseDistance object's current sequence.
     */
    uint64_t GetCurrentSequence() { return sequence; }


    /**
     * Clean up the address window. That is, reclaim memory for all addresses strictly outside
     * the maximum window size. This is done for you periodically so you don't ever really
     * need to call this yourself.
     *
     * @return none
     */
    void Cleanup();

    /**
     * Set the frequency with which Clean is called. By default this is defined using the minimum
     * of ReuseDistance::MinimumCleanFrequency and the window size.
     *
     * @param c  The frequency with which to call Cleanup.
     *
     * @return none
     */
    void SetCleanFrequency(uint64_t c);
};

