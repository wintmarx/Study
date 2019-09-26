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
    uint32_t n;
    uint32_t g;
} DHKey;

typedef uint32_t uint;

class RSA
{
public:
    void send(DHKey &k);
    DHKey recv(uint priv);
    DHKey generateKeys();

private:

};

#endif // RSA_H
