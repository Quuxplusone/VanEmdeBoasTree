#include <stdlib.h>
#include <memory>
#include "VEBTree.h"

typedef unsigned int uint;
typedef unsigned char uchar;

#define WORD (8 * sizeof(unsigned int))

static uint clz(uint x)
{
  return __builtin_clz(x);
}

static uint ctz(uint x)
{
  return __builtin_ctz(x);
}

static uint fls(uint x)
{
  return WORD - clz(x);
}

static uint
bytes(uint x)
{
    return x/8+(x%8>0);
}

static uint
zeros(uint k)
{
    return ~0<<k;
}

static uint
ones(uint k)
{
    return ~zeros(k);
}

static uint
ipow(uint k)
{
    return 1<<k;
}

static uint
lowbits(uint x, uint k)
{
    return x&ones(k);
}

static uint
highbits(uint x, uint k)
{
    return x>>k;
}

static uint
decode(const unsigned char *D, uint b)
{
    uint x = 0;
    for (uint i = 0; i < b; ++i)
        x |= D[i]<<8*i;
    return x;
}

static void
encode(unsigned char *D, uint b, uint x)
{
    for (uint i = 0; i < b; ++i)
        D[i] = (x>>8*i)&0xff;
}

static void
set(unsigned char *D, uint x)
{
    D[x/8] |= (1<<x%8);
}

static void
unset(unsigned char *D, uint x)
{
    D[x/8] &= ~(1<<x%8);
}

static uint low(VebViewConst T)
{
    if (T.M <= WORD) {
        uint x = decode(T.D,bytes(T.M));
        if (x == 0)
            return T.M;
        return ctz(x);
    }
    return decode(T.D,bytes(T.k));
}

static void
setlow(VebView T, uint x)
{
    if (T.M <= WORD)
        set(T.D,x);
    else
        encode(T.D,bytes(T.k),x);
}

static uint high(VebViewConst T)
{
    if (T.M <= WORD) {
        uint x = decode(T.D,bytes(T.M));
        if (x == 0)
            return T.M;
        return fls(x)-1;
    }
    return decode(T.D+bytes(T.k),bytes(T.k));
}

static void
sethigh(VebView T, uint x)
{
    if (T.M <= WORD)
        set(T.D,x);
    else
        encode(T.D+bytes(T.k),bytes(T.k),x);
}

unsigned int vebsize(unsigned int M)
{
    if (M <= WORD)
        return bytes(M);
    uint k = fls(M-1);
    uint m = highbits(M-1,k/2)+1;
    uint n = ipow(k/2);
    return 2*bytes(k)+vebsize(m)+(m-1)*vebsize(n)+vebsize(M-(m-1)*n);
}

template<class VV>
static VV aux(VV S)
{
    VV T;
    T.k = S.k-S.k/2;
    T.D = S.D+2*bytes(S.k);
    T.M = highbits(S.M-1,S.k/2)+1;
    return T;
}

template<class VV>
static VV branch(VV S, uint i)
{
    VV T;
    uint k = S.k/2;
    uint m = highbits(S.M-1,k)+1;
    uint n = ipow(k);
    if (i < m-1) {
        T.M = n;
        T.k = k;
    } else {
        T.M = S.M-(m-1)*n;
        T.k = fls(T.M-1);
    }
    T.D = S.D+2*bytes(S.k)+vebsize(m)+i*vebsize(n);
    return T;
}

static int
empty(VebViewConst T)
{
    if (T.M <= WORD)
        return decode(T.D,bytes(T.M))==0;
    if (low(T) <= high(T))
        return 0;
    return 1;
}

static void
mkempty(VebView T)
{
    if (T.M <= WORD) {
        encode(T.D,bytes(T.M),0);
        return;
    }
    setlow(T,1);
    sethigh(T,0);
    mkempty(aux(T));
    uint m = highbits(T.M-1,T.k/2)+1;
    for (uint i = 0; i < m; ++i)
        mkempty(branch(T,i));
}

static void
mkfull(VebView T)
{
    if (T.M <= WORD) {
        encode(T.D,bytes(T.M),ones(T.M));
        return;
    }
    setlow(T,0);
    sethigh(T,T.M-1);
    mkfull(aux(T));
    uint m = highbits(T.M-1,T.k/2)+1;
    for (uint i = 0; i < m; ++i) {
        VebView B = branch(T,i);
        mkfull(B);
        if (i == 0)
            vebdel(B,0);
        if (i == m-1)
            vebdel(B,lowbits(T.M-1,T.k/2));
    }
}

VebTree::VebTree(unsigned M, bool full)
{
    this->k = fls(M-1);
    this->D = std::make_unique<unsigned char[]>(vebsize(M));
    this->M = M;
    if (full) {
        mkfull(this->view());
    } else {
        mkempty(this->view());
    }
}

void vebput(VebView T, unsigned int x)
{
    if (x >= T.M)
        return;
    if (T.M <= WORD) {
        set(T.D,x);
        return;
    }
    if (empty(T)) {
        setlow(T,x);
        sethigh(T,x);
        return;
    }
    uint lo = low(T);
    uint hi = high(T);
    if (x == lo || x == hi)
        return;
    if (x < lo) {
        setlow(T,x);
        if (lo == hi)
            return;
        x = lo;
    } else if (x > hi) {
        sethigh(T,x);
        if (lo == hi)
            return;
        x = hi;
    }
    uint i = highbits(x,T.k/2);
    uint j = lowbits(x,T.k/2);
    VebView B = branch(T,i);
    vebput(B,j);
    if (low(B) == high(B))
        vebput(aux(T),i);
}

void vebdel(VebView T, unsigned int x)
{
    if (empty(T) || x >= T.M)
        return;
    if (T.M <= WORD) {
        unset(T.D,x);
        return;
    }
    uint lo = low(T);
    uint hi = high(T);
    if (x < lo || x > hi)
        return;
    if (lo == hi && x == lo) {
        sethigh(T,0);
        setlow(T,1);
        return;
    }
    uint i,j;
    VebView B;
    VebView A = aux(T);
    if (x == lo) {
        if (empty(A)) {
            setlow(T,hi);
            return;
        } else {
            i = low(A);
            B = branch(T,i);
            j = low(B);
            setlow(T,i*ipow(T.k/2)+j);
        }
    } else if (x == hi) {
        if (empty(A)) {
            sethigh(T,lo);
            return;
        } else {
            i = high(A);
            B = branch(T,i);
            j = high(B);
            sethigh(T,i*ipow(T.k/2)+j);
        }
    } else {
        i = highbits(x,T.k/2);
        j = lowbits(x,T.k/2);
        B = branch(T,i);
    }
    vebdel(B,j);
    if (empty(B))
        vebdel(A,i);
}

unsigned int vebsucc(VebViewConst T, unsigned int x)
{
    uint hi = high(T);
    if (empty(T) || x > hi)
        return T.M;
    if (T.M <= WORD) {
        uint y = decode(T.D,bytes(T.M));
        y &= zeros(x);
        if (y > 0)
            return ctz(y);
        return T.M;
    }
    uint lo = low(T);
    if (x <= lo)
        return lo;
    VebViewConst A = aux(T);
    if (empty(A) || x == hi)
        return hi;
    uint i = highbits(x,T.k/2);
    uint j = lowbits(x,T.k/2);
    VebViewConst B = branch(T,i);
    if (!empty(B) && j <= high(B))
        return i*ipow(T.k/2)+vebsucc(B,j);
    i = vebsucc(A,i+1);
    if (i == A.M)
        return hi;
    B = branch(T,i);
    return i*ipow(T.k/2)+low(B);
}

unsigned int vebpred(VebViewConst T, unsigned int x)
{
    uint lo = low(T);
    if (empty(T) || x < lo || x >= T.M)
        return T.M;
    if (T.M <= WORD) {
        uint y = decode(T.D,bytes(T.M));
        y &= ones(x+1);
        if (y > 0)
            return fls(y)-1;
        return T.M;
    }
    uint hi = high(T);
    if (x >= hi)
        return hi;
    VebViewConst A = aux(T);
    if (empty(A) || x == lo)
        return lo;
    uint i = highbits(x,T.k/2);
    uint j = lowbits(x,T.k/2);
    VebViewConst B = branch(T,i);
    if (!empty(B) && j >= low(B))
        return i*ipow(T.k/2)+vebpred(B,j);
    i = vebpred(A,i-1);
    if (i == A.M)
        return lo;
    B = branch(T,i);
    return i*ipow(T.k/2)+high(B);
}
