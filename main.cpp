#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>

#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "Cache.hpp"

void FullCache(uint32_t _cacheSize,
               uint32_t _lineSize,
               std::string_view _traceFile,
               bool _useFIFO = true)
{
    std::cout << "Simulating fully associative cache...\n";
    std::cout << "Cache size: " << _cacheSize << "\n";
    std::cout << "Line size: " << _lineSize << "\n";
    std::cout << "Lines in cache: " << _cacheSize / _lineSize << "\n\n";

    FullyAssociativeCache cache {
            _cacheSize,
            _lineSize,
            _useFIFO
    };

    std::ifstream in;
    std::string line;

    in.open(_traceFile.data());
    if (!in.is_open())
    {
        std::cout << "Error opening file: " << _traceFile.data() << std::endl;
        exit(1);
    }

    while (std::getline(in, line))
    {
        std::stringstream ss { line };
        std::string op;
        uint32_t addr;

        ss >> op >> std::hex >> addr;

        bool hit { cache.AddrInCache(addr) };
#ifdef SHOW_DEBUG
        std::cout << (hit ? " | Hit" : " | Miss") << "\n";
#endif
    }

    std::cout << "Hit rate: " << cache.HitRate() << "\n";

    in.close();
}

void DirectCache(uint32_t _cacheSize,
                 uint32_t _lineSize,
                 std::string_view _traceFile,
                 bool _useFIFO = true)
{
    std::cout << "Simulating direct mapped cache\n";
    std::cout << "Cache size: " << _cacheSize << "\n";
    std::cout << "Line size: " << _lineSize << "\n";
    std::cout << "Lines in cache: " << _cacheSize / _lineSize << "\n\n";

    DirectMappedCache cache {
            _cacheSize,
            _lineSize,
            _useFIFO
    };

    std::ifstream in;
    std::string line;

    in.open(_traceFile.data());
    if (!in.is_open())
    {
        std::cout << "Error opening file: " << _traceFile.data() << std::endl;
        exit(1);
    }

    while (std::getline(in, line))
    {
        std::stringstream ss { line };
        std::string op;
        uint32_t addr;

        ss >> op >> std::hex >> addr;

        bool hit { cache.AddrInCache(addr) };
#ifdef SHOW_DEBUG
        std::cout << (hit ? " | Hit" : " | Miss") << "\n";
#endif
    }

    std::cout << "Hit rate: " << cache.HitRate() << "\n";

    in.close();
}

void SetAssociative(uint32_t _cacheSize,
                    uint32_t _lineSize,
                    uint32_t _associativity,
                    std::string_view _traceFile,
                    bool _useFIFO = true)
{
    std::cout << "Simulating set associative cache\n";
    std::cout << "Cache size: " << _cacheSize << "\n";
    std::cout << "Line size: " << _lineSize << "\n";
    std::cout << "Lines in cache: " << _cacheSize / _lineSize << "\n";
    std::cout << "Associativity: " << _associativity << " way\n\n";

    SetAssociativeCache cache {
            _cacheSize,
            _lineSize,
            _associativity,
            _useFIFO
    };

    std::ifstream in;
    std::string line;

    in.open(_traceFile.data());
    if (!in.is_open())
    {
        std::cout << "Error opening file: " << _traceFile.data() << std::endl;
        exit(1);
    }

    auto start = std::chrono::high_resolution_clock::now();
    while (std::getline(in, line))
    {
        std::stringstream ss { line };
        std::string op;
        uint32_t addr;

        ss >> op >> std::hex >> addr;

        bool hit { cache.AddrInCache(addr) };
#ifdef SHOW_DEBUG
        std::cout << (hit ? " | Hit" : " | Miss") << "\n";
#endif
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Hit rate: " << cache.HitRate() << "\n";
    std::cout << "Time taken: " << duration.count() << " milliseconds\n";

    in.close();
}

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        std::cout << "Usage: " << argv[0] << " <cache size> <block size> <type> <trace file> [--lru]\n";
        exit(1);
    }

    uint32_t cacheSize { static_cast<uint32_t>(std::stoi(argv[1])) };
    uint32_t lineSize { static_cast<uint32_t>(std::stoi(argv[2])) };

    if (double cacheLog2 { std::log2(cacheSize) };
        cacheLog2 != std::floor(cacheLog2))
    {
        std::cout << "Cache size must be a power of 2\n";
        exit(1);
    }

    if (double lineLog2 { std::log2(lineSize) };
        lineLog2 != std::floor(lineLog2))
    {
        std::cout << "Line size must be a power of 2\n";
        exit(1);
    }

    if (std::strcmp(argv[3], "-f") == 0)
    {
        if (argc < 6)
            FullCache(std::stoi(argv[1]), std::stoi(argv[2]), argv[4]);
        else
        {
            if (std::strcmp(argv[5], "--lru") != 0)
            {
                std::cout << "Invalid replacement strategy: " << argv[5] << "\n";
                exit(1);
            }
            FullCache(std::stoi(argv[1]), std::stoi(argv[2]), argv[4], false);
        }

    }

    if (std::strcmp(argv[3], "-d") == 0)
    {
        if (argc < 6)
            DirectCache(std::stoi(argv[1]), std::stoi(argv[2]), argv[4]);
        else
        {
            if (std::strcmp(argv[5], "--lru") != 0)
            {
                std::cout << "Invalid replacement strategy: " << argv[5] << "\n";
                exit(1);
            }
            DirectCache(std::stoi(argv[1]), std::stoi(argv[2]), argv[4], false);
        }
    }

    if (std::strcmp(argv[3], "-s") == 0)
    {
        std::cout << "Enter associativity: ";
        uint32_t associativity;
        std::cin >> associativity;

        if (double assocLog2 { std::log2(associativity) }; assocLog2 != std::floor(assocLog2))
        {
            std::cout << "Associativity must be a power of 2\n";
            exit(1);
        }

        if (argc < 6)
            SetAssociative(std::stoi(argv[1]), std::stoi(argv[2]), associativity, argv[4]);
        else
            SetAssociative(std::stoi(argv[1]), std::stoi(argv[2]), associativity, argv[4], false);
    }

    return 0;
}