//============определение стандартных функций===============
FUNC_DEFINE(
    print,
    FUNC_PRINT,
    {
        printf("%lf\n",left);
        return 0;
    },
    false,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("out eax");
    }
)

FUNC_DEFINE(
    input,
    FUNC_INPUT,
    {
        double res = 0;
        scanf("%lf", &res);
        return res;
    },
    false,
    {
        pushLine("fin eax");
        pushLine("push eax");
    }
)

FUNC_DEFINE(
    neg,
    FUNC_NEG,
    {
        return -left;
    },
    true,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("fmul eax, -1.0");
        pushLine("push eax");
    }
)

FUNC_DEFINE(
	sin,
	FUNC_SIN,
	{
		return sinf(left);
    },
    true,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("fsin eax");
        pushLine("push eax");
    }
)


FUNC_DEFINE(
	cos,
	FUNC_COS,
	{
		return cosf(left);
	},
    true,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("fcos eax");
        pushLine("push eax");
    }
)

FUNC_DEFINE(
	tan,
	FUNC_TAN,
	{
		return tanf(left);
	},
    true,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("mov ebx, eax");
        pushLine("fsin eax");
        pushLine("mov esi, eax");
        pushLine("fcos ebx");
        pushLine("mov edi, eax");
        pushLine("fdiv esi, edi");
        pushLine("mov eax, esi");
        pushLine("push eax");
    }
)

FUNC_DEFINE(
	cot,
	FUNC_COT,
	{
		return 1.0/tanf(left);
	},
    true,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("mov ebx, eax");
        pushLine("fcos eax");
        pushLine("mov esi, eax");
        pushLine("fsin ebx");
        pushLine("mov edi, eax");
        pushLine("fdiv esi, edi");
        pushLine("mov eax, esi");
        pushLine("push eax");
    }
)

FUNC_DEFINE(
	ln,
	FUNC_LN,
	{
		return log(left);
	},
    true,
    {
        Assert_c("This function doesn't implement");
        return;
    }
)

FUNC_DEFINE(
	sqrt,
	FUNC_SQRT,
	{
		return sqrt(left);
	},
    true,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("fsqrt eax");
        pushLine("push eax");
    }
)

FUNC_DEFINE(
	exp,
	FUNC_EXP,
	{
		return exp(left);
	},
    true,
    {
        Assert_c("This function doesn't implement");
        return;
    }
)

FUNC_DEFINE(
    abs,
    FUNC_ABS,
    {
        return left > 0 ? left : -left;
    },
    true,
    {
        genAsmByTree(node->link[0], asmInfo);
        pushLine("pop eax");
        pushLine("fabs eax");
        pushLine("push eax");
    }
)