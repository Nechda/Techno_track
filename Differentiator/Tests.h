
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