TEST_DEFINE(
    "sin(x)",
    1,
    "Ok"
)

TEST_DEFINE(
    "sin(x^2+1.0 / 2)*sqrt(x/ln(x-0))",
    1,
    "Ok"
)

TEST_DEFINE(
    "(2*x+cos(x))/(2*ln(x^3)*sqrt(x^2 + sin(x)))",
    1,
    "Ok"
)

TEST_DEFINE(
    "(2*xcos(x))/(2*ln(x^3)*sqrt(x^2 + sin(x)))",
    0,
    "Missing +"
)


TEST_DEFINE(
    "(2*x+cos(x))/(2*ln(x^3)*sqrt(x^2 + sin(x))",
    0,
    "Missing ) at the end of expression"
)

TEST_DEFINE(
    "(2*x+cos(x))/(2*ln(x^3)*sqrt(x^2 +-sin(x)))",
    0,
    "Two operators place near -> \'+-\'"
)

TEST_DEFINE(
    "(2*x+cos(t))/(2*ln(x^3)*sqrt(x^2 + sin(x)))",
    0,
    "Undefined lexema t"
)

TEST_DEFINE(
    "(2*x+cos(0.3x))/(2*ln(x^3)*sqrt(x^2 + sin(x)))",
    0,
    "Missing operator 0.3x"
)

TEST_DEFINE(
    "(2*x+cos(0.303.0))/(2*ln(x^3)*sqrt(x^2 + sin(x)))",
    0,
    "Invalid structure of number 0.303.0"
)