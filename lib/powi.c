long long powi(int base, int exp)
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
    return powi(argc,(int)argv);
}