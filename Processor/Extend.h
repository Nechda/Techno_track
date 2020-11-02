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
         bytes:            10                     2                       2                 2
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



DEF(
    MOV,
    1 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);

        *dst = *src;
    }
)

DEF(
    ADD,
    2 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
        if (!CPU.isFloatPointMath)
            *dst += *src;
        else
            *((float*)(dst)) += *((float*)(src));
    }
)

DEF(
    SUB,
    3 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);

        if (!CPU.isFloatPointMath)
            *dst -= *src;
        else
            *((float*)(dst)) -= *((float*)(src));
    }
)

DEF(
    DIV,
    4 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);



        if (!CPU.isFloatPointMath)
        {
            if (*src == 0)
                CPU.interruptCode = 1; // при делении на ноль, возникает прерывание
            else
                *dst /= *src;
        }
        else
        {
            float divisitor = *((float*)src);
            if (isZero(divisitor))
                CPU.interruptCode = 1; // при делении на ноль, возникает прерывание
            else
                *((float*)dst) /= divisitor;
        }
    }
)

DEF(
    MUL,
    5 << 6 | 0 << 4 | 0 << 2 | 0x2, "RMB", "RNMB",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);

        if (!CPU.isFloatPointMath)
            *dst *= *src;
        else
        {
            float mul = *((float*)src);
            *((float*)(dst)) *= mul;
        }
    }
)

DEF(
    POP,
    6 << 6 | 0 << 4 | 0 << 2 | 0x1, "RMB", "",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
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
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);
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
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    HLT,
    9 << 6 | 0 << 4 | 0 << 2 | 0x0, "", "",
    {}
)

DEF(
    CMP,
    10 << 6 | 0 << 4 | 0 << 2 | 0x2, "RNMB", "RNMB",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);

        if (!CPU.isFloatPointMath)
        {
            ui32 result = *dst - *src;

            setBit(&CPU.Register.efl, FLAG_CF, result >> (sizeof(ui32) * 8 - 1));
            setBit(&CPU.Register.efl, FLAG_ZF, result == 0 ? 1 : 0);
            setBit(&CPU.Register.efl, FLAG_SF, (int)result >= 0 ? 0 : 1);
        }
        else
        {
            float result = *(float*)dst - *(float*)src;

            setBit(&CPU.Register.efl, FLAG_CF, (int)result >> (sizeof(ui32) * 8 - 1));
            setBit(&CPU.Register.efl, FLAG_ZF, isZero(result));
            setBit(&CPU.Register.efl, FLAG_SF, result >= 0 ? 0 : 1);
        }
    }
)

DEF(
    JE,
    11 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    JNE,
    12 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (!getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    JA,
    13 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (!getBit(CPU.Register.efl, FLAG_CF) && !getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    JAE,
    14 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (!getBit(CPU.Register.efl, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    JB,
    15 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (getBit(CPU.Register.efl, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    JBE,
    16 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        if (getBit(CPU.Register.efl, FLAG_CF) || getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    CALL,
    17 << 6 | 0 << 4 | 0 << 2 | 0x1, "N", "",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);

        ui8* data = (ui8*)&CPU.Register.eip;
        for (ui8 i = 0; i < sizeof(ui32); i++)
            stackPush(&CPU.stack, &data[i]);
        CPU.Register.esp += sizeof(ui32);
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
    }
)

DEF(
    RET,
    18 << 6 | 0 << 4 | 0 << 0 | 0x0, "", "",
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
    19 << 6 | 0 << 4 | 0 << 0 | 0x0, "", "",
    {
        CPU.isFloatPointMath = 1;
    }
)

DEF(
    FPMOFF,
    20 << 6 | 0 << 4 | 0 << 0 | 0x0, "", "",
    {
        CPU.isFloatPointMath = 0;
    }
)


DEF(
    SQRT,
    21 << 6 | 0 << 4 | 0 << 0 | 0x1, "RNMB", "",
    {
        ui32* dst = NULL;
        ui32* src = NULL;
        getOperandsPointer(cmd, &dst, &src);

        if (dst < 0)
        {
            CPU.interruptCode = 2;
            return;
        }
        *((float*)&CPU.Register.eax) = sqrt(*((float*)dst));
    }
)
