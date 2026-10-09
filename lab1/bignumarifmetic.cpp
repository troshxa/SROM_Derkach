#include "bignumarifmetic.h"

uint32_t LongAdd(const BigNum& a, const BigNum& b, BigNum& c){
    uint32_t carry = 0;
    uint64_t temp = 0;
    for (int i = 0; i < WORDS; i++){
        temp = (uint64_t)a.d[i] +b.d[i] + carry;
        c.d[i] = temp & MASK;
        carry = temp >> W;
    }
    return carry;
}

uint32_t LongSub(const BigNum& a, const BigNum& b, BigNum& c){
    uint32_t borrow = 0;
    int64_t temp = 0;
    for(int i = 0; i <WORDS; i++){
        temp = (int64_t)a.d[i] - b.d[i] - borrow;
        if(temp>=0){
            c.d[i] = (uint32_t)temp;
            borrow = 0;
        }else{
            c.d[i] = (uint32_t)(BASE + temp);
            borrow = 1;
        }
    }
    return borrow;
}

int LongCmp(const BigNum& a, const BigNum& b){
    int i = WORDS-1;
    while(i >=0 && a.d[i] == b.d[i]){
        i-=1;
    }
    if(i==-1) return 0;
    else{
        if(a.d[i]>b.d[i]) return 1; 
        else return -1;
    }

}

BigNum ToBigNum(uint64_t x){
    BigNum a ={};
    a.d[0] = x & MASK;
    a.d[1] = x >> W;
    return a;
}

uint32_t LongMulOneDigit(const BigNum& a, uint32_t b, BigNum& c){
    uint32_t carry =0;
    uint64_t temp = 0;
    for(int i =0; i < WORDS; i++){
        temp = (uint64_t)a.d[i]*b + carry;
        c.d[i] = temp & MASK;
        carry = temp >> W;
    }
    return carry;
}

BigNum LongShiftDigitsToHigh(const BigNum& a, int x){
    BigNum c = {};
    if (x >= WORDS) return c;
    for(int i = x; i < WORDS; i++){
        c.d[i] = a.d[i-x];
    }
    return c;
}

BigNum LongShiftDigitsToLow(const BigNum& a, int x){
    BigNum c = {};
    if (x >= WORDS) return c;
    for(int i = 0; i < WORDS-x; i++){
        c.d[i] = a.d[i+x];
    }
    return c;
}

BigNum LongShiftBitsToHigh(const BigNum& a, int x){
    int digits = x / W;
    int bits = x % W;
    BigNum c = LongShiftDigitsToHigh(a, digits);
    uint32_t tail = 0;
    uint64_t temp =0;
    for(int i =0; i <WORDS; i++){
        temp = (uint64_t)c.d[i] <<bits;
        c.d[i] = (temp & MASK) | tail;
        tail = temp >> W;
    }
    return c;
}

BigNum LongShiftBitsToLow(const BigNum& a, int x){
    int digits = x / W;
    int bits = x % W;
    BigNum c = LongShiftDigitsToLow(a, digits);
    uint32_t tail = 0;
    uint64_t temp =0;
    for(int i =WORDS-1; i >=0; i --){
        temp = ((uint64_t)c.d[i]<<W) >>bits;
        c.d[i] = (temp >> W) | tail;
        tail = temp & MASK;
    }
    return c;
}

void LongMul(const BigNum& a, const BigNum& b, BigNum& c){
    BigNum tempC = {};
    BigNum temp = {};
    for(int i =0; i < WORDS; i++){
        LongMulOneDigit(a,b.d[i],temp);
        LongAdd(tempC, LongShiftDigitsToHigh(temp,i), tempC);
    }
    c = tempC;
}

void LongSquare(const BigNum& a, BigNum& b) {
    BigNum t = {};
    BigNum d = {};
    uint32_t carry;
    uint64_t temp = 0;

    for(int i =0; i <WORDS; i++){
        carry=0;
        for(int j = i + 1; i + j < WORDS; j++){
            temp = (uint64_t)t.d[i + j] + (uint64_t)a.d[i] * a.d[j] + carry;
            t.d[i + j] = temp & MASK;
            carry = temp >> W;
        }
    }

    t = LongShiftBitsToHigh(t, 1);

    for(int i = 0; 2 * i < WORDS; i++){
        temp = (uint64_t)a.d[i] * a.d[i];
        d.d[2 * i] = temp & MASK;
        d.d[2 * i + 1] = temp >> W;
    }

    LongAdd(t, d, b);
}

int BitLength(const BigNum& a){

    for(int i = WORDS-1; i >=0; i--){
        if(a.d[i]!=0){
            uint32_t v = a.d[i];
            int bits = 0;
            while (v!=0){
                v = v>>1;
                bits++;
            }
            return i*W +bits;
        }
    }
    return 0;
}

bool LongDivMod(const BigNum& a,const BigNum& b,BigNum& q,BigNum& r){
    int k = BitLength(b);
    if (k ==0){
        q = {};
        r = {};
        return false;
    }
    int t ;
    int s;
    BigNum c;
    BigNum tempR = a;
    BigNum tempQ = {};
    while(LongCmp(tempR,b)!=-1){
        t = BitLength(tempR);
        c = LongShiftBitsToHigh(b, t-k);
        if (LongCmp(c,tempR)==1){
            t--;
            c = LongShiftBitsToHigh(b,t-k);
        }
        LongSub(tempR,c,tempR);
        s = t-k;
        tempQ.d[s/W] |= 1u << (s%W);
    }
    q = tempQ;
    r = tempR;
    return true;
}

void LongPower(const BigNum& a,const BigNum& b,BigNum& c){
    BigNum tempC = {};
    tempC.d[0] = 1;
    for(int i = BitLength(b)-1; i >=0; i--){
        if ((b.d[i / W] >> (i % W)) & 1){
            LongMul(tempC, a, tempC);
        }
        if(i!=0){
            LongSquare(tempC,tempC);
        }
    }
    c = tempC;
}

static int CharValue(char ch, int bitsPerChar){
    int v = -1;
    if (ch >= '0' && ch <= '9') v = ch - '0';
    else if (ch >= 'a' && ch <= 'f') v = ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F') v = ch - 'A' + 10;
    if (v >= (1 << bitsPerChar)) return -1;
    return v;
}

static bool FromPow2String(const std::string& s, int bitsPerChar, BigNum& a){
    if (s.empty()) return false;
    BigNum t = {};
    int pos = 0;                                 
    for (size_t k = s.size(); k > 0; k--){      
        int v = CharValue(s[k - 1], bitsPerChar);
        if (v < 0) return false;
        if (pos >= BITS){                         
            if (v != 0) return false;             
            continue;
        }
        t.d[pos / W] |= (uint32_t)v << (pos % W);
        pos += bitsPerChar;
    }
    a = t;
    return true;
}

static std::string ToPow2String(const BigNum& a, int bitsPerChar){
    const char* digits = "0123456789ABCDEF";
    int n = BitLength(a);
    if (n == 0) return "0";
    int chars = (n + bitsPerChar - 1) / bitsPerChar;
    uint32_t mask = (1u << bitsPerChar) - 1;
    std::string s;
    for (int pos = (chars - 1) * bitsPerChar; pos >= 0; pos -= bitsPerChar){
        s += digits[(a.d[pos / W] >> (pos % W)) & mask];
    }
    return s;
}

bool FromHex(const std::string& s, BigNum& a){
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return FromPow2String(s.substr(2), 4, a);
    return FromPow2String(s, 4, a);
}

std::string ToHex(const BigNum& a){
    return ToPow2String(a, 4);
}

bool FromBin(const std::string& s, BigNum& a){
    if (s.size() >= 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B'))
        return FromPow2String(s.substr(2), 1, a);
    return FromPow2String(s, 1, a);
}

std::string ToBin(const BigNum& a){
    return ToPow2String(a, 1);
}
