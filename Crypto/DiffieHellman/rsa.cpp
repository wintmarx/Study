#include "rsa.h"
#include "string.h"
#include<stdio.h>
#include<cmath>
#include<ctime>
#include<cstdlib>
#include<string.h>
#include<bits/stdc++.h>
#include<QFile>

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

// Returns true if n is prime
bool isPrime(int n)
{
    // Corner cases
    if (n <= 1)  return false;
    if (n <= 3)  return true;

    // This is checked so that we can skip
    // middle five numbers in below loop
    if (n%2 == 0 || n%3 == 0) return false;

    for (int i=5; i*i<=n; i=i+6)
        if (n%i == 0 || n%(i+2) == 0)
            return false;

    return true;
}

// Utility function to store prime factors of a number
void findPrimefactors(std::vector<uint> &s, uint n)
{
    // Print the number of 2s that divide n
    while (n%2 == 0)
    {
        s.push_back(2);
        n = n/2;
    }

    // n must be odd at this point. So we can skip
    // one element (Note i = i +2)
    for (uint i = 3; i <= sqrt(n); i = i+2)
    {
        // While i divides n, print i and divide n
        while (n%i == 0)
        {
            s.push_back(i);
            n = n/i;
        }
    }

    // This condition is to handle the case when
    // n is a prime number greater than 2
    if (n > 2)
        s.push_back(n);
}

// Function to find smallest primitive root of n
uint findPrimitive(uint n)
{
    std::vector<uint> s;

    // Check if n is prime or not
    if (isPrime(n)==false)
        return -1;

    // Find value of Euler Totient function of n
    // Since n is a prime number, the value of Euler
    // Totient function is n-1 as there are n-1
    // relatively prime numbers.
    uint phi = n-1;

    // Find prime factors of phi and store in a set
    findPrimefactors(s, phi);

    // Check for every number from 2 to phi
    for (uint r=2; r<=phi; r++)
    {
        // Iterate through all prime factors of phi.
        // and check if we found a power with value 1
        bool flag = false;
        for (auto it = s.begin(); it != s.end(); it++)
        {

            // Check if r^((phi)/primefactors) mod n
            // is 1 or not
            if (pows(r, phi/(*it), n) == 1)
            {
                flag = true;
                break;
            }
         }

         // If there was no power with value 1.
         if (flag == false)
           return r;
    }

    // If no primitive root found
    return -1;
}

uint primitiveRoot(uint n)
{
    return findPrimitive(n);
}

DHKey RSA::generateKeys()
{
    srand(time(NULL));
    DHKey key;
    //printf("\nGEN KEYS");
    do
    {
        key.n = rand()%MAX_PRIMARY_NUM;
    } while (!ferma(key.n));
    //printf("\np %d", p);

    key.g = primitiveRoot(key.n);

    return key;
}


void RSA::send(DHKey &k)
{
    QFile file("channel.txt");
    if(file.open(QIODevice::WriteOnly| QIODevice::Text))
    {
        QString data = QString::number(k.n) + "\n" + QString::number(k.g) + "\n" + QString::number(pows(k.g, k.priv, k.n));
        file.write(data.toLatin1().data(), data.toLatin1().size());
        file.close();
    }
}

DHKey RSA::recv(uint priv)
{
    DHKey k;
    QFile file("channel.txt");
    if(file.open(QIODevice::ReadWrite| QIODevice::Text))
    {
        QTextStream in(&file);
        QString line = in.readLine();
        k.n = line.toUInt();
        qDebug("n %d", k.n);
        line = in.readLine();
        k.g = line.toUInt();
        qDebug("g %d", k.g);
        line = in.readLine();
        k.pub = line.toUInt();
        qDebug("pub %d", k.pub);
        k.priv = pows(k.pub, priv, k.n);
        qDebug("priv %d", k.priv);
        file.close();
    }
    else
    {
        k.n = 0;
    }
    return k;
}
