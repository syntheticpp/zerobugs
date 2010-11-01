struct Bits
{
    bool flag : 1;
    unsigned char size : 7;
};
struct MoreBits
{
    unsigned char magic;
    unsigned int  data : 24;
};


int main()
{
    Bits b;
    MoreBits moreBits;

    b.flag = true;
    b.size = 9;

    moreBits.magic = 0xff;
    moreBits.data  = 0x42;

    return 0;
}
