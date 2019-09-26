#ifndef RSA_H
#define RSA_H

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <QDebug>

#define MAX_PRIMARY_NUM 65536

typedef struct {
    uint32_t pub;
    uint32_t priv;
    uint32_t module;
} RSAKeys;

typedef uint32_t uint;

class RSA
{
public:
    void encrypt(char* text, uint** e, uint len, RSAKeys keys);
    void decrypt(uint* text, char** d, uint len, RSAKeys keys);
    RSAKeys generateKeys();

private:

};

#endif // RSA_H
