#define BOOST_TEST_MAIN
#if !defined(WINDOWS)
    #define BOOST_TEST_DYN_LINK
#endif
#include <boost/test/auto_unit_test.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <lc3_all.hpp>
#include "ExpressionEvaluator.hpp"

struct LC3Test
{
    lc3_state state;

    LC3Test()
    {
        lc3_init(state, false);
    }
};

BOOST_FIXTURE_TEST_CASE(InstructionDecodeTest, LC3Test)
{
    std::ifstream file("testdata/allinstrs.obj", std::ios::binary);
    file.ignore(4);

    unsigned short data;
    lc3_instr instr[18];

                            //BR #1             : op, n, z, p, pc_offset
    instr[0].br             = (br_instr){BR_INSTR, 1, 1, 1, 1};
                            //ADD R0, R1, R2    : op, dr, sr1, 0, 0, sr2
    instr[1].arith.reg      = (arithreg_instr){ADD_INSTR, 0, 1, 0, 0, 2};
                            //ADD R0, R1, #2    : op, dr, sr1, 1, imm
    instr[2].arith.imm      = (arithimm_instr){ADD_INSTR, 0, 1, 1, 2};
                            //LD R0, #-1        : op, reg, pc_offset
    instr[3].mem.offset     = (memoryoffset_instr){LD_INSTR, 0, -1};
                            //ST R0, #-1        : op, reg, pc_offset
    instr[4].mem.offset     = (memoryoffset_instr){ST_INSTR, 0, -1};
                            //JSR #-1           : op, 1, pc_offset
    instr[5].subr.jsr       = (jsr_instr){JSR_INSTR, 1, -1};
                            //JSRR R0           : op, 0, 0, reg, 0
    instr[6].subr.jsrr      = (jsrr_instr){JSRR_INSTR, 0, 0, 0, 0};
                            //AND R0, R1, R2    : op, dr, sr1, 0, 0, sr2
    instr[7].arith.reg      = (arithreg_instr){AND_INSTR, 0, 1, 0, 0, 2};
                            //AND R0, R1, #2    : op, dr, sr1, 1, imm
    instr[8].arith.imm      = (arithimm_instr){AND_INSTR, 0, 1, 1, 2};
                            //LDR R0, R1, #2    : op, reg, base, offset
    instr[9].mem.reg        = (memoryreg_instr){LDR_INSTR, 0, 1, 2};
                            //STR R0, R1, #2    : op, reg, base, offset
    instr[10].mem.reg       = (memoryreg_instr){STR_INSTR, 0, 1, 2};
                            //NOT R0, R1        : op, dr, sr1, 0x3F
    instr[11].arith.inv     = (not_instr){NOT_INSTR, 0, 1, 0x3F};
                            // LDI R0, #-1      : op, reg, pc_offset
    instr[12].mem.offset    = (memoryoffset_instr){LDI_INSTR, 0, -1};
                            // STI R0, #-1      : op, reg, pc_offset
    instr[13].mem.offset    = (memoryoffset_instr){STI_INSTR, 0, -1};
                            // RET              : op, 0, 7, 0
    instr[14].jmp           = (jmp_instr){RET_INSTR, 0, 7, 0};
                            // JMP R0           : op, 0, reg, 0
    instr[15].jmp           = (jmp_instr){JMP_INSTR, 0, 0, 0};
                            // LEA R0, #-1      : op, reg, pc_offset
    instr[16].mem.offset    = (memoryoffset_instr){LEA_INSTR, 0, -1};
                            // TRAP x25         : op, 0, vector
    instr[17].trap          = (trap_instr){TRAP_INSTR, 0, 0x25};

    unsigned short data_answers[18] = {
        0x0E01, 0x1042, 0x1062, 0x21ff, 0x31ff, 0x4fff, 0x4000,
        0x5042, 0x5062, 0x6042, 0x7042, 0x907f, 0xa1ff, 0xb1ff,
        0xc1c0, 0xc000, 0xe1ff, 0xf025
    };


    BOOST_REQUIRE(file.good());

    for (int i = 0; i < 18; i++)
    {
        file.read((char*) &data, sizeof(unsigned short));
        data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
        BOOST_CHECK_EQUAL(data, data_answers[i]);
        BOOST_CHECK_EQUAL(instr[i].bits, lc3_decode(state, data).bits);
    }

    // Testing RTI and ERROR for complete test.
    lc3_instr rti = lc3_decode(state, 0x8000);
    lc3_instr error = lc3_decode(state, 0xD392);

    BOOST_CHECK_EQUAL(rti.data.opcode, 0x8);
    BOOST_CHECK_EQUAL(rti.data.data, 0x000);

    BOOST_CHECK_EQUAL(error.data.opcode, 0xD);
    BOOST_CHECK_EQUAL(error.data.data, 0x392);

    file.close();
}

BOOST_FIXTURE_TEST_CASE(InstructionBasicDisassembleTest, LC3Test)
{
    std::ifstream file("testdata/allinstrs.obj", std::ios::binary);
    unsigned short data;

    const std::string answers[18] = {
        "BR #1",
        "ADD R0, R1, R2",
        "ADD R0, R1, #2",
        "LD R0, #-1",
        "ST R0, #-1",
        "JSR #-1",
        "JSRR R0",
        "AND R0, R1, R2",
        "AND R0, R1, #2",
        "LDR R0, R1, #2",
        "STR R0, R1, #2",
        "NOT R0, R1",
        "LDI R0, #-1",
        "STI R0, #-1",
        "RET",
        "JMP R0",
        "LEA R0, #-1",
        "TRAP x25",
    };

    BOOST_REQUIRE(file.good());
    file.ignore(4);

    for (unsigned int i = 0; i < 18; i++)
    {
        file.read((char*) &data, sizeof(unsigned short));
        data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
        std::string instruct = lc3_basic_disassemble(state, data);
        BOOST_CHECK_EQUAL(instruct,  answers[i]);
    }

    file.close();

    // Test special cases
    const std::string more_answers[5] =
    {
        "BRN #3",
        "NOP",
        "RTI",
        "ERROR",
        "NOP ('0')"
    };

    // BRN 3
    std::string brn = lc3_basic_disassemble(state, 0x0803);
    std::string nop = lc3_basic_disassemble(state, 0x0000);
    std::string rti = lc3_basic_disassemble(state, 0x8000);
    std::string error = lc3_basic_disassemble(state, 0xD392);
    std::string nop0 = lc3_basic_disassemble(state, 0x0030);

    BOOST_CHECK_EQUAL(brn, more_answers[0]);
    BOOST_CHECK_EQUAL(nop, more_answers[1]);
    BOOST_CHECK_EQUAL(rti, more_answers[2]);
    BOOST_CHECK_EQUAL(error, more_answers[3]);
    BOOST_CHECK_EQUAL(nop0, more_answers[4]);
}

BOOST_FIXTURE_TEST_CASE(TestDisassemble, LC3Test)
{
    std::ifstream file("testdata/disassemble.obj", std::ios::binary);
    unsigned short data;
    BOOST_REQUIRE(file.good());
    file.ignore(4);

    std::ifstream sym_file("testdata/disassemble.sym");
    BOOST_REQUIRE(sym_file.good());
    lc3_load_sym(state, sym_file);
    sym_file.close();

    const std::vector<std::string> answers = {
        "AND R0, R0, R0",
        "AND R0, R1, R1",
        "AND R0, R0, R1",
        "AND R0, R1, R2",
        "AND R0, R0, #-1",
        "AND R0, R1, #-1",
        "AND R0, R7, #0",
        "AND R0, R0, #7",
        "AND R0, R1, #7",
        "ADD R0, R0, R1",
        "ADD R0, R1, R2",
        "ADD R0, R0, #0",
        "ADD R0, R1, #0",
        "ADD R0, R0, #1",
        "ADD R0, R0, #-1",
        "ADD R0, R0, #7",
        "ADD R0, R1, #7",
        "NOT R0, R0",
        "NOT R0, R1",
        "NOP",
        "NOP ('0')",
        "NOP",
        "BRN LABEL",
        "BRZ LABEL",
        "BRP LABEL",
        "BRNZ LABEL",
        "BRNP LABEL",
        "BRZP LABEL",
        "BR LABEL",
        "BR LABEL",
        "BR #1",
        "BRZP #-6",
        "JSR LABEL",
        "JSR #1",
        "JSRR R1",
        "RET",
        "JMP R0",
        "LD R0, LABEL",
        "ST R0, LABEL",
        "LDR R0, R1, #0",
        "LDR R0, R1, #7",
        "STR R0, R1, #0",
        "STR R0, R1, #7",
        "LDI R0, LABEL",
        "STI R0, LABEL",
        "STI R0, #-4",
        "LEA R0, LABEL",
        "LEA R0, #1",
        "GETC",
        "OUT",
        "PUTS",
        "IN",
        "PUTSP",
        "HALT",
        "TRAP xFF",
        "RTI",
        "ERROR",
    };

    for (unsigned int i = 0; i < answers.size(); i++)
    {
        state.pc += 1;
        file.read((char*) &data, sizeof(unsigned short));
        data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
        std::string instruct = lc3_disassemble(state, data);
        BOOST_CHECK_EQUAL(instruct,  answers[i]);
    }

    file.close();
}

BOOST_FIXTURE_TEST_CASE(TestSmartDisassemble, LC3Test)
{
    std::ifstream file("testdata/disassemble.obj", std::ios::binary);
    unsigned short data;
    BOOST_REQUIRE(file.good());
    file.ignore(4);

    std::ifstream sym_file("testdata/disassemble.sym");
    BOOST_REQUIRE(sym_file.good());
    lc3_load_sym(state, sym_file);
    sym_file.close();

    const std::vector<std::string> answers = {
        "TEST R0",
        "R0 = R1",
        "R0 &= R1",
        "R0 = R1 & R2",
        "TEST R0",
        "R0 = R1",
        "R0 = 0",
        "R0 &= 7",
        "R0 = R1 & 7",

        "R0 += R1",
        "R0 = R1 + R2",
        "TEST R0",
        "R0 = R1",
        "R0++",
        "R0--",
        "R0 += 7",
        "R0 = R1 + 7",

        "R0 = ~R0",
        "R0 = ~R1",

        "NOP",
        "NOP ('0')",
        "NOP",
        "if cc<0, PC = LABEL",
        "if cc=0, PC = LABEL",
        "if cc>0, PC = LABEL",
        "if cc<=0, PC = LABEL",
        "if cc!=0, PC = LABEL",
        "if cc>=0, PC = LABEL",
        "PC = LABEL",
        "PC = LABEL",
        "PC += 1",
        "if cc>=0, PC -= 6",

        "CALL LABEL",
        "CALL PC + 1",
        "CALL R1",

        "RET",
        "JUMP R0",

        "R0 = LABEL",
        "LABEL = R0",

        "R0 = R1[0]",
        "R0 = R1[7]",

        "R1[0] = R0",
        "R1[7] = R0",

        "R0 = *LABEL",
        "*LABEL = R0",
        "*(PC - 4) = R0",

        "R0 = &LABEL",
        "R0 = PC + 1",

        "GETC",
        "OUT",
        "PUTS",
        "IN",
        "PUTSP",
        "HALT",
        "TRAP xff",

        "RTI",
        "ERROR",
    };

    for (unsigned int i = 0; i < answers.size(); i++)
    {
        state.pc += 1;
        file.read((char*) &data, sizeof(unsigned short));
        data = ((data >> 8) & 0xFF) | ((data & 0xFF) << 8);
        std::string instruct = lc3_smart_disassemble(state, data);
        BOOST_CHECK_EQUAL(instruct,  answers[i]);
    }

    file.close();
}

BOOST_FIXTURE_TEST_CASE(TestArithInstructions, LC3Test)
{
    arithreg_instr add_r, and_r;
    arithimm_instr add_i, and_i;
    not_instr not_r;
    lc3_instr instr;

            //ADD R0, R1, R2    : op, dr, sr1, 0, 0, sr2
    add_r = (arithreg_instr){ADD_INSTR, 0, 1, 0, 0, 2};
            //ADD R0, R1, #2    : op, dr, sr1, 1, imm
    add_i = (arithimm_instr){ADD_INSTR, 0, 1, 1, 2};
            //AND R0, R1, R2    : op, dr, sr1, 0, 0, sr2
    and_r = (arithreg_instr){AND_INSTR, 0, 1, 0, 0, 2};
            //AND R0, R1, #2    : op, dr, sr1, 1, imm
    and_i = (arithimm_instr){AND_INSTR, 0, 1, 1, 2};
            //NOT R0, R1        : op, dr, sr1, 0x3F
    not_r = (not_instr){NOT_INSTR, 0, 1, 0x3F};

    // ADD R0, R1, R2
    state.regs[0] = 23;
    state.regs[1] = 27;
    state.regs[2] = 43;
    instr.arith.reg = add_r;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 70);
    BOOST_CHECK_EQUAL(state.regs[1], 27);
    BOOST_CHECK_EQUAL(state.regs[2], 43);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);

    // ADD R0, R1, #2
    state.regs[0] = 23;
    state.regs[1] = -666;
    instr.arith.imm = add_i;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], -664);
    BOOST_CHECK_EQUAL(state.regs[1], -666);
    BOOST_CHECK_EQUAL(state.n, 1);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 0);

    // AND R0, R1, R2
    state.regs[0] = 23;
    state.regs[1] = (short)0xFFFF;
    state.regs[2] = 0x0;
    instr.arith.reg = and_r;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 0);
    BOOST_CHECK_EQUAL(state.regs[1], (short)0xFFFF);
    BOOST_CHECK_EQUAL(state.regs[2], 0x0);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 1);
    BOOST_CHECK_EQUAL(state.p, 0);

    // AND R0, R1, #2
    state.regs[0] = 23;
    state.regs[1] = (short)0xFFFF;
    instr.arith.imm = and_i;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 2);
    BOOST_CHECK_EQUAL(state.regs[1], (short)0xFFFF);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);

    // NOT R0, R1
    state.regs[0] = 23;
    state.regs[1] = 0x0;
    instr.arith.inv = not_r;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], (short)0xFFFF);
    BOOST_CHECK_EQUAL(state.regs[1], 0x0);
    BOOST_CHECK_EQUAL(state.n, 1);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 0);
}

BOOST_FIXTURE_TEST_CASE(TestControlInstructions, LC3Test)
{
    br_instr brnzp, brn, brz, brp, brnil;
    jsr_instr jsr;
    jsrr_instr jsrr;
    jsrr_instr jsrr7;
    jmp_instr ret, jmp;
    trap_instr halt;
    lc3_instr instr;

    //BR #1             : op, n, z, p, pc_offset
    brnzp = (br_instr){BR_INSTR, 1, 1, 1, 1};
    //BRN #1            : op, n, z, p, pc_offset
    brn   = (br_instr){BR_INSTR, 1, 0, 0, 1};
    //BRZ #-11          : op, n, z, p, pc_offset
    brz   = (br_instr){BR_INSTR, 0, 1, 0, -11};
    //BRP #1            : op, n, z, p, pc_offset
    brp   = (br_instr){BR_INSTR, 0, 0, 1, 1};
    //.fill #1          : op, n, z, p, pc_offset
    brnil = (br_instr){BR_INSTR, 0, 0, 0, 1};

    //JSR #-1           : op, 1, pc_offset
    jsr   = (jsr_instr){JSR_INSTR, 1, -1};
    //JSRR R0           : op, 0, 0, reg, 0
    jsrr  = (jsrr_instr){JSRR_INSTR, 0, 0, 0, 0};
    //JSRR R7           : op, 0, 0, reg, 0
    jsrr7  = (jsrr_instr){JSRR_INSTR, 0, 0, 7, 0};
    // RET              : op, 0, 7, 0
    ret   = (jmp_instr){RET_INSTR, 0, 7, 0};
    // JMP R0           : op, 0, reg, 0
    jmp   = (jmp_instr){JMP_INSTR, 0, 0, 0};
    // TRAP x25         : op, 0, vector
    halt  = (trap_instr){TRAP_INSTR, 0, 0x25};


    // BR #1 (Taken)
    instr.br = brnzp;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x3001U);

    // BRN #1 (Not taken)
    state.pc = 0x3000U;
    instr.br = brn;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x3000U);

    // BRZ #-11 (taken)
    instr.br = brz;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x2FF5);

    // BRP #1 (Not taken)
    state.pc = 0x3000U;
    instr.br = brp;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x3000U);

    // .fill #1 (Not taken / This is also a NOP)
    state.pc = 0x3000U;
    instr.br = brnil;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x3000U);

    // JSR #-1
    state.pc = 0x3000U;
    instr.subr.jsr = jsr;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x2FFFU);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3000);

    // JSRR R0
    state.pc = 0x3000U;
    state.regs[0] = 0x6000;
    instr.subr.jsrr = jsrr;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x6000U);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3000);

    // JSRR R7
    state.pc = 0x3000U;
    state.regs[7] = 0x6000;
    instr.subr.jsrr = jsrr7;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x6000U);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3000);

    // JMP R0
    state.pc = 0x3000U;
    state.regs[0] = 0x4000;
    state.regs[7] = 0x2000;
    instr.jmp = jmp;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x4000U);
    BOOST_CHECK_EQUAL(state.regs[0], 0x4000);
    BOOST_CHECK_EQUAL(state.regs[7], 0x2000);

    // RET
    state.pc = 0x3000U;
    state.regs[7] = 0x2000;
    instr.jmp = ret;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x2000U);
    BOOST_CHECK_EQUAL(state.regs[7], 0x2000);

    // TRAP x25 aka HALT
    state.pc = 0x3000U;
    instr.trap = halt;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.halted, 1);

    // TEST 2 with TRAP x25 (True traps)
    state.true_traps = 1;
    state.pc = 0x3000U;
    instr.trap = halt;
    state.mem[0x25] = 0x0490;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x0490U);
    BOOST_CHECK_EQUAL(state.regs[0x7], 0x3000);
}

BOOST_FIXTURE_TEST_CASE(TestErrorInstructions, LC3Test)
{
    lc3_instr instr;

    state.pc = 0x3000U;
    instr.data.opcode = ERROR_INSTR;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x2FFFU);
    BOOST_CHECK_EQUAL(state.halted, 1);
    BOOST_CHECK_EQUAL(state.warnings, 1U);

    state.pc = 0x3000U;
    state.halted = 0;
    state.warnings = 0;
    instr.data.opcode = RTI_INSTR;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x2FFFU);
    BOOST_CHECK_EQUAL(state.halted, 1);
    BOOST_CHECK_EQUAL(state.warnings, 1U);

    state.pc = 0x3000U;
    state.halted = 0;
    state.warnings = 0;
    instr.data.opcode = TRAP_INSTR;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.pc, 0x2FFFU);
    BOOST_CHECK_EQUAL(state.halted, 1);
    BOOST_CHECK_EQUAL(state.warnings, 1U);
}

BOOST_FIXTURE_TEST_CASE(TestMemoryInstructions, LC3Test)
{
    memoryoffset_instr ld, st, ldi, sti, lea;
    memoryreg_instr ldr, str;
    lc3_instr instr;

    // LD R0, #-1        : op, reg, pc_offset
    ld = (memoryoffset_instr){LD_INSTR, 0, -1};
    // ST R0, #-1        : op, reg, pc_offset
    st = (memoryoffset_instr){ST_INSTR, 0, -1};
    // LDR R0, R1, #2    : op, reg, base, offset
    ldr = (memoryreg_instr){LDR_INSTR, 0, 1, 2};
    // STR R0, R1, #2    : op, reg, base, offset
    str = (memoryreg_instr){STR_INSTR, 0, 1, 2};
    // LDI R0, #-1      : op, reg, pc_offset
    ldi = (memoryoffset_instr){LDI_INSTR, 0, -1};
    // STI R0, #-1      : op, reg, pc_offset
    sti = (memoryoffset_instr){STI_INSTR, 0, -1};
    // LEA R0, #-1      : op, reg, pc_offset
    lea = (memoryoffset_instr){LEA_INSTR, 0, -1};

    // LD R0, #-1
    state.mem[0x2FFF] = 23;
    instr.mem.offset = ld;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 23);
    BOOST_CHECK_EQUAL(state.mem[0x2FFF], 23);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);

    // ST R0, #-1
    state.regs[0] = 0;
    instr.mem.offset = st;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 0);
    BOOST_CHECK_EQUAL(state.mem[0x2FFF], 0);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);

    // LDI R0, #-1
    state.mem[0x2FFF] = 0x4000;
    state.mem[0x4000] = -102;
    instr.mem.offset = ldi;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], -102);
    BOOST_CHECK_EQUAL(state.mem[0x2FFF], 0x4000);
    BOOST_CHECK_EQUAL(state.mem[0x4000], -102);
    BOOST_CHECK_EQUAL(state.n, 1);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 0);

    // STI R0, #-1
    state.regs[0] = 1337;
    state.mem[0x2FFF] = 0x4000;
    state.mem[0x4000] = -102;
    instr.mem.offset = sti;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 1337);
    BOOST_CHECK_EQUAL(state.mem[0x2FFF], 0x4000);
    BOOST_CHECK_EQUAL(state.mem[0x4000], 1337);
    BOOST_CHECK_EQUAL(state.n, 1);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 0);

    // LEA R0, #-1
    instr.mem.offset = lea;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 0x2FFF);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);

    // LDR R0, R1, #2
    state.regs[0] = 1337;
    state.regs[1] = 0x3004;
    state.mem[0x3006] = -7331;
    instr.mem.reg = ldr;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], -7331);
    BOOST_CHECK_EQUAL(state.regs[1], 0x3004);
    BOOST_CHECK_EQUAL(state.mem[0x3006], -7331);
    BOOST_CHECK_EQUAL(state.n, 1);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 0);

    // STR R0, R1, #2
    state.regs[0] = 1337;
    state.regs[1] = 0x3004;
    state.mem[0x3006] = -7331;
    instr.mem.reg = str;
    lc3_execute(state, instr);
    BOOST_CHECK_EQUAL(state.regs[0], 1337);
    BOOST_CHECK_EQUAL(state.regs[1], 0x3004);
    BOOST_CHECK_EQUAL(state.mem[0x3006], 1337);
    BOOST_CHECK_EQUAL(state.n, 1);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 0);
}

BOOST_FIXTURE_TEST_CASE(TestLoadObj, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    BOOST_CHECK_EQUAL(state.mem[0x3000], 0x2003);
    BOOST_CHECK_EQUAL(state.mem[0x3001], 0x2203);
    BOOST_CHECK_EQUAL(state.mem[0x3002], 0x1401);
    BOOST_CHECK_EQUAL(state.mem[0x3003], (short)0xF025);
    BOOST_CHECK_EQUAL(state.mem[0x3004], 0x003C);
    BOOST_CHECK_EQUAL(state.mem[0x3005], 0x0006);
    BOOST_CHECK_EQUAL(state.mem[0x3006], 0x0000);
}

BOOST_FIXTURE_TEST_CASE(TestLoadHex, LC3Test)
{
    std::ifstream file("testdata/simple.hex");
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_hex);
    file.close();

    BOOST_CHECK_EQUAL(state.mem[0x3000], 0x2003);
    BOOST_CHECK_EQUAL(state.mem[0x3001], 0x2203);
    BOOST_CHECK_EQUAL(state.mem[0x3002], 0x1401);
    BOOST_CHECK_EQUAL(state.mem[0x3003], (short)0xF025);
    BOOST_CHECK_EQUAL(state.mem[0x3004], 0x003C);
    BOOST_CHECK_EQUAL(state.mem[0x3005], 0x0006);
    BOOST_CHECK_EQUAL(state.mem[0x3006], 0x0000);
}

BOOST_FIXTURE_TEST_CASE(TestLoadSym, LC3Test)
{
    std::ifstream file("testdata/devreg.sym");
    BOOST_REQUIRE(file.good());
    lc3_load_sym(state, file);
    file.close();

    BOOST_CHECK_EQUAL(state.symbols["MCR"],     0x0503);
    BOOST_CHECK_EQUAL(state.symbols["KBDR"],    0x3028);
    BOOST_CHECK_EQUAL(state.symbols["KBSR"],    0x3027);
    BOOST_CHECK_EQUAL(state.symbols["POLLIN2"], 0x3023);
    BOOST_CHECK_EQUAL(state.symbols["POLLIN"],  0x3020);
    BOOST_CHECK_EQUAL(state.symbols["DDR"],     0x3005);
    BOOST_CHECK_EQUAL(state.symbols["DSR"],     0x3004);
    BOOST_CHECK_EQUAL(state.symbols["POLLOUT"], 0x3000);
}

BOOST_FIXTURE_TEST_CASE(TestRunOne, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_step(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3001U);
    BOOST_CHECK_EQUAL(state.regs[0], 60);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);
    BOOST_CHECK_EQUAL(state.executions, 1U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 1U);
}

BOOST_FIXTURE_TEST_CASE(TestRunMany, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_run(state, 3);

    BOOST_CHECK_EQUAL(state.pc, 0x3003U);
    BOOST_CHECK_EQUAL(state.regs[0], 60);
    BOOST_CHECK_EQUAL(state.regs[1], 6);
    BOOST_CHECK_EQUAL(state.regs[2], 66);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 3U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 3U);
}

BOOST_FIXTURE_TEST_CASE(TestRun, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_run(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3003U);
    BOOST_CHECK_EQUAL(state.regs[0], 60);
    BOOST_CHECK_EQUAL(state.regs[1], 6);
    BOOST_CHECK_EQUAL(state.regs[2], 66);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);
    BOOST_CHECK_EQUAL(state.halted, 1);
    BOOST_CHECK_EQUAL(state.executions, 4U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 4U);
}

BOOST_FIXTURE_TEST_CASE(TestBack, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_step(state);
    lc3_back(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3000U);
    BOOST_CHECK_EQUAL(state.regs[0], 0);
    BOOST_CHECK_EQUAL(state.regs[1], 0);
    BOOST_CHECK_EQUAL(state.regs[2], 0);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 1);
    BOOST_CHECK_EQUAL(state.p, 0);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 0U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 0U);
}

BOOST_FIXTURE_TEST_CASE(TestBackMany, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_run(state, 3);
    lc3_rewind(state, 3);

    BOOST_CHECK_EQUAL(state.pc, 0x3000U);
    BOOST_CHECK_EQUAL(state.regs[0], 0);
    BOOST_CHECK_EQUAL(state.regs[1], 0);
    BOOST_CHECK_EQUAL(state.regs[2], 0);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 1);
    BOOST_CHECK_EQUAL(state.p, 0);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 0U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 0U);
}

BOOST_FIXTURE_TEST_CASE(TestBackFull, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_run(state);
    lc3_rewind(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3000U);
    BOOST_CHECK_EQUAL(state.regs[0], 0);
    BOOST_CHECK_EQUAL(state.regs[1], 0);
    BOOST_CHECK_EQUAL(state.regs[2], 0);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 1);
    BOOST_CHECK_EQUAL(state.p, 0);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 0U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 0U);
}

BOOST_FIXTURE_TEST_CASE(TestBackInvalid, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_back(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3000U);
    BOOST_CHECK_EQUAL(state.regs[0], 0);
    BOOST_CHECK_EQUAL(state.regs[1], 0);
    BOOST_CHECK_EQUAL(state.regs[2], 0);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 1);
    BOOST_CHECK_EQUAL(state.p, 0);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 0U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 0U);
}

BOOST_FIXTURE_TEST_CASE(TestBackStore, LC3Test)
{
    std::ifstream file("testdata/store.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    // GIMME SYMBOLS
    std::ifstream symfile("testdata/store.sym");
    BOOST_REQUIRE(symfile.good());
    lc3_load_sym(state, symfile);
    symfile.close();

    state.regs[0] = 0;
    // ADD R0, R0, 15
    lc3_step(state);
    // ST R0, HELLO
    lc3_step(state);
    BOOST_CHECK_EQUAL(state.mem[state.symbols["HELLO"]], 15);

    // Go back.
    lc3_back(state);
    BOOST_CHECK_EQUAL(state.mem[state.symbols["HELLO"]], 7892);
}

BOOST_FIXTURE_TEST_CASE(TestBackR7, LC3Test)
{
    std::ifstream file("testdata/simple2.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    state.regs[0] = 25;
    state.regs[1] = 75;

    lc3_step(state);
    lc3_back(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3000U);
    BOOST_CHECK_EQUAL(state.regs[0], 25);
    BOOST_CHECK_EQUAL(state.regs[1], 75);
    BOOST_CHECK_EQUAL(state.regs[2], 0);
    BOOST_CHECK_EQUAL(state.regs[7], 0x490);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 1);
    BOOST_CHECK_EQUAL(state.p, 0);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 0U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 0U);
}

BOOST_FIXTURE_TEST_CASE(TestNextLine, LC3Test)
{
    std::ifstream file("testdata/simplesubr.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_next_line(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3001U);
    BOOST_CHECK_EQUAL(state.regs[0], 60);
    BOOST_CHECK_EQUAL(state.regs[1], 6);
    BOOST_CHECK_EQUAL(state.regs[2], 66);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 5U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 5U);
}

BOOST_FIXTURE_TEST_CASE(TestPrevLine, LC3Test)
{
    std::ifstream file("testdata/simplesubr.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_next_line(state);
    lc3_prev_line(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3000U);
    BOOST_CHECK_EQUAL(state.regs[0], 0);
    BOOST_CHECK_EQUAL(state.regs[1], 0);
    BOOST_CHECK_EQUAL(state.regs[2], 0);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 1);
    BOOST_CHECK_EQUAL(state.p, 0);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 0U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 0U);
}

BOOST_FIXTURE_TEST_CASE(TestFinish, LC3Test)
{
    std::ifstream file("testdata/simplesubr.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    lc3_step(state);
    lc3_finish(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3001U);
    BOOST_CHECK_EQUAL(state.regs[0], 60);
    BOOST_CHECK_EQUAL(state.regs[1], 6);
    BOOST_CHECK_EQUAL(state.regs[2], 66);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);
    BOOST_CHECK_EQUAL(state.halted, 0);
    BOOST_CHECK_EQUAL(state.executions, 5U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 5U);
}

BOOST_FIXTURE_TEST_CASE(TestUndoStack, LC3Test)
{
    std::ifstream file("testdata/simple.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    state.max_stack_size = 1;

    lc3_step(state);
    lc3_step(state);

    BOOST_CHECK_EQUAL(state.undo_stack.size(), 1U);

    lc3_back(state);
    lc3_back(state);

    BOOST_CHECK_EQUAL(state.pc, 0x3001U);
    BOOST_CHECK_EQUAL(state.regs[0], 60);
    BOOST_CHECK_EQUAL(state.n, 0);
    BOOST_CHECK_EQUAL(state.z, 0);
    BOOST_CHECK_EQUAL(state.p, 1);
    BOOST_CHECK_EQUAL(state.executions, 1U);
    BOOST_CHECK_EQUAL(state.undo_stack.size(), 0U);

}

BOOST_FIXTURE_TEST_CASE(TestTrapInstructions, LC3Test)
{
    std::ifstream file("testdata/traps.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    BOOST_CHECK_EQUAL(state.mem[0x3000], (short)0xF021);
    BOOST_CHECK_EQUAL(state.mem[0x3001], (short)0xF023);
    BOOST_CHECK_EQUAL(state.mem[0x3002], (short)0xF020);
    BOOST_CHECK_EQUAL(state.mem[0x3003], (short)0xE004);
    BOOST_CHECK_EQUAL(state.mem[0x3004], (short)0xF022);
    BOOST_CHECK_EQUAL(state.mem[0x3005], (short)0xE00E);
    BOOST_CHECK_EQUAL(state.mem[0x3006], (short)0xF024);
    BOOST_CHECK_EQUAL(state.mem[0x3007], (short)0xF025);
    BOOST_CHECK_EQUAL(state.mem[0x3008], 'H');
    BOOST_CHECK_EQUAL(state.mem[0x3009], 'E');
    BOOST_CHECK_EQUAL(state.mem[0x300A], 'L');
    BOOST_CHECK_EQUAL(state.mem[0x300B], 'L');
    BOOST_CHECK_EQUAL(state.mem[0x300C], 'O');
    BOOST_CHECK_EQUAL(state.mem[0x300D], ' ');
    BOOST_CHECK_EQUAL(state.mem[0x300E], 'W');
    BOOST_CHECK_EQUAL(state.mem[0x300F], 'O');
    BOOST_CHECK_EQUAL(state.mem[0x3010], 'R');
    BOOST_CHECK_EQUAL(state.mem[0x3011], 'L');
    BOOST_CHECK_EQUAL(state.mem[0x3012], 'D');
    BOOST_CHECK_EQUAL(state.mem[0x3013], 0x0000);
    BOOST_CHECK_EQUAL(state.mem[0x3014], '1' << 8 | '0');
    BOOST_CHECK_EQUAL(state.mem[0x3015], '3' << 8 | '2');
    BOOST_CHECK_EQUAL(state.mem[0x3016], '5' << 8 | '4');
    BOOST_CHECK_EQUAL(state.mem[0x3017], '7' << 8 | '6');
    BOOST_CHECK_EQUAL(state.mem[0x3018], '9' << 8 | '8');
    BOOST_CHECK_EQUAL(state.mem[0x3019], '0');
    BOOST_CHECK_EQUAL(state.mem[0x301A], 0x0000);


    std::ifstream proginput("testdata/trapsinput.txt");
    std::ofstream progoutput("testdata/trapsoutput.txt");

    BOOST_REQUIRE(proginput.good() && progoutput.good());

    state.input = &proginput;
    state.output = &progoutput;

    // OUT;
    state.regs[0] = 65;
    lc3_step(state);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3001);

    // IN
    lc3_step(state);
    BOOST_CHECK_EQUAL(state.regs[0], 66);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3002);

    // GETC
    lc3_step(state);
    BOOST_CHECK_EQUAL(state.regs[0], 67);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3003);

    // LEA R0, HELLOWORLD; PUTS;
    lc3_run(state, 2);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3005);

    // LEA R0, PUTSPSTR; PUTSP;
    lc3_run(state, 2);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3007);

    // HALT
    lc3_step(state);
    BOOST_CHECK_EQUAL(state.halted, 0x1);
    BOOST_CHECK_EQUAL(state.pc, 0x3007);
    BOOST_CHECK_EQUAL(state.regs[7], 0x3008);

    proginput.close();
    progoutput.close();
    state.input = &std::cin;
    state.output = &std::cout;

    std::ifstream verify("testdata/trapsoutput.txt");
    BOOST_CHECK_EQUAL(verify.get(), 65);

    char* str = new char[18];
    const std::string answers[3] = {"Input character: ", "HELLO WORLD", "01234567890"};
    std::string val;

    verify.get(str, 18);
    str[17] = 0;
    val = str;
    BOOST_CHECK_EQUAL(val, answers[0]);

    BOOST_CHECK_EQUAL(verify.get(), 66);

    verify.get(str, 12);
    str[12] = 0;
    val = str;
    BOOST_CHECK_EQUAL(val, answers[1]);

    verify.get(str, 12);
    str[12] = 0;
    val = str;
    BOOST_CHECK_EQUAL(val, answers[2]);
    delete[] str;

    verify.close();
}

BOOST_FIXTURE_TEST_CASE(TestDeviceRegisters, LC3Test)
{
    std::ifstream file("testdata/devreg.obj", std::ios::binary);
    BOOST_REQUIRE(file.good());
    lc3_load(state, file, lc3_reader_obj);
    file.close();

    state.regs[0x1] = 65;

    std::ifstream proginput("testdata/devreginput.txt");
    std::ofstream progoutput("testdata/devregoutput.txt");
    BOOST_REQUIRE(proginput.good() && progoutput.good());

    state.input = &proginput;
    state.output = &progoutput;

    // Execute DSR/DDR test
    lc3_run(state);

    // Execute KBSR/KBDR test
    state.halted = 0;
    state.pc = 0x3020;
    lc3_run(state);

    BOOST_CHECK_EQUAL(state.regs[0x1], 66);
    BOOST_CHECK_EQUAL(state.regs[0x2], 67);

    // Execute MCR Test
    state.halted = 0;
    state.pc = 0x500;
    lc3_run(state);

    BOOST_CHECK_EQUAL(state.halted, 0x1);
    BOOST_CHECK_EQUAL(state.pc, 0x502);
    BOOST_CHECK_EQUAL(state.warnings, 0U); // No warning since its in the OS MEM.
    BOOST_CHECK_EQUAL(state.regs[1], (short)0x8000);

    proginput.close();
    progoutput.close();
    state.input = &std::cin;
    state.output = &std::cout;

    std::ifstream verify("testdata/devregoutput.txt");
    BOOST_CHECK_EQUAL(verify.get(), 65);
    verify.close();
}

BOOST_FIXTURE_TEST_CASE(TestSymbolTable, LC3Test)
{
    // LOL is not in the symbol table.
    BOOST_CHECK_EQUAL(lc3_sym_lookup(state, "LOL"), -1);
    // 0x3000 is not referenced in the symbol table
    BOOST_CHECK_EQUAL(lc3_sym_rev_lookup(state, 0x3000), "");

    // Add LOL to the symbol table
    lc3_sym_add(state, "LOL", 0x3000);


    BOOST_CHECK_EQUAL(lc3_sym_lookup(state, "LOL"), 0x3000);
    BOOST_CHECK_EQUAL(lc3_sym_rev_lookup(state, 0x3000), "LOL");

    lc3_sym_delete(state, "LOL");

    BOOST_CHECK_EQUAL(lc3_sym_lookup(state, "LOL"), -1);
    BOOST_CHECK_EQUAL(lc3_sym_rev_lookup(state, 0x3000), "");

    lc3_sym_add(state, "LOL_LOL", 0x3000);

    state.regs[0] = 0;
    state.regs[1] = 0;
    state.regs[2] = 0;
    state.regs[3] = 0;
    state.regs[4] = 0;
    state.regs[5] = 0;
    state.regs[6] = 0;
    state.regs[7] = 0;

    state.mem[0] = 0;

    int ans, ans1, ans2, ans3;
    BOOST_CHECK_EQUAL(lc3_calculate(state, "*LOL_LOL + R0 + R1 + R2+R3 + R4 + R5 + R6 + R7", ans), 0);
    BOOST_CHECK_EQUAL(lc3_calculate(state, "&LOL_LOL + LOL_LOL", ans1), 0);
    BOOST_CHECK_EQUAL(lc3_calculate(state, "NOTHERE", ans2), (int)ExpressionEvaluator::eval_undefinedsymbol);
    // References
    BOOST_CHECK_EQUAL(lc3_calculate(state, "R0[0] + R1[0] + R2[0] + R3[0] + R4[0] + R5[0] + R6[0] + R7[0] + MEM[0] + LOL_LOL[0]", ans2), 0);
    BOOST_CHECK_EQUAL(lc3_calculate(state, "PC + PC[0]", ans3), 0);
    BOOST_CHECK_EQUAL(ans, state.mem[0x3000]);
    BOOST_CHECK_EQUAL(ans1, 0);
    BOOST_CHECK_EQUAL(ans2, state.mem[0x3000]);
    BOOST_CHECK_EQUAL(ans3, state.pc);

    lc3_sym_clear(state);

    BOOST_CHECK_EQUAL(lc3_sym_lookup(state, "LOL"), -1);
    BOOST_CHECK_EQUAL(lc3_sym_rev_lookup(state, 0x3000), "");
}

BOOST_FIXTURE_TEST_CASE(TestBreakpoints, LC3Test)
{
    lc3_add_break(state, 0x3012);
    lc3_add_break(state, 0x3016, "", "1");
    lc3_add_break(state, 0x3020, "", "0");
    lc3_add_break(state, 0x3040, "", "1", 3);
    state.mem[0x3060] = (short)0xF025;

    lc3_run(state);
    BOOST_CHECK_EQUAL(state.pc, 0x3012);

    state.halted = 0;

    lc3_run(state);
    BOOST_CHECK_EQUAL(state.pc, 0x3016);

    state.halted = 0;
    lc3_run(state, 5);
    BOOST_CHECK_EQUAL(state.halted, 0);

    for (int i = 0; i < 4; i++)
    {
        state.halted = 0;
        lc3_run(state);
        state.pc = 0x3025;
    }

    BOOST_CHECK(lc3_remove_break(state, 0x3040));
}

BOOST_FIXTURE_TEST_CASE(InstructionBasicAssembleTest, LC3Test)
{
    const std::vector<std::string> instruct = {
        "BR #1",
        "ADD R0, R1, R2",
        "ADD R0, R1, #2",
        "LD R0, #-1",
        "ST R0, #-1",
        "JSR #-1",
        "JSRR R0",
        "AND R0, R1, R2",
        "AND R0, R1, #2",
        "LDR R0, R1, #2",
        "STR R0, R1, #2",
        "NOT R0, R1",
        "LDI R0, #-1",
        "STI R0, #-1",
        "RET",
        "JMP R0",
        "LEA R0, #-1",
        "TRAP x25",
        "BR 012",
        "GETC",
        "OUT",
        "PUTS",
        "IN",
        "PUTSP",
        "HALT",
        "RTI",
        "BRN #0",
        "BRNZ #0",
        "BRNP #0",
        "BRZ #0",
        "BRZP #0",
        "BRP #0",
        "BRNZP #0",
        "TRAP xFF"
    };

    std::vector<unsigned short> answers = {
        0x0E01U,
        0x1042U,
        0x1062U,
        0x21FFU,
        0x31FFU,
        0x4FFFU,
        0x4000U,
        0x5042U,
        0x5062U,
        0x6042U,
        0x7042U,
        0x907FU,
        0xA1FFU,
        0xB1FFU,
        0xC1C0U,
        0xC000U,
        0xE1FFU,
        0xF025U,
        0x0E0AU,
        0xF020U,
        0xF021U,
        0xF022U,
        0xF023U,
        0xF024U,
        0xF025U,
        0x8000U,
        0x0800U,
        0x0C00U,
        0x0A00U,
        0x0400U,
        0x0600U,
        0x0200U,
        0x0E00U,
        0xF0FFU,
    };

    for (unsigned int i = 0; i < instruct.size(); i++)
    {
        try
        {
            std::string instruction = instruct[i];
            unsigned short assembled = lc3_assemble_one(state, 0, instruction);
            //BOOST_TEST_MESSAGE(instruction.c_str());
            BOOST_CHECK_EQUAL(assembled,  answers[i]);
        }
        catch (LC3AssembleException e)
        {
            BOOST_FAIL(e.what());
        }
    }
}

BOOST_FIXTURE_TEST_CASE(InstructionAssembleTest, LC3Test)
{
    try
    {
        lc3_assemble(state, "testdata/simple.asm");
    }
    catch (LC3AssembleException e)
    {
        BOOST_FAIL(e.what());
    }

    BOOST_CHECK_EQUAL(state.mem[0x3000], 0x2003);
    BOOST_CHECK_EQUAL(state.mem[0x3001], 0x2203);
    BOOST_CHECK_EQUAL(state.mem[0x3002], 0x1401);
    BOOST_CHECK_EQUAL(state.mem[0x3003], (short)0xF025);
    BOOST_CHECK_EQUAL(state.mem[0x3004], 0x003C);
    BOOST_CHECK_EQUAL(state.mem[0x3005], 0x0006);
    BOOST_CHECK_EQUAL(state.mem[0x3006], 0x0000);

    BOOST_CHECK_EQUAL(state.symbols["A"], 0x3004);
    BOOST_CHECK_EQUAL(state.symbols["B"], 0x3005);
}
