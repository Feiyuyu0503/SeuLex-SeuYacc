int main()
{
    /*just
    a 
    test
    */
    int x;
    x = 9;
    int y;
    y = 0x1eU;
    float z;
    z = 0.123f;
    // line comment
    if (x == 0)
    {
        y = y-1;
        z = y+1;
    }
    else
    { 
        y = y*2;
        z = y-5.3;
    }
    return 0;
}