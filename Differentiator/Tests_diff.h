TEST_DEFINE(
    "sin(x)",
    d_test_1,
    {
        return cosf(x);
    },
    0,
    50,
    0.1
)

TEST_DEFINE(
    "cos(x)",
    d_test_2,
    {
        return -sinf(x);
    },
    0,
    50,
    0.1
)

TEST_DEFINE(
    "x^2",
    d_test_3,
    {
        return 2*x;
    },
    0,
    50,
    0.1
)

TEST_DEFINE(
    "ln(x^3.14)",
    d_test_4,
    {
        return 3.14/x;
    },
    1, 10, 0.1
)

TEST_DEFINE(
    "sqrt(x)/(1+x^2)",
    d_test_5,
    {
        return (1-3*x*x)/(2*sqrt(x)*pow(1+x*x,2));
    },
    0.2,
    50,
    0.3
)

TEST_DEFINE(
    "sqrt(2 + x^2 + sin(x))/ln(x^3)",
    d_test_6,
    {
        return (2 * x + cosf(x)) / (2 * log(pow(x,3)) * sqrt(x*x + 2 + sinf(x))) - (3 * sqrt(x*x + 2 + sinf(x))) / (x * pow(log(pow(x,3)) ,2));
    },
    2,
    100,
    0.25
)