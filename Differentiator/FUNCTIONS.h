
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
    {
        d(node->link[0]);
    }
)

FUNC_DEFINE(
	sin,
	FUNC_SIN,
	{
		return sinf(left);
    },
    {
        //sin(f) -> cos(f) * f'
        
        Expression::TNode* y;
        y = c(node);
        node->ptrToData->type = NODE_TYPE_OPERATION;
        node->ptrToData->data.opNumber = OP_MUL;
        y->ptrToData->data.opNumber = FUNC_COS;
        node->link[1] = y;

        setLinkParent(node);
        setLinkParent(node->link[0]);
        setLinkParent(node->link[1]);

        d(node->link[0]);
    }
)


FUNC_DEFINE(
	cos,
	FUNC_COS,
	{
		return cosf(left);
	},
	{
        //cos(f) -> -sin(f) * f'
        
        Expression::TNode* y;
        y = c(node);
        node->ptrToData->type = NODE_TYPE_OPERATION;
        node->ptrToData->data.opNumber = OP_MUL;
        y->ptrToData->data.opNumber = FUNC_SIN;
        node->link[1] = createNode(y,NULL,NODE_TYPE_FUNCTION,FUNC_NEG,NULL);

        setLinkParent(node);
        setLinkParent(node->link[0]);
        setLinkParent(node->link[1]);

        d(node->link[0]);
    }
)

FUNC_DEFINE(
	tan,
	FUNC_TAN,
	{
		return tanf(left);
	},
	{
        //tg f -> 1/cos^2(f) f'
        
        
        Expression::TNode *f;
        f = c(node->link[0]);
        node->ptrToData->type = NODE_TYPE_OPERATION;
        node->ptrToData->data.opNumber = OP_DIV;

        node->link[1] = createNode(
            createNode(f, NULL, NODE_TYPE_FUNCTION, FUNC_COS, NULL),
            createNode(NULL, NULL, NODE_TYPE_NUMBER, 2.0, NULL),
            NODE_TYPE_OPERATION,
            OP_POW,
            node
        );

        setLinkParent(node);
        setLinkParent(node->link[0]);
        setLinkParent(node->link[1]);
        d(node->link[0]);
        
    }
)

FUNC_DEFINE(
	cot,
	FUNC_COT,
	{
		return 1.0/tanf(left);
	},
	{
        //ctg f -> -1/sin^2(f) f'
        
        Expression::TNode *f;
        f = c(node->link[0]);
        node->ptrToData->type = NODE_TYPE_OPERATION;
        node->ptrToData->data.opNumber = OP_DIV;

        node->link[0] = createNode(
            node->link[0],
            NULL,
            NODE_TYPE_FUNCTION,
            FUNC_NEG,
            node
        );

        node->link[1] = createNode(
            createNode(f, NULL, NODE_TYPE_FUNCTION, FUNC_SIN, NULL),
            createNode(NULL, NULL, NODE_TYPE_NUMBER, 2.0, NULL),
            NODE_TYPE_OPERATION,
            OP_POW,
            node
        );

        setLinkParent(node);
        setLinkParent(node->link[0]);
        setLinkParent(node->link[1]);
        d(node->link[0]);
       
    }
)

FUNC_DEFINE(
	ln,
	FUNC_LN,
	{
		return log(left);
	},
	{
        
        Expression::TNode *f;
        Expression::TNode *df;
        f = node->link[0];
        df = c(f);
        d(df);


        node->ptrToData->type = NODE_TYPE_OPERATION;
        node->ptrToData->data.opNumber = OP_DIV;

        node->link[1] = f;
        node->link[0] = df;

        setLinkParent(node);
        setLinkParent(node->link[0]);
        setLinkParent(node->link[1]);
    }
)

FUNC_DEFINE(
	sqrt,
	FUNC_SQRT,
	{
		return sqrt(left);
	},
    {
        Expression::TNode *f;
        Expression::TNode *df;
        f = node->link[0];
        df = c(f);
        d(df);
        f = c(node);


        node->ptrToData->type = NODE_TYPE_OPERATION;
        node->ptrToData->data.opNumber = OP_DIV;

        node->link[1] = createNode(
            createNode(NULL, NULL, NODE_TYPE_NUMBER, 2.0, NULL),
            f,
            NODE_TYPE_OPERATION,
            OP_MUL,
            node
        );
        node->link[0] = df;

        setLinkParent(node);
        setLinkParent(node->link[0]);
        setLinkParent(node->link[1]);
    }
)

FUNC_DEFINE(
	exp,
	FUNC_EXP,
	{
		return exp(left);
	},
	{
        Expression::TNode *f;
        Expression::TNode *df;
        f = node->link[0];
        df = c(f);
        d(df);


        node->ptrToData->type = NODE_TYPE_OPERATION;
        node->ptrToData->data.opNumber = OP_MUL;

        node->link[1] = f;
        node->link[0] = df;

        setLinkParent(node);
        setLinkParent(node->link[0]);
        setLinkParent(node->link[1]);
    }
)
