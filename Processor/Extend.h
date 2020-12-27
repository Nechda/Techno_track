/**
\file    В данном фалй описываются команды языка ассмеблера, а так же их реализации применительно
         к эмуляции на виртуальном процессоре.
\details Для задания новой команды требудется вызвать макрос, прототип которого имеет вид:
         #define DEF(name, machineCode, validStrOperand_1, validStrOperand_2, code)
         name --- имя команды, указывается ЗАГЛАВНЫМИ буквами
         machineCode --- машинный код команды, правило формирования кода указано ниже
         validStrOperand_1 --- строка допустимых типов первого операнда, правило формирование смотри ниже
         validStrOperand_2 --- строка допустимых типов второго операнда, правило формирование смотри ниже
         code --- код функции, реализующий действие команды на процессоре

         Рассмотрим пример раскрытия макроса как функции:
         DEF(
             JMP,
             8 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
             {
                CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
             }
         )
         будет создавать функцию, которая описывается следующим кодом:
         void run_JMP(Command cmd)
         {
            CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
         }


         Структура кодирования команд в машинном коде:
         bytes:            10                         2                       2                    2
         description: command general code | type of second operand | type of first operand  | nOperands

         Кодирование типов операндов в машинном коде:
         0b00 --- operand is register
         0b01 --- operand is number
         0b10 --- operand is memory, based by number
         0b11 --- operand is memory, based by register


         Типы символов в строках validOperand:
         R --- operand is Register
         N --- operand is Number
         M --- operand is Memory, based by number
         B --- operand is memory, Based by register
         Пример:
         Для команды mov строка validOperand будет равна "RMB" для первого операнда и "RNMB" для второго операнда.
         Такое кодирование означает, что команда mov не может записать данные в число.

*/

#define isInterruptOccur() if(CPU.interruptCode) return;

DEF(
    HLT,
    0 << 6 | 0 << 4 | 0 << 2 | 0x0, "", "",
    {}
)

DEF(
    MOV,
    1 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        *dst = *src;
    }
)

DEF(
    ADD,
    2 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();


        if (!CPU.isFloatPointMath)
            dst->ivalue += src->ivalue;
        else
            dst->fvalue += src->fvalue;
    }
)

DEF(
    SUB,
    3 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();
        if (!CPU.isFloatPointMath)
            dst->ivalue -= src->ivalue;
        else
            dst->fvalue -= src->fvalue;
    }
)

DEF(
    DIV,
    4 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();


        if (!CPU.isFloatPointMath)
        {
            if (src->ivalue == 0)
                CPU.interruptCode = 1; // при делении на ноль, возникает прерывание
            else
                dst->ivalue /= src->ivalue;
        }
        else
        {
            if (isZero(src->ivalue))
                CPU.interruptCode = 1; // при делении на ноль, возникает прерывание
            else
                dst->fvalue /= src->fvalue;
        }
    }
)

DEF(
    MUL,
    5 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        if (!CPU.isFloatPointMath)
            dst->ivalue *= src->ivalue;
        else
            dst->fvalue *= src->fvalue;
    }
)

DEF(
    POP,
    6 << 6 | 0 << 4 | 0 << 2 | 0x1, "RMB", "",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        ui8* data = (ui8*)dst;
        for (ui8 i = 0; i < sizeof(ui32); i++)
            stackPop(&CPU.stack, &data[sizeof(ui32) - 1 - i]);

        //в данной реализации процессора в качестве стека используется immortal stack,
        //соттветственно при push он растет в сторону больших адресов,
        //а при pop адрес вершины уменьшается
        CPU.Register.esp -= sizeof(ui32);
    }
)

DEF(
    PUSH,
    7 << 6 | 0 << 4 | 0 << 2 | 0x1, "RNMB", "",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();
        ui8* data = (ui8*)dst;
        for (ui8 i = 0; i < sizeof(ui32); i++)
            stackPush(&CPU.stack, &data[i]);

        //в данной реализации процессора в качестве стека используется immortal stack,
        //соттветственно при push он растет в сторону больших адресов,
        //а при pop адрес вершины уменьшается
        CPU.Register.esp += sizeof(ui32);
    }
)

DEF(
    JMP,
    8 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)


DEF(
    CMP,
    9 << 6 | 0 << 4 | 0 << 2 | 0x2, "RNMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        OperandUnion result;

        if (!CPU.isFloatPointMath)
            result.ivalue = dst->ivalue - src->ivalue;
        else
            result.fvalue = dst->fvalue - src->fvalue;

        setBit(&CPU.Register.efl, FLAG_CF, result.ivalue >> (sizeof(ui32) * 8 - 1));

        //printf("%d <-> %f => CF == %d\n", result.ivalue, result.fvalue, result.ivalue >> (sizeof(ui32) * 8 - 1) & 1);

        if (!CPU.isFloatPointMath)
        {
            setBit(&CPU.Register.efl, FLAG_ZF, result.ivalue == 0 ? 1 : 0);
            setBit(&CPU.Register.efl, FLAG_SF, result.ivalue >= 0 ? 0 : 1);
        }
        else
        {
            setBit(&CPU.Register.efl, FLAG_ZF, isZero(result.fvalue));
            setBit(&CPU.Register.efl, FLAG_SF, result.fvalue >= 0 ? 0 : 1);
        }
    }
)

DEF(
    JE,
    10 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)

DEF(
    JNE,
    11 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (!getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)

DEF(
    JA,
    12 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (!getBit(CPU.Register.efl, FLAG_CF) && !getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)

DEF(
    JAE,
    13 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (!getBit(CPU.Register.efl, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)

DEF(
    JB,
    14 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (getBit(CPU.Register.efl, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)

DEF(
    JBE,
    15 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (getBit(CPU.Register.efl, FLAG_CF) || getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)

DEF(
    CALL,
    16 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        ui8* data = (ui8*)&CPU.Register.eip;
        for (ui8 i = 0; i < sizeof(ui32); i++)
            stackPush(&CPU.stack, &data[i]);
        CPU.Register.esp += sizeof(ui32);
        CPU.Register.eip = CPU.Register.ecs + cmd.operand[0].ivalue;
    }
)

DEF(
    RET,
    17 << 6 | 0 << 4 | 0 << 2 | 0x0, "", "",
    {
        ui32 ptrReturn = 0;

        ui8* data = (ui8*)&ptrReturn;
        for (ui8 i = 0; i < sizeof(ui32); i++)
            stackPop(&CPU.stack, &data[sizeof(ui32) - 1 - i]);
        CPU.Register.esp -= sizeof(ui32);
        CPU.Register.eip = ptrReturn;
    }
)

DEF(
    FPMON,
    18 << 6 | 0 << 4 | 0 << 2 | 0x0, "", "",
    {
        CPU.isFloatPointMath = 1;
    }
)

DEF(
    FPMOFF,
    19 << 6 | 0 << 4 | 0 << 2 | 0x0, "", "",
    {
        CPU.isFloatPointMath = 0;
    }
)


DEF(
    SQRT,
    20 << 6 | 0 << 4 | 0 << 2 | 0x1, "RNMB", "",
    {
        OperandUnion* dst = (OperandUnion*)&CPU.Register.eax;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &src, &dst);
        isInterruptOccur();

        if (src->fvalue < 0 || !CPU.isFloatPointMath)
        {
            CPU.interruptCode = 3; // извлечение корня из отрицательного числа
            return;
        }
        dst->fvalue = sqrt(src->fvalue);
    }
)


DEF(
    TRUNC,
    21 << 6 | 0 << 4 | 0 << 2 | 0x1, "RMB", "",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        dst->ivalue = static_cast<ui32>(dst->fvalue);
    }
)


DEF(
    SIN,
    22 << 6 | 0 << 4 | 0 << 2 | 0x1, "RNMB", "",
    {
        OperandUnion* dst = (OperandUnion*)&CPU.Register.eax;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &src, &dst);
        isInterruptOccur();

        if(!CPU.isFloatPointMath)
        {
            CPU.interruptCode = 3; // должна быть включена арифметика с плавающей точкой
            return;
        }

        dst->fvalue = sinf(src->fvalue);
    }
)


DEF(
    COS,
    23 << 6 | 0 << 4 | 0 << 2 | 0x1, "RNMB", "",
    {
        OperandUnion* dst = (OperandUnion*)&CPU.Register.eax;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &src, &dst);
        isInterruptOccur();

        if(!CPU.isFloatPointMath)
        {
            CPU.interruptCode = 3; // должна быть включена арифметика с плавающей точкой
            return;
        }

        dst->fvalue = cosf(src->fvalue);
    }
)


DEF(
    MOVB,
    24 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        ui8 srcB = src->ivalue;
        memcpy(dst, &srcB, sizeof(ui8));
        
    }
)

DEF(
    MOVW,
    25 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();
        ui16 srcW = src->ivalue;
        memcpy(dst, &srcW, sizeof(ui16));

    }
)

DEF(
    DUMP,
    26 << 6 | 0 << 4 | 0 << 2 | 0x0, "", "",
    {
        cpuDump(stdout);
        system("pause");
    }
)

DEF(
    FLOAT,
    27 << 6 | 0 << 4 | 0 << 2 | 0x1, "RMB", "",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        dst->fvalue = static_cast<float>(dst->ivalue);
    }
)

DEF(
    OUT,
    28 << 6 | 0 << 4 | 0 << 2 | 0x1, "RNMB", "",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();


        if (CPU.isFloatPointMath)
            printf("%f\n", dst->fvalue);
        else
            printf("%d (0x%X)\n", static_cast<i32>(dst->ivalue), dst->ivalue);
    }
)

DEF(
    IN,
    29 << 6 | 0 << 4 | 0 << 2 | 0x1, "RMB", "",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();


        if (CPU.isFloatPointMath)
            scanf("%f", &dst->fvalue);
        else
            scanf("%d", &dst->ivalue);
    }
)

DEF(
    OR,
    30 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        if (!CPU.isFloatPointMath)
            dst->ivalue = dst->ivalue || src->ivalue;
        else
            dst->fvalue = isZero(dst->fvalue) || isZero(src->fvalue) ? 1.0 : 0.0;
    }
)

DEF(
    AND,
    31 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        if (!CPU.isFloatPointMath)
            dst->ivalue = dst->ivalue && src->ivalue;
        else
            dst->fvalue = !isZero(dst->fvalue) && !isZero(src->fvalue) ? 1.0 : 0.0;
    }
)

DEF(
    ABS,
    32 << 6 | 0 << 4 | 0 << 2 | 0x1, "RNMB", "",
    {
        OperandUnion* dst = (OperandUnion*)&CPU.Register.eax;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &src, &dst);
        isInterruptOccur();

        if(!CPU.isFloatPointMath)
            dst->ivalue = static_cast<ui32>(abs(static_cast<i32>(src->ivalue)));
        else
            dst->fvalue = abs(src->fvalue);
    }
)

DEF(
    POW,
    33 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        OperandUnion* dst = NULL;
        OperandUnion* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        isInterruptOccur();

        if(!CPU.isFloatPointMath)
        {
            CPU.interruptCode = 3; // должна быть включена арифметика с плавающей точкой
            return;
        }

        dst->fvalue = powf(dst->fvalue, src->fvalue);
    }
)