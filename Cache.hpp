#pragma once

#include <cstdint>
#include <cmath>

#include <memory>
#include <array>
#include <queue>
#include <limits>

constexpr uint32_t ADDR_SIZE { 32 };

// Used to create a mask to isolate bits from a 32-bit address
uint32_t mask(uint32_t a, uint32_t b)
{
    uint32_t r = 0;
    for (uint32_t i = a; i <= b; i++)
        r |= 1 << i;
    return r;
}

struct Line
{
    uint32_t tag { 0x0 };
    std::size_t counter { 0 };
    bool inUse { false };
};

struct Set
{
    std::unique_ptr<Line[]> lines;
    std::queue<Line*> toReplace;
};

class Cache
{
public:
    Cache(uint32_t _cacheSize,
          uint32_t _lineSize,
          bool _useFIFO = true)
          :
          cacheSize { _cacheSize },
          lineSize { _lineSize },
          totalLines { _cacheSize / _lineSize },
          useFIFO { _useFIFO }
    {}

    virtual ~Cache() = default;

    [[nodiscard]] double HitRate() const { return static_cast<double>(hits) / static_cast<double>(hits + misses);}
    [[nodiscard]] double MissRate() const { return static_cast<double>(misses) / static_cast<double>(hits + misses);}

    [[nodiscard]] virtual bool AddrInCache(uint32_t _addr) = 0;
protected:
    uint32_t cacheSize;
    uint32_t lineSize;
    uint32_t totalLines;

    bool useFIFO;

    std::unique_ptr<Set[]> sets;

    uint32_t hits { 0 };
    uint32_t misses { 0 };
};

// 1 set of n lines
class FullyAssociativeCache : public Cache
{
public:
    FullyAssociativeCache(uint32_t _cacheSize,
                          uint32_t _lineSize,
                          bool _useFIFO = true)
                          :
                          Cache { _cacheSize, _lineSize, _useFIFO }
    {
        sets = std::make_unique<Set[]>(1);
        sets[0].lines = std::make_unique<Line[]>(totalLines);
    }

    ~FullyAssociativeCache() override = default;

    [[nodiscard]] bool AddrInCache(uint32_t _addr) override
    {
        Set& set = sets[0];

        uint32_t offsetWidth { static_cast<uint32_t>(std::log2(lineSize)) };
        uint32_t tag { (mask(offsetWidth, ADDR_SIZE - 1) & _addr) >> offsetWidth };

#ifdef SHOW_DEBUG
        std::cout << "Address: 0x" << std::hex << _addr << " | Tag: 0x" << std::hex << tag;
#endif

        for (uint32_t i = 0; i < totalLines; i++)
        {
            if (set.lines[i].inUse && set.lines[i].tag == tag)
            {
                if (!useFIFO)
                    set.lines[i].counter++;
                hits++;
                return true;
            }
        }

        misses++;

        for (uint32_t i = 0; i < totalLines; i++)
        {
            if (!set.lines[i].inUse)
            {
                set.lines[i].inUse = true;
                set.lines[i].tag = tag;
                if(useFIFO)
                    set.toReplace.push(&set.lines[i]);
                else
                    set.lines[i].counter = 0;
                return false;
            }
        }

        if (useFIFO)
        {
            auto* lineToReplace { set.toReplace.front() };
            lineToReplace->tag = tag;
            set.toReplace.pop();
            set.toReplace.push(lineToReplace);
        }
        else
        {
            uint32_t minIndex { 0 };
            std::size_t minCounter { std::numeric_limits<std::size_t>::max() };

            for (uint32_t l { 0 }; l < totalLines; l++)
            {
                if (set.lines[l].counter < minCounter)
                {
                    minCounter = set.lines[l].counter;
                    minIndex = l;
                }
            }

            set.lines[minIndex].tag = tag;
        }

        return false;
    }
};

// n sets of 1 line
class DirectMappedCache : public Cache
{
public:
    DirectMappedCache(uint32_t _cacheSize,
                      uint32_t _bytesInLine,
                      bool _useFIFO = true)
                      :
                      Cache { _cacheSize, _bytesInLine, _useFIFO }
    {
        sets = std::make_unique<Set[]>(totalLines);
        for (uint32_t i = 0; i < totalLines; i++)
            sets[i].lines = std::make_unique<Line[]>(1);
    }

    ~DirectMappedCache() override = default;

    [[nodiscard]] bool AddrInCache(uint32_t _addr) override
    {
        uint32_t bitsInOffset { static_cast<uint32_t>(std::log2(lineSize)) };
        uint32_t bitsInLine { static_cast<uint32_t>(std::log2(totalLines)) };

        uint32_t line { (mask(bitsInOffset, bitsInOffset + bitsInLine - 1) & _addr) >> bitsInOffset };
        uint32_t tag { (mask(bitsInOffset + bitsInLine, ADDR_SIZE - 1) & _addr) >> (bitsInOffset + bitsInLine) };

#ifdef SHOW_DEBUG
        std::cout << "Address: 0x" << std::hex << _addr << " | Tag: 0x" << std::hex << tag << " | Line: " << std::hex << line;
#endif

        // Check to see if the tag is in the cache
        if (sets[line].lines[0].inUse && sets[line].lines[0].tag == tag)
        {
            hits++;
            return true;
        }

        if (!sets[line].lines[0].inUse) sets[line].lines[0].inUse = true;
        sets[line].lines[0].tag = tag;

        return false;
    }
};

// n sets of m lines
class SetAssociativeCache : public Cache
{
public:
    SetAssociativeCache(uint32_t _cacheSize,
                        uint32_t _lineSize,
                        uint32_t _associativity,
                        bool _useFIFO = true)
                        :
                        Cache( _cacheSize, _lineSize, _useFIFO ),
                        associativity { _associativity }
    {
        numSets = totalLines / _associativity;
        sets = std::make_unique<Set[]>(numSets);
        for (uint32_t i = 0; i < numSets; i++)
        {
            sets[i].lines = std::make_unique<Line[]>(_associativity);
        }
    }

    ~SetAssociativeCache() override = default;

    [[nodiscard]] bool AddrInCache(uint32_t _addr) override
    {
        uint32_t offsetWidth { static_cast<uint32_t>(std::log2(lineSize)) };
        uint32_t setWidth { static_cast<uint32_t>(std::log2(numSets)) };

        uint32_t set { (mask(offsetWidth, offsetWidth + setWidth - 1) & _addr) >> offsetWidth };
        uint32_t tag { (mask(offsetWidth + setWidth, ADDR_SIZE - 1) & _addr) >> (offsetWidth + setWidth) };

#ifdef SHOW_DEBUG
        std::cout << "Address: 0x" << std::hex << _addr << " | Tag: 0x" << std::hex << tag << " | Set: " << std::hex << set;
#endif

        for (uint32_t i = 0; i < associativity; i++)
        {
            if (sets[set].lines[i].inUse &&
                sets[set].lines[i].tag == tag)
            {
                if (!useFIFO)
                    sets[set].lines[i].counter++;

                hits++;
                return true;
            }
        }

        misses++;

        for (uint32_t i = 0; i < associativity; i++)
        {
            if (!sets[set].lines[i].inUse)
            {
                sets[set].lines[i].inUse = true;
                sets[set].lines[i].tag = tag;
                if (useFIFO)
                    sets[set].toReplace.push(&sets[set].lines[i]);
                else
                    sets[set].lines[i].counter = 0;
                return false;
            }
        }

        if (useFIFO)
        {
            auto* lineToReplace { sets[set].toReplace.front() };
            lineToReplace->tag = tag;
            sets[set].toReplace.pop();
            sets[set].toReplace.push(lineToReplace);
        }
        else
        {
            uint32_t minIndex { 0 };
            std::size_t minCounter { std::numeric_limits<std::size_t>::max() };

            for (uint32_t l { 0 }; l < totalLines; l++)
            {
                if (sets[set].lines[l].counter < minCounter)
                {
                    minCounter = sets[set].lines[l].counter;
                    minIndex = l;
                }
            }

            sets[set].lines[minIndex].tag = tag;
        }

        return false;
    }

private:
    uint32_t numSets;
    uint32_t associativity;
};
