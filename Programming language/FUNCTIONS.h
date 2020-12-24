//============определение стандартных функций===============
FUNC_DEFINE(
    print,
    FUNC_PRINT,
    {
        printf("%lf\n",left);
        return 0;
    },
    false
)

FUNC_DEFINE(
    input,
    FUNC_INPUT,
    {
        double res = 0;
        scanf("%lf", &res);
        return res;
    },
    false
)

FUNC_DEFINE(
    neg,
    FUNC_NEG,
    {
        return -left;
    },
    true
)

FUNC_DEFINE(
	sin,
	FUNC_SIN,
	{
		return sinf(left);
    },
    true
)


FUNC_DEFINE(
	cos,
	FUNC_COS,
	{
		return cosf(left);
	},
    true
)

FUNC_DEFINE(
	tan,
	FUNC_TAN,
	{
		return tanf(left);
	},
    true
)

FUNC_DEFINE(
	cot,
	FUNC_COT,
	{
		return 1.0/tanf(left);
	},
    true
)

FUNC_DEFINE(
	ln,
	FUNC_LN,
	{
		return log(left);
	},
    true
)

FUNC_DEFINE(
	sqrt,
	FUNC_SQRT,
	{
		return sqrt(left);
	},
    true
)

FUNC_DEFINE(
	exp,
	FUNC_EXP,
	{
		return exp(left);
	},
    true
)

FUNC_DEFINE(
    abs,
    FUNC_ABS,
    {
        return left > 0 ? left : -left;
    },
    true
)