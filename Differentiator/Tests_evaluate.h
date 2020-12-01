TEST_DEFINE(
    "x^0",
    TEST_SIMPLIFY_1,
    {
        return 1;
    },
    0,
    100,
    10
)


TEST_DEFINE(
    "x^1",
    TEST_SIMPLIFY_2,
    {
        return x;
    },
    0,
    100,
    10
)


TEST_DEFINE(
    "0^x",
    TEST_SIMPLIFY_3,
    {
        return 0;
    },
    0,
    100,
    10
)


TEST_DEFINE(
    "1^x",
    TEST_SIMPLIFY_4,
    {
        return 1;
    },
    0,
    100,
    10
)


TEST_DEFINE(
    "0+x",
    TEST_SIMPLIFY_5,
    {
        return x;
    },
    0,
    100,
    10
)


TEST_DEFINE(
    "x*0",
    TEST_SIMPLIFY_6,
    {
        return 0;
    },
    0,
    100,
    10
)


TEST_DEFINE(
    "x*1",
    TEST_SIMPLIFY_7,
    {
        return x;
    },
    0,
    100,
    10
)


TEST_DEFINE(
    "sin(x)",
    sin,
    {
        return sinf(x);
    },
    0,
    100,
    0.1
)

TEST_DEFINE(
    "cos(x)",
    cos,
    {
        return cosf(x);
    },
    0,
    100,
    0.1
)

TEST_DEFINE(
    "sqrt(x)",
    sqrt,
    {
        return sqrt(x);
    },
    0,
    100,
    1.5
)

TEST_DEFINE(
    "sqrt(x^2+sin(x))/ln(x^(17.5/3.58))",
    test_1,
    {
        return sqrt(x*x + sinf(x)) / log(pow(x ,(17.5 / 3.58)));
    },
    1,
    500,
    1
)