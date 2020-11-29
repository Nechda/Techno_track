
OP_DEFINE(
	'+',
    sum,
	OP_SUM,
	2,
	{
		return left+right;
	},
    {}
)

OP_DEFINE(
	'-',
    sub,
	OP_SUB,
	2,
	{
		return left-right;
	},
    {}
)

OP_DEFINE(
	'*',
    mul,
	OP_MUL,
	1,
	{
		return left*right;
	},
    {}
)

OP_DEFINE(
	'+',
    div,
	OP_DIV,
	1,
	{
		return left/right;
	},
    {}
)

OP_DEFINE(
	'^',
    pow,
	OP_POW,
	1,
	{
		return powf(left,right);
	},
    {}
)



//============функции===============

FUNC_DEFINE(
    neg,
    FUNC_NEG,
    {
        return -left;
    },
    {}
        )

FUNC_DEFINE(
	sin,
	FUNC_SIN,
	{
		return sinf(left);
	},
	{}
)

FUNC_DEFINE(
	cos,
	FUNC_COS,
	{
		return cosf(left);
	},
	{}
)

FUNC_DEFINE(
	tan,
	FUNC_TAN,
	{
		return tanf(left);
	},
	{}
)

FUNC_DEFINE(
	cot,
	FUNC_COT,
	{
		return 1.0/tanf(left);
	},
	{}
)

FUNC_DEFINE(
	ln,
	FUNC_LN,
	{
		return log(left);
	},
	{}
)

FUNC_DEFINE(
	sqrt,
	FUNC_SQRT,
	{
		return sqrt(left);
	},
	{
	//тут реализация специфического вывода
	}
)

FUNC_DEFINE(
	exp,
	FUNC_EXP,
	{
		return exp(left);
	},
	{}
)