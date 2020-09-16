#include <cstdio>
#include <string>
#include <string.h>
struct Inst
{
    int opc = 0;
    int opc2 = 0;
    int opc3 = 0;
    int imm = 0;
    int imm2 = 0;
    int rs1 = 0;
    int rs2 = 0;
    int rd = 0;
    int imm3 = 0;
    int imm4 = 0;
    int imm5 = 0;

    // I'm sure there was a better way to do this, but this is my C++ "dictionary"... (first value in each is the size)
    int opc_keys[4] = { 4, 0x37, 0x6F, 0x67 };
    std::string opc_vals[3] = { "lui", "jal", "jalr" };
    int LOAD_keys[8] = { 8, 0x0, 0x1, 0x2, 0x4, 0x5, 0x6, 0x3 };
    std::string LOAD_vals[7] = { "lb", "lh", "lw", "lbu", "lhu", "lwu", "ld" };
    int STORE_keys[5] = { 5, 0x0, 0x1, 0x2, 0x3 };
    std::string STORE_vals[4] = { "sb", "sh", "sw", "sd" };
    int OP_IMM_keys[6] = { 6, 0x0, 0x4, 0x6, 0x7, 0x1 };
    std::string OP_IMM_vals[5] = { "addi", "xori", "ori", "andi", "slli" };
    int OP_IMM2_keys[3] = { 3, 0x0, 0x10 };
    std::string OP_IMM2_vals[2] = { "srli", "srai" };
    int OP_keys[5] = { 5, 0x1, 0x4, 0x6, 0x7 };
    std::string OP_vals[4] = { "sll", "xor", "or", "and" };
    int OP2_keys[3] = { 3, 0x0, 0x20 };
    std::string OP2_vals[2] = { "add", "sub" };
    int OP3_keys[3] = { 3, 0x0, 0x20 };
    std::string OP3_vals[2] = { "srl", "sra" };
    int S_keys[3] = { 3, 0x0, 0x20 };
    std::string S_vals[2] = { "srliw", "sraiw" };

    int r_keys[33] = { 33, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };
    std::string r_vals[32] = { "zero","ra","sp","gp","tp","t0","t1","t2","s0","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6" };
};

int* createList(const char* file_name, int& read);
int bitCpy(int a, bool is_neg, int b, int c, int d);
std::string lookup(int keys[], int key, std::string vals[]);

int main(int argc, const char* argv[])
{
    int read;
    bool rFlag = true; // flag for to print zero or x0 etc
    FILE* file;
    int* list = createList(argv[1], read); // creates list of bytes from .bin file
    if (strcmp(argv[2], "-") == 0) file = stdout; // sets output to stdout if - argument
    else file = fopen(argv[2], "w");

    if (argc == 4) // if 4th argument is specified
    {
        if (strcmp(argv[3], "x") == 0) rFlag = false;
    }

    for (int i = 0; i < read; i++) // for each instruction in .bin file..
    {
        std::string arg{};
        Inst inst = {};

        /* Switch statement for eery argument to be disassembled including invalids

           Each case evaluates the first opcode, if there are more than 1 of the same opcode
           another switch activates to evaluate the second opcode and so on...

           Each portion of the instruction is assigned according to the opcode
           Then the arguments are compiled into a single string for formatting purposes
           The argument string is changed based on the third program parameter (x or a)

           Finally the finalized string is printed along with the comment containing the bytes
        */
        switch (list[i] & 0x7F) // Check 1st opcode
        {
        case(0x37): // LUI
            inst.opc = 0x37;
            inst.imm = 0xFFFFF000 & list[i];
            inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
            arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm);
            fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.opc_keys, inst.opc, inst.opc_vals).c_str(), arg.c_str(), list[i]);
            break;
        case(0x6F): // JAL
            inst.opc = 0x6F;
            // rearranging the immediate bits
            inst.imm3 = (0xFF000 & list[i]) >> 1;
            inst.imm2 = (0x100000 & list[i]) >> 10;
            inst.imm4 = (0x80000000 & list[i]) >> 12;
            inst.imm = (0x7FE00000 & list[i]) >> 21;
            inst.imm5 = (inst.imm | inst.imm2 | inst.imm3 | inst.imm4) << 1;
            if ((list[i] >> 31) & 1) inst.imm5 |= 0xFFE00000; // checking if immediate was negative
            inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
            arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm5) : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm5);
            fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.opc_keys, inst.opc, inst.opc_vals).c_str(), arg.c_str(), list[i]);
            break;
        case(0x67): // JALR
            inst.opc = 0x67;
            inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);
            inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
            inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
            arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm5) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm5);
            fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.opc_keys, inst.opc, inst.opc_vals).c_str(), arg.c_str(), list[i]);
            break;
        default:
            fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
            break;

            // LOAD Instructions
        case(0x3):
            inst.opc = 0x3;
            switch (list[i] & 0x7000)
            {
            case(0): // LB
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm) + "(x" + std::to_string(inst.rs1) + ")";
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.LOAD_keys, inst.opc2, inst.LOAD_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x1000): // LH
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm) + "(x" + std::to_string(inst.rs1) + ")";                        fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.LOAD_keys, inst.opc2, inst.LOAD_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x2000): // LW
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm) + "(x" + std::to_string(inst.rs1) + ")";                        fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.LOAD_keys, inst.opc2, inst.LOAD_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x4000): // LBU
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm) + "(x" + std::to_string(inst.rs1) + ")";                        fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.LOAD_keys, inst.opc2, inst.LOAD_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x5000): // LHU
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm) + "(x" + std::to_string(inst.rs1) + ")";                        fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.LOAD_keys, inst.opc2, inst.LOAD_vals).c_str(), arg.c_str(), list[i]);
                break;

                // RV64I
            case(0x6000): // LWU
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm) + "(x" + std::to_string(inst.rs1) + ")";                        fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.LOAD_keys, inst.opc2, inst.LOAD_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x3000):
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + std::to_string(inst.imm) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rd) + ", " + std::to_string(inst.imm) + "(x" + std::to_string(inst.rs1) + ")";                        fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.LOAD_keys, inst.opc2, inst.LOAD_vals).c_str(), arg.c_str(), list[i]);
                break;
            default:
                fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
                break;
            }
            break;

            // STORE Instructions
        case(0x23):
            inst.opc = 0x23;
            switch (list[i] & 0x7000)
            {
            case(0): // sb
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.imm = bitCpy(inst.imm, true, list[i], 25, 32);
                inst.imm2 = bitCpy(inst.imm2, true, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rs2, inst.r_vals) + ", " + std::to_string(inst.imm + inst.imm2) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rs2) + ", " + std::to_string(inst.imm + inst.imm2) + "(x" + std::to_string(inst.rs1) + ")";
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.STORE_keys, inst.opc2, inst.STORE_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x1000): // sh
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.imm = bitCpy(inst.imm, true, list[i], 25, 32);
                inst.imm2 = bitCpy(inst.imm2, true, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rs2, inst.r_vals) + ", " + std::to_string(inst.imm + inst.imm2) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rs2) + ", " + std::to_string(inst.imm + inst.imm2) + "(x" + std::to_string(inst.rs1) + ")";
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.STORE_keys, inst.opc2, inst.STORE_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x2000): // sw
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.imm = bitCpy(inst.imm, true, list[i], 25, 32);
                inst.imm2 = bitCpy(inst.imm2, true, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rs2, inst.r_vals) + ", " + std::to_string(inst.imm + inst.imm2) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rs2) + ", " + std::to_string(inst.imm + inst.imm2) + "(x" + std::to_string(inst.rs1) + ")";
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.STORE_keys, inst.opc2, inst.STORE_vals).c_str(), arg.c_str(), list[i]);
                break;

                // RV64I
            case(0x3000): // sd
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.imm = bitCpy(inst.imm, true, list[i], 25, 32);
                inst.imm2 = bitCpy(inst.imm2, true, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rs2, inst.r_vals) + ", " + std::to_string(inst.imm + inst.imm2) + "(" + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ")" : "x" + std::to_string(inst.rs2) + ", " + std::to_string(inst.imm + inst.imm2) + "(x" + std::to_string(inst.rs1) + ")";
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.STORE_keys, inst.opc2, inst.STORE_vals).c_str(), arg.c_str(), list[i]);
                break;
            default:
                fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
                break;
            }
            break;

            // OP-IMM Instructions
        case(0x13):
            inst.opc = 0x13; // 1st opcode = OP-IMM
            switch (list[i] & 0x7000) // 2nd opcode
            {
            case(0): // ADDI
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                //print = lookup(inst.OP_IMM_keys, inst.opc2, inst.OP_IMM_vals) "  + std::to_string(inst.rd) + std::to_string(inst.rs1) + std::to_string(inst.imm);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + "x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_IMM_keys, inst.opc2, inst.OP_IMM_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x4000): // XORI
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + "x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_IMM_keys, inst.opc2, inst.OP_IMM_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x6000): // ORI
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + "x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_IMM_keys, inst.opc2, inst.OP_IMM_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x7000): // ANDI
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15); // indexes 12-15 for 2nd opcode
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 32);  // indexes 20-32 for immediate
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);    // indexes 7-10 for rd
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);  // indexes 15-20 for rs1
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + "x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_IMM_keys, inst.opc2, inst.OP_IMM_vals).c_str(), arg.c_str(), list[i]);
                break;

                // R64I
            case(0x1000): // SLLI
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 25);
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + "x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_IMM_keys, inst.opc2, inst.OP_IMM_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x5000): // opc3
                switch (list[i] & 0xFE000000) // opcode 3
                {
                case(0x0): // SRLI
                    inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 31);
                    inst.imm = bitCpy(inst.imm, true, list[i], 20, 25);
                    inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                    inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                    arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + "x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                    fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_IMM2_keys, 0x0, inst.OP_IMM2_vals).c_str(), arg.c_str(), list[i]);
                    break;
                case(0x40000000): // SRAI
                    inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 31);
                    inst.imm = bitCpy(inst.imm, true, list[i], 20, 25);
                    inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                    inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                    arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", " + "x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                    fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_IMM2_keys, 0x10, inst.OP_IMM2_vals).c_str(), arg.c_str(), list[i]);
                    break;
                default:
                    fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
                    break;
                }
            }
            break;

            // OP Instructions
        case(0x33):
            inst.opc = 0x33;
            switch (list[i] & 0x7000) // 2nd opcode
            {
            case(0x0):
                inst.opc2 = 0x0;
                switch (list[i] & 0xFE000000) // 3rd opcode
                {
                case(0x0): // Add
                    inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 32);
                    inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                    inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                    inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                    arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                    fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP2_keys, inst.opc3, inst.OP2_vals).c_str(), arg.c_str(), list[i]);
                    break;
                case(0x40000000): // Sub
                    inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 32);
                    inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                    inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                    inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                    arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                    fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP2_keys, inst.opc3, inst.OP2_vals).c_str(), arg.c_str(), list[i]);
                    break;
                default:
                    fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
                    break;
                }
                break;
            case(0x1000): // SLL
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_keys, inst.opc2, inst.OP_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x4000): // XOR
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_keys, inst.opc2, inst.OP_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x5000): // opcode 3
                switch (list[i] & 0xFE000000)
                {
                case(0x0): // SRL
                    inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 32);
                    inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                    inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                    inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                    arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                    fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP3_keys, inst.opc3, inst.OP3_vals).c_str(), arg.c_str(), list[i]);
                    break;
                case(0x40000000): // SRA
                    inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 32);
                    inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                    inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                    inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                    arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                    fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP3_keys, inst.opc3, inst.OP3_vals).c_str(), arg.c_str(), list[i]);
                    break;
                default:
                    fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
                    break;
                }
                break;
            case(0x6000): // OR
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 32);
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_keys, inst.opc2, inst.OP_vals).c_str(), arg.c_str(), list[i]);
                break;

            case(0x7000): // AND
                inst.opc2 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 32);
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                inst.rs2 = bitCpy(inst.rs2, false, list[i], 20, 25);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs2, inst.r_vals) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", x" + std::to_string(inst.rs2);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.OP_keys, inst.opc2, inst.OP_vals).c_str(), arg.c_str(), list[i]);
                break;
            default:
                fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
                break;
            }
            break;

            // W Shift Instructions
        case(0x1B):
            switch (list[i] & 0xFE000000) // opcode 3
            {
            case(0x0): // SRLIW
                inst.opc3 = bitCpy(inst.opc2, false, list[i], 12, 15);
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 25);
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.S_keys, inst.opc3, inst.S_vals).c_str(), arg.c_str(), list[i]);
                break;
            case(0x40000000): // SRAIW
                inst.opc3 = bitCpy(inst.opc3, false, list[i], 25, 32);
                inst.imm = bitCpy(inst.imm, true, list[i], 20, 25);
                inst.rd = bitCpy(inst.rd, false, list[i], 7, 12);
                inst.rs1 = bitCpy(inst.rs1, false, list[i], 15, 20);
                arg = rFlag ? lookup(inst.r_keys, inst.rd, inst.r_vals) + ", " + lookup(inst.r_keys, inst.rs1, inst.r_vals) + ", " + std::to_string(inst.imm) : "x" + std::to_string(inst.rd) + ", x" + std::to_string(inst.rs1) + ", " + std::to_string(inst.imm);
                fprintf(file, "     %-7s%-18s// 0x%08x\n", lookup(inst.S_keys, inst.opc3, inst.S_vals).c_str(), arg.c_str(), list[i]);
                break;
            default:
                fprintf(file, "     inv    invalid           // 0x%08x\n", list[i]);
            }

            // 0
        case(0):
            break;
        }

        //cout << inst.opc << '\n' << inst.opc2 << '\n' << inst.rd << '\n' << inst.rs1 << '\n' << inst.imm << '\n';
    }
    if (strcmp(argv[2], "-") == 1) fclose(file);
    delete[] list; // cleanup memory
    return 0;
}

// Creates array of bytes from the .bin file
int* createList(const char* file_name, int& read)
{
    int counter = 1;
    FILE* myFile = fopen(file_name, "rb");
    if (myFile == nullptr)
    {
        printf("\nFile not opened\n");
        return 0;
    }

    fseek(myFile, 0, SEEK_END);
    counter = ftell(myFile) / sizeof(int);
    rewind(myFile);

    int* inst = new int[counter]; // allocate array based on size of file
    read = counter;               // passes the size of the array

    int x = 0;
    for (int i = 0; !(feof(myFile)); i++)
    {
        fread(&x, sizeof(int), 1, myFile);
        inst[i] = x;
        x = 0;
    }
    fclose(myFile);
    return inst;
}

// Copies bits from one section of a value to the beginning of a specified value
int bitCpy(int a, bool is_neg, int b, int c, int d)
{
    if (is_neg)
    {
        int range = d - c;
        b <<= (32 - d);
        b >>= (32 - range);
        a = b;
        return a;
    }
    int range = d - c;
    b <<= (32 - d);
    b >>= (32 - range);
    if (range == 5) b &= 0x1F; 
    if (range == 3) b &= 0x7;
    if (range == 7) b &= 0x7F;
    a = b;
    return a;
}

// Lookup function for my hacked together dictionary - Takes a set of keys, a key to match, and a set of values to grab based on key
std::string lookup(int keys[], int key, std::string vals[])
{
    for (int i = 0; i < keys[0]; ++i)
    {
        if ((keys[i] == key) && (i != 0))
        {
            return vals[i - 1]; // i - 1 because each set of keys is 1 element larger due to storing the size of the set of values in index 0
        }
    }
    return "Error ";
}