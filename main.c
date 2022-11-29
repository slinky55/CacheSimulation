#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


typedef struct 
{
    uint32_t numSets;
    uint32_t numBlocks;
    uint32_t numBytes;

} Cache;


int main(int argc, char* argv[])
{
    assert(argc == 4);
    
    Cache cache;
    cache.numSets = atoi(argv[1]);
    cache.numBlocks = atoi(argv[2]);
    cache.numBytes = atoi(argv[3]);

    printf("Sets: %d", cache.numSets);
    printf("Blocks: %d", cache.numBlocks);
    printf("Bytes per block: %d", cache.numBytes);

    return 0;
}
