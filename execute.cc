#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <string>
#include "execute.h"
#include <iostream>

using namespace std;

int mem[1000];
int next_available = 0;

std::vector<int> inputs;
int next_input = 0;

void print(struct InstructionNode *head)
{
    struct InstructionNode *current = head;
    int instruction_number = 0;

    while (current != nullptr)
    {
        printf("Instruction %d: ", instruction_number++);

        switch (current->type)
        {
        case NOOP:
            printf("NOOP\n");
            break;

        case IN:
            printf("IN var[%d]\n", current->input_inst.var_index);
            break;

        case OUT:
            printf("OUT var[%d]\n", current->output_inst.var_index);
            break;

        case ASSIGN:
            printf("ASSIGN: var[%d] = ", current->assign_inst.left_hand_side_index);
            printf("var[%d] ", current->assign_inst.opernd1_index);

            switch (current->assign_inst.op)
            {
            case OPERATOR_PLUS:
                printf("+ ");
                break;
            case OPERATOR_MINUS:
                printf("- ");
                break;
            case OPERATOR_MULT:
                printf("* ");
                break;
            case OPERATOR_DIV:
                printf("/ ");
                break;
            case OPERATOR_NONE:
                break;
            }

            if (current->assign_inst.op != OPERATOR_NONE)
            {
                printf("var[%d]", current->assign_inst.opernd2_index);
            }
            printf("\n");
            break;

        case CJMP:
            printf("CJMP: if var[%d] ", current->cjmp_inst.opernd1_index);
            switch (current->cjmp_inst.condition_op)
            {
            case CONDITION_GREATER:
                printf("> ");
                break;
            case CONDITION_LESS:
                printf("< ");
                break;
            case CONDITION_NOTEQUAL:
                printf("!= ");
                break;
            }
            break;

        case JMP:
            printf("JMP to inst %p\n", current->jmp_inst.target);
            break;

        default:
            printf("UNKNOWN INSTRUCTION TYPE\n");
            break;
        }

        current = current->next;
    }
    printf("End of instruction list\n\n");
}

void execute_program(struct InstructionNode * program)
{
    struct InstructionNode * pc = program;
    int op1, op2, result;

    while(pc != NULL)
    {
        switch(pc->type)
        {
            case NOOP:
                pc = pc->next;
                break;
            case IN:

                mem[pc->input_inst.var_index] = inputs[next_input];
                next_input++;
                pc = pc->next;
                break;
            case OUT:
                printf("%d ", mem[pc->output_inst.var_index]);
		fflush(stdin);
                pc = pc->next;
                break;
            case ASSIGN:
                switch(pc->assign_inst.op)
                {
                    case OPERATOR_PLUS:
                        op1 = mem[pc->assign_inst.opernd1_index];
                        op2 = mem[pc->assign_inst.opernd2_index];
                        result = op1 + op2;
                        break;
                    case OPERATOR_MINUS:
                        op1 = mem[pc->assign_inst.opernd1_index];
                        op2 = mem[pc->assign_inst.opernd2_index];
                        result = op1 - op2;
                        break;
                    case OPERATOR_MULT:
                        op1 = mem[pc->assign_inst.opernd1_index];
                        op2 = mem[pc->assign_inst.opernd2_index];
                        result = op1 * op2;
                        break;
                    case OPERATOR_DIV:
                        op1 = mem[pc->assign_inst.opernd1_index];
                        op2 = mem[pc->assign_inst.opernd2_index];
                        result = op1 / op2;
                        break;
                    case OPERATOR_NONE:
                        op1 = mem[pc->assign_inst.opernd1_index];
                        result = op1;
                        break;
                }
                mem[pc->assign_inst.left_hand_side_index] = result;
                pc = pc->next;

                break;
            case CJMP:
                if (pc->cjmp_inst.target == NULL)
                {
                    debug("Error: pc->cjmp_inst->target is null.\n");
                    exit(1);
                }

                op1 = mem[pc->cjmp_inst.opernd1_index];
                op2 = mem[pc->cjmp_inst.opernd2_index];
                switch(pc->cjmp_inst.condition_op)
                {
                    case CONDITION_GREATER:
                        if(op1 > op2)
                            pc = pc->next;
                        else
                            pc = pc->cjmp_inst.target;
                        break;
                    case CONDITION_LESS:
                        if(op1 < op2)
                            pc = pc->next;
                        else
                            pc = pc->cjmp_inst.target;
                        break;
                    case CONDITION_NOTEQUAL:
                        if(op1 != op2)
                            pc = pc->next;
                        else
                            pc = pc->cjmp_inst.target;
                        break;
                }
                break;
            case JMP:

                if (pc->jmp_inst.target == NULL)
                {
                    debug("Error: pc->jmp_inst->target is null.\n");
                    exit(1);
                }
                pc = pc->jmp_inst.target;
                break;
            default:
                debug("Error: invalid value for pc->type (%d).\n", pc->type);
                exit(1);
                break;
        }
    }
}

int main()
{
    struct InstructionNode * program;
    program = parse_Generate_Intermediate_Representation();
    print(program);
    execute_program(program);
    return 0;
}
