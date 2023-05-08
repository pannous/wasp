//  'powl' is a builtin with type 'long double (long double, long double)'
long long pow_long(long long base,long long exp)
{
    long long result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}
int main(int argc, char const *argv[])
{
    
    return (int) pow_long(argc,(long)argv);
}