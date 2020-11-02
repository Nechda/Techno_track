DEF(
    FPMON,
    19 << 6 | 0 << 4 | 0 << 0 | 0x0,
    "", "",
    {
        CPU.isFloatPointMath = 1;
    }
)

DEF(
    FPMOFF,
    20 << 6 | 0 << 4 | 0 << 0 | 0x0,
    "", "",
    {
        CPU.isFloatPointMath = 0;
    }
)


DEF(
    SQRT,
    21 << 6 | 0 << 4 | 0 << 0 | 0x1,
    "RNMB", "",
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
