#include "rsa.h"
#include "string.h"
#include<stdio.h>
#include<cmath>
#include<ctime>
#include<cstdlib>
#include<string.h>

uint gcd(uint a, uint b){
    if (b == 0) { return a; }
    return gcd(b, a % b);
}

uint mul(uint a, uint b, uint m){
    if (b == 1) { return a; }
    if (b%2 == 0)
    {
        uint t = mul(a, b/2, m);
        return (2 * t) % m;
    }
    return (mul(a, b - 1, m) + a) % m;
}

uint pows(uint a, uint b, uint m){
    if (b == 0) { return 1; }
    if (b%2 == 0)
    {
        uint t = pows(a, b/2, m);
        return mul(t , t, m) % m;
    }
    return (mul(pows(a, b - 1, m) , a, m)) % m;
}

bool ferma(uint x){
    if (x == 2) { return true; }
    srand(time(NULL));
    for (int i = 0; i < 100; i++)
    {
        uint a = (rand() % (x - 2)) + 2;
        if (gcd(a, x) != 1) { return false; }
        if (pows(a, x-1, x) != 1) { return false; }
    }
    return true;
}

void extendedEcd(uint a, uint b, uint *x, uint *y, uint *d)
{
    uint q;
    uint r;
    uint x1;
    uint x2;
    uint y1;
    uint y2;

    if (b == 0)
    {
        *d = a;
        *x = 1;
        *y = 0;
        return;
    }

    x2 = 1;
    x1 = 0;
    y2 = 0;
    y1 = 1;

    while (b > 0)
    {
        q = a / b;
        r = a - q * b;
        *x = x2 - q * x1;
        *y = y2 - q * y1;
        a = b;
        b = r;
        x2 = x1;
        x1 = *x;
        y2 = y1;
        y1 = *y;
    }

    *d = a;
    *x = x2;
    *y = y2;
}

uint modinv(uint u, uint v)
{
    uint inv, u1, u3, v1, v3, t1, t3, q;
    int iter;
    /* Step X1. Initialise */
    u1 = 1;
    u3 = u;
    v1 = 0;
    v3 = v;
    /* Remember odd/even iterations */
    iter = 1;
    /* Step X2. Loop while v3 != 0 */
    while (v3 != 0)
    {
        /* Step X3. Divide and "Subtract" */
        q = u3 / v3;
        t3 = u3 % v3;
        t1 = u1 + q * v1;
        /* Swap */
        u1 = v1; v1 = t1; u3 = v3; v3 = t3;
        iter = -iter;
    }
    /* Make sure u3 = gcd(u,v) == 1 */
    if (u3 != 1)
        return 0;   /* Error: No inverse exists */
    /* Ensure a positive result */
    if (iter < 0)
        inv = v - u1;
    else
        inv = u1;
    return inv;
}

RSAKeys RSA::generateKeys()
{
    srand(time(NULL));
    RSAKeys keys;
    uint p;
    uint q;
    //printf("\nGEN KEYS");
    do
    {
        p = rand()%65536;
    } while (!ferma(p));
    //printf("\np %d", p);

    do
    {
        q = rand()%65536;
    } while (q == p || !ferma(q));

    //printf("\nq %d", q);

    keys.module = q * p;
    //printf("\nmod %d", keys.module);
    uint f = (q - 1) * (p - 1);
    //printf("\nf %d", f);
    do
    {
        keys.pub = rand()%(f - 1) + 1;
    } while (gcd(keys.pub, f) != 1);

    keys.priv = modinv(keys.pub, f);

    return keys;
}


void RSA::encrypt(char* text, uint** e, uint len, RSAKeys keys)
{
    (*e) = new uint[len + 1];
    for(uint i = 0; i < len; i++)
    {
        //printf("\nbefore %d", text[i]);
        (*e)[i] = pows(text[i], keys.pub, keys.module);
        //printf("\nafter %d", (*e)[i]);
    }
}

void RSA::decrypt(uint* text, char** d, uint len, RSAKeys keys)
{
    (*d) = new char[len + 1];
    for(uint i = 0; i < len; i++)
    {
        //printf("\nbefore %d", text[i]);
        (*d)[i] = pows(text[i], keys.priv, keys.module);
        //printf("\nafter %d", (*d)[i]);
    }
}
