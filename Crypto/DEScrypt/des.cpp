#include "des.h"
#include "string.h"

using namespace std;

template<typename T>
void setBit(T &byte, uint8_t index, bool value)
{
    if (value)
    {
        byte |= ((T)1 << (sizeof(T) * BITS_IN_BYTE - 1 - index));
    }
    else
    {
        byte &= ~((T)1 << (sizeof(T) * BITS_IN_BYTE - 1 - index));
    }
}

template<typename T>
bool getBit(T byte, uint8_t index)
{
    return byte & ((T)1 << (sizeof(T) * BITS_IN_BYTE - 1 - index));
}

template<typename T>
void cyclicShift(T &bits, uint8_t bitsCount, uint8_t shift, bool left)
{
    uint8_t tmp = 0;
    uint8_t offset = (sizeof(T) * BITS_IN_BYTE) - bitsCount;
    if(left)
    {
        tmp |= (uint8_t)(bits >> (offset + bitsCount - shift));
        bits = bits << shift;
        bits |= (uint32_t)tmp << offset;
    }
    else
    {
        tmp |= (uint8_t)((bits << (bitsCount - shift)) >> (offset + bitsCount - shift));
        bits = (bits >> (shift + offset)) << offset;
        bits |= (uint32_t)tmp << (offset + bitsCount - shift);
    }
}

void PrintBlock(uint64_t block)
{
    char *str;
    str = (char*)//"4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|0|9|8|7|6|5|4|3|2|1|";
                   "1|2|3|4|5|6|7|8| 9|0|1|2|3|4|5|6| 7|8|9|0|1|2|3|4| 5|6|7|8|9|0|1|2| 3|4|5|6|7|8|9|0| 1|2|3|4|5|6|7|8| 9|0|1|2|3|4|5|6| 7|8|9|0|1|2|3|4|";
    qDebug(str);
    delete str;
    str = new char[BITS_IN_BLOCK * 2 + 8 + 1];
    str[0] = '\0';
    for(uint8_t i = 0; i < BITS_IN_BLOCK; i++)
    {
        strcat(str, getBit(block, i) ? "1" : "0");
        strcat(str, "|");
        if((i + 1) % 8 == 0)
        {
            strcat(str, " ");
        }
    }
    qDebug(str);
    delete str;
}

uint64_t tableTransform(uint64_t src, uint8_t *table, uint8_t table_size)
{
    uint64_t result = 0;
    //qDebug("SRC: %#llX", src);
    //PrintBlock(src);
    for(uint32_t j = 0; j < table_size; j++)
    {
        setBit(result, j, getBit(src, table[j] - 1));
        //qDebug("TABLE %d [%d] bit: %d: %#llX", j, table[j], getBit(src, table[j] - 1), result);
        //PrintBlock(result);
    }
    return result;
}

uint64_t DES::generateKey()
{
    uint64_t key = 0;
    //count of 1, should be odd
    uint8_t counter;
    bool randomBit;
    for(int i = 0; i < BYTES_IN_BLOCK; i++)
    {
        counter = 0;
        for(int j = 0; j < BITS_IN_BYTE - 1; j++)
        {
            randomBit = rand() > RAND_MAX/2;
            if(randomBit)
            {
                counter++;
            }
            setBit(key, i * BITS_IN_BYTE + j, randomBit);
        }
        setBit(key, (i + 1) * BITS_IN_BYTE - 1, ((counter%2 == 0) || (counter == 0)));
    }
    qDebug("KEY: %#llX", key);
    PrintBlock(key);
    return key;
}

void DES::PrepareKey(uint64_t &key, uint64_t &cd, uint8_t index, bool encrypt)
{
    key = 0;

    uint32_t mask = ~((uint32_t)0xF);

    //qDebug("cd %d: %#llX",index, cd);
   // PrintBlock(cd);

    uint32_t c = (uint32_t)(mask & (cd >> (BITS_IN_BLOCK/2)));
    //qDebug("c %d: %#X", index, c);
    //PrintBlock((uint64_t)c);

    uint32_t d = (uint32_t)(mask & (cd >> (BITS_IN_BYTE/2)));
    //qDebug("d %d: %#X",index, d);
    //PrintBlock((uint64_t)d);

    if(encrypt)
    {
        cyclicShift(c, BITS_IN_SUBKEY/2, key_offsets[index], true);
        cyclicShift(d, BITS_IN_SUBKEY/2, key_offsets[index], true);
    }
    else if(index != 0)
    {
        cyclicShift(c, BITS_IN_SUBKEY/2, key_offsets[index], false);
        cyclicShift(d, BITS_IN_SUBKEY/2, key_offsets[index], false);
    }

    //qDebug("c after %d: %#X",index, c);
    //PrintBlock((uint64_t)c);
    //
    //qDebug("d after %d: %#X",index, d);
    //PrintBlock((uint64_t)d);

    cd = ((uint64_t)c << BITS_IN_BLOCK/2) | ((uint64_t)d << BITS_IN_BYTE/2);
    //qDebug("cd after shift %d: %#llX", key_offsets[index], cd);
    //PrintBlock(cd);

    key = tableTransform(cd, key_final_transform, TRANSFORMED_KEY_SIZE);
    //qDebug("key %d: %#llX", index, key);
    //PrintBlock(key);
}

uint32_t DES::sTransform(uint64_t expandedHalfBlock)
{
    uint8_t rowIndex = 0;
    uint8_t columnIndex = 0;
    uint32_t result = 0;
    //qDebug("EXPANDED HALF: %#llX", expandedHalfBlock);
    //PrintBlock(expandedHalfBlock);
    for(uint32_t i = 0; i < S_BLOCKS_COUNT; i++)
    {
        setBit(rowIndex,    0, getBit(expandedHalfBlock, i * BITS_IN_B_BLOCK));
        setBit(rowIndex,    1, getBit(expandedHalfBlock, (i + 1) * BITS_IN_B_BLOCK - 1));
        rowIndex = rowIndex >> 6;

        setBit(columnIndex, 0, getBit(expandedHalfBlock, i * BITS_IN_B_BLOCK + 1));
        setBit(columnIndex, 1, getBit(expandedHalfBlock, i * BITS_IN_B_BLOCK + 2));
        setBit(columnIndex, 2, getBit(expandedHalfBlock, i * BITS_IN_B_BLOCK + 3));
        setBit(columnIndex, 3, getBit(expandedHalfBlock, i * BITS_IN_B_BLOCK + 4));
        columnIndex = columnIndex >> 4;

        //qDebug("%d row: %d", i, rowIndex);
        //qDebug("%d index: %d", i, columnIndex);

        result |= (uint32_t)s_blocks[i][rowIndex][columnIndex] << (S_BLOCKS_COUNT - 1 - i) * BITS_IN_BYTE/2;
        //qDebug("s block: %#X", s_blocks[i][rowIndex][columnIndex]);
        //PrintBlock(s_blocks[i][rowIndex][columnIndex]);
        //qDebug("pre res: %#X", result);
        //PrintBlock(result);
    }
    //qDebug("HALF RESULT: %#X", result);
    //PrintBlock(result);
    return result;
}

uint32_t DES::feistelFunction(uint32_t &halfBlock, uint64_t &key)
{
    //qDebug("HALF_BLOCK: %#X", halfBlock);
    //PrintBlock(halfBlock);

    uint64_t expandedBlock = tableTransform((uint64_t)halfBlock << BITS_IN_BLOCK/2, expansion, TRANSFORMED_KEY_SIZE);
    //qDebug("EXPANDED BLOCK: %#llX", expandedBlock);
    //PrintBlock(expandedBlock);

    //qDebug("key: %#llX", key);
    //PrintBlock(key);

    expandedBlock ^= key;
    //qDebug("XOR: %#llX", expandedBlock);
    //PrintBlock(expandedBlock);

    expandedBlock = sTransform(expandedBlock);
    //qDebug("HALF_BLOCK FINAL: %#llX", expandedBlock);
    //PrintBlock(expandedBlock);

    return (uint32_t)(tableTransform(expandedBlock << BITS_IN_BLOCK/2, permutation_func, BITS_IN_BLOCK/2) >> BITS_IN_BLOCK/2);
}

uint64_t DES::feistelTransform(uint32_t halfL, uint32_t halfR, uint64_t &key)
{
    uint32_t ff = feistelFunction(halfR, key);
    //qDebug("HALF_R: %#X", halfR);
    //PrintBlock(halfR);
    //
    //qDebug("HALF_L: %#X", halfL);
    //PrintBlock(halfL);
    //
    //qDebug("SHIFT HALF_R: %#llX", ((uint64_t)halfR << BITS_IN_BLOCK/2));
    //PrintBlock(((uint64_t)halfR << BITS_IN_BLOCK/2));
    //
    //
    //qDebug("FF: %#X", ff);
    //PrintBlock(ff);
    //
    //qDebug("FF XOR HALF_L: %#llX", (uint64_t)(ff ^ halfL));
    //PrintBlock((uint64_t)(ff ^ halfL));

    return (uint64_t)(ff ^ halfL) | ((uint64_t)halfR << BITS_IN_BLOCK/2);
}

uint64_t DES::cryptBlock(uint64_t block, uint64_t key, bool encrypt)
{
    //ip transform

    //key = 0x133457799BBCDFF1;
    //qDebug("key: %#llX", key);
    //PrintBlock(key);

    block = tableTransform(block, ip, BITS_IN_BLOCK);
    //qDebug("IP: %#llX", block);
    //PrintBlock(block);

    uint64_t cd = tableTransform(key, key_prep_transform, BITS_IN_SUBKEY);
    //qDebug("CD: %#llX", cd);
    //PrintBlock(cd);

    //16 loops of feistel transformation
    for(int i = 0; i < FEISTEL_TRANSFORM_COUNT; i++)
    {
        PrepareKey(key, cd, i, encrypt);

        //qDebug("full block %d: %#llX", i, block);
        //PrintBlock(block);

        //qDebug("half_L %d: %#X", i, (uint32_t)(block >> (encrypt ? BITS_IN_BLOCK/2 : 0)));
        //PrintBlock((uint32_t)(block >> (encrypt ? BITS_IN_BLOCK/2 : 0)));

        //qDebug("half_R %d: %#X", i, (uint32_t)(block >> (encrypt ? 0 : BITS_IN_BLOCK/2)));
        //PrintBlock((uint32_t)(block >> (encrypt ? 0 : BITS_IN_BLOCK/2)));

        //qDebug("key: %#llX", key);
        //PrintBlock(key);
        //
        //qDebug("BLOCK F %d: %#llX", i, block);
        //PrintBlock(block);

        block = feistelTransform((uint32_t)(block >> BITS_IN_BLOCK/2),
                                 (uint32_t)(block >> 0),
                                 key);

        //qDebug("FEISTEL %d: %#llX", i, block);
        //PrintBlock(block);
    }
    //final swap
    uint32_t swap = (uint32_t)block;
    block = (block >> BITS_IN_BLOCK/2) | ((uint64_t)swap << BITS_IN_BLOCK/2);
    //qDebug("final swap: %#llX", block);
    //PrintBlock(block);

    //final permutation
    return tableTransform(block, final_permutation, BITS_IN_BLOCK);
}

uint32_t DES::encrypt(char *src, char **encrypted, uint32_t srcLength, uint64_t key)
{
    qDebug("START ENCRYPT");
    uint32_t blocksCount = (uint32_t)ceil(srcLength * 1.f / BYTES_IN_BLOCK);
    uint32_t resLength = blocksCount * BYTES_IN_BLOCK;
    (*encrypted) = new char[resLength];
    uint64_t *blocks = new uint64_t[blocksCount];
    qDebug() << "SRC TEXT: " << src;

    for(uint32_t i = 0; i < blocksCount; i++)
    {
        blocks[i] = 0;
        for(uint32_t j = 0; j < BYTES_IN_BLOCK; j++)
        {
            if(i * BYTES_IN_BLOCK + j >= srcLength)
            {
                blocks[i] |= ((uint64_t)' ') << (j * BYTES_IN_BLOCK);
            }
            else
            {
                blocks[i] |= (uint64_t)src[i * BYTES_IN_BLOCK + j] << (j * BYTES_IN_BLOCK);
            }
        }
        qDebug("BLOCK %d, %#llX", i, blocks[i]);
        PrintBlock(blocks[i]);
    }

    for(uint32_t i = 0; i < blocksCount; i++)
    {
        //blocks[i] = 0x0123456789ABCDEF;
        //qDebug("BLOCK: %#llX", blocks[i]);
        //PrintBlock(blocks[i]);

        blocks[i] = cryptBlock(blocks[i], key, true);
        qDebug("ENCRYPTED BLOCK %d: %#llX", i, blocks[i]);
        PrintBlock(blocks[i]);
    }

    memcpy((*encrypted), blocks, resLength);

    delete[] blocks;
    return resLength;
}

uint32_t DES::decrypt(char *src, char **decrypted, uint32_t srcLength, uint64_t key)
{ 
    qDebug("START DECRYPT");
    if(srcLength % BYTES_IN_BLOCK != 0)
    {
       qDebug("text mod 8 != 0, length: %d", srcLength);
       return -1;
    }
    uint32_t blocksCount = srcLength/BYTES_IN_BLOCK;
    uint64_t *blocks = new uint64_t[blocksCount];
    uint32_t resLength = blocksCount * BYTES_IN_BLOCK;
    (*decrypted) = new char[blocksCount];
    qDebug() << "SRC TEXT: " << src;

    memcpy(blocks, src, resLength);

    for(uint32_t i = 0; i < blocksCount; i++)
    {
        qDebug("BLOCK %d, %#llX", i, blocks[i]);
        PrintBlock(blocks[i]);

        blocks[i] = cryptBlock(blocks[i], key, false);

        qDebug("DECRYPTED BLOCK %d: %#llX", i, blocks[i]);
        PrintBlock(blocks[i]);
    }

    memcpy((*decrypted), blocks, resLength);

    delete[] blocks;
    return resLength;
}
