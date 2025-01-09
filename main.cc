#include <unordered_map>
#include "execute.h"
#include "lexer.h"

using namespace std;

LexicalAnalyzer lexer;
unordered_map<string, int> location_table;

struct ExprResult
{
    Token first;
    Token op;
    Token second;
};

void parse_var_list();
struct InstructionNode* parse_body();
void parse_input();

int get_location(string name)
{
    if (location_table.find(name) == location_table.end())
    {
        location_table[name] = next_available++;
    }
    return location_table[name];
}

int set_location(int num)
{
    mem[next_available] = num;

    return next_available++;
}

Token expect(TokenType target)
{
    if (lexer.peek(1).token_type != target)
    {
        exit(1);
    }
    return lexer.GetToken();
}

struct InstructionNode * parse_Generate_Intermediate_Representation()
{
    parse_var_list();
    struct InstructionNode* program = parse_body();
    parse_input();

    return program;
}

void parse_id_list()
{
    Token t = expect(ID);

    get_location(t.lexeme);
    if(lexer.peek(1).token_type == COMMA)
    {
        expect(COMMA);
        parse_id_list();
    }
}

void parse_var_list()
{
    parse_id_list();
    expect(SEMICOLON);
}

Token parse_primary()
{
    Token t;
    if(lexer.peek(1).token_type == ID) t = expect(ID);
    else t = expect(NUM);

    return t;
}

Token parse_op()
{
    Token t;

    switch(lexer.peek(1).token_type)
    {
        case PLUS: t = expect(PLUS); break;
        case MINUS: t = expect(MINUS); break;
        case MULT: t = expect(MULT); break;
        case DIV: t = expect(DIV); break;
    }
    return t;
}

ExprResult parse_expr()
{
    ExprResult result;
    result.first = parse_primary();
    result.op = parse_op();
    result.second = parse_primary();
    return result;
}

struct InstructionNode* parse_assign_stmt()
{
    struct InstructionNode * inst = new InstructionNode;
    inst->type = ASSIGN;
    inst->next = nullptr;

    Token t;
    t = expect(ID);

    inst->assign_inst.left_hand_side_index = get_location(t.lexeme);
    expect(EQUAL);

    if(lexer.peek(2).token_type == SEMICOLON)
    {
        t = parse_primary();
        inst->assign_inst.op = OPERATOR_NONE;

        if(t.token_type == ID) inst->assign_inst.opernd1_index = get_location(t.lexeme);
        else inst->assign_inst.opernd1_index = set_location(stoi(t.lexeme));

    }
    else
    {
        ExprResult expr = parse_expr();

        switch(expr.op.token_type)
        {
            case PLUS: inst->assign_inst.op = OPERATOR_PLUS; break;
            case MINUS: inst->assign_inst.op = OPERATOR_MINUS; break;
            case MULT: inst->assign_inst.op = OPERATOR_MULT; break;
            case DIV: inst->assign_inst.op = OPERATOR_DIV; break;
        }

        if(expr.first.token_type == ID) inst->assign_inst.opernd1_index = get_location(expr.first.lexeme);
        else inst->assign_inst.opernd1_index =  set_location(stoi(expr.first.lexeme));

        if(expr.second.token_type == ID) inst->assign_inst.opernd2_index = get_location(expr.second.lexeme);
        else inst->assign_inst.opernd2_index =  set_location(stoi(expr.second.lexeme));
    }

    expect(SEMICOLON);
    return inst;

}

Token parse_relop()
{
    Token t;
    switch(lexer.peek(1).token_type)
    {
        case GREATER: t = expect(GREATER); break;
        case LESS: t = expect(LESS); break;
        case NOTEQUAL: t = expect(NOTEQUAL); break;
    }
    return t;
}

ExprResult parse_condition()
{
    ExprResult relation;
    relation.first = parse_primary();
    relation.op = parse_relop();
    relation.second = parse_primary();
    return relation;
}


struct InstructionNode* parse_for_stmt()
{
    expect(FOR);
    expect(LPAREN);

    struct InstructionNode* init = parse_assign_stmt();

    ExprResult cond = parse_condition();
    expect(SEMICOLON);

    struct InstructionNode* incr = parse_assign_stmt();
    expect(RPAREN);

    struct InstructionNode* cjmp = new InstructionNode;
    cjmp->type = CJMP;

    switch(cond.op.token_type)
    {
        case GREATER: cjmp->cjmp_inst.condition_op = CONDITION_GREATER; break;
        case LESS: cjmp->cjmp_inst.condition_op = CONDITION_LESS; break;
        case NOTEQUAL: cjmp->cjmp_inst.condition_op = CONDITION_NOTEQUAL; break;
    }

    if(cond.first.token_type == ID) cjmp->cjmp_inst.opernd1_index = get_location(cond.first.lexeme);
    else cjmp->cjmp_inst.opernd1_index = set_location(stoi(cond.first.lexeme));

    if(cond.second.token_type == ID) cjmp->cjmp_inst.opernd2_index = get_location(cond.second.lexeme);
    else cjmp->cjmp_inst.opernd2_index = set_location(stoi(cond.second.lexeme));

    struct InstructionNode* body = parse_body();

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;

    struct InstructionNode* jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = cjmp;
    jmp->next = noop;

    init->next = cjmp;
    cjmp->next = body;
    cjmp->cjmp_inst.target = noop;

    struct InstructionNode* last_body = body;
    while(last_body->next != nullptr) last_body = last_body->next;
    last_body->next = incr;

    incr->next = jmp;

    return init;
}

struct InstructionNode* parse_while_stmt()
{
    struct InstructionNode* inst = new InstructionNode;
    inst->type = CJMP;

    expect(WHILE);
    ExprResult condition = parse_condition();

    switch(condition.op.token_type)
    {
        case GREATER: inst->cjmp_inst.condition_op = CONDITION_GREATER; break;
        case LESS: inst->cjmp_inst.condition_op = CONDITION_LESS; break;
        case NOTEQUAL: inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL; break;
    }

    if(condition.first.token_type == ID) inst->cjmp_inst.opernd1_index = get_location(condition.first.lexeme);
    else inst->cjmp_inst.opernd1_index = set_location(stoi(condition.first.lexeme));

    if(condition.second.token_type == ID) inst->cjmp_inst.opernd2_index = get_location(condition.second.lexeme);
    else inst->cjmp_inst.opernd2_index = set_location(stoi(condition.second.lexeme));

    struct InstructionNode* while_body = parse_body();

    struct InstructionNode* jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = inst;


    InstructionNode* last = while_body;
    while(last->next != nullptr) last = last->next;
    last->next = jmp;

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;

    inst->next = while_body;
    inst->cjmp_inst.target = noop;

    jmp->next = noop;
    return inst;
}


struct InstructionNode* parse_if_stmt()
{
    struct InstructionNode* inst = new InstructionNode;
    inst->type = CJMP;

    expect(IF);
    ExprResult condition = parse_condition();

    switch(condition.op.token_type)
    {
        case GREATER: inst->cjmp_inst.condition_op = CONDITION_GREATER; break;
        case LESS: inst->cjmp_inst.condition_op = CONDITION_LESS; break;
        case NOTEQUAL: inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL; break;
    }

    if(condition.first.token_type == ID) inst->cjmp_inst.opernd1_index = get_location(condition.first.lexeme);
    else inst->cjmp_inst.opernd1_index =  set_location(stoi(condition.first.lexeme));

    if(condition.second.token_type == ID) inst->cjmp_inst.opernd2_index = get_location(condition.second.lexeme);
    else inst->cjmp_inst.opernd2_index = set_location(stoi(condition.second.lexeme));

    struct InstructionNode* if_body = parse_body();

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;
    inst->next = if_body;


    InstructionNode* last = if_body;
    while(last->next != nullptr) last = last->next;
    last->next = noop;

    inst->cjmp_inst.target = noop;
    return inst;
}

struct InstructionNode* parse_default(InstructionNode* jump)
{
    expect(DEFAULT);
    expect(COLON);

    struct InstructionNode* body = parse_body();
    if (body != nullptr)
    {
        struct InstructionNode* temp = body;
        while (temp != nullptr && temp->next != nullptr) temp = temp->next;
        if (temp != nullptr) temp->next = jump;
    }

    return body;
}

struct InstructionNode* parse_case(InstructionNode* jump, int op1)
{
    struct InstructionNode* inst = new InstructionNode;
    inst->next = nullptr;
    inst->type = CJMP;

    inst->cjmp_inst.opernd1_index = op1;
    inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    expect(CASE);

    Token t = expect(NUM);
    inst->cjmp_inst.opernd2_index = set_location(stoi(t.lexeme));

    expect(COLON);

    struct InstructionNode* body = parse_body();
    if (body != nullptr)
    {
        inst->cjmp_inst.target = body;

        struct InstructionNode* temp = body;
        while (temp != nullptr && temp->next != nullptr) temp = temp->next;
        if (temp != nullptr) temp->next = jump;
    }

    return inst;
}

struct InstructionNode* parse_case_list(InstructionNode* jump, int op1)
{
    struct InstructionNode* first = parse_case(jump, op1);
    struct InstructionNode* current = first;

    while (lexer.peek(1).token_type == CASE)
    {
        struct InstructionNode* rest = parse_case(jump, op1);
        if (rest != nullptr)
        {
            current->next = rest;
            current = rest;
        }
    }

    if (lexer.peek(1).token_type == DEFAULT)
    {
        struct InstructionNode* default_case = parse_default(jump);
        if (default_case != nullptr) current->next = default_case;
    }

    return first;
}

struct InstructionNode* parse_switch_stmt()
{
    expect(SWITCH);

    Token t = expect(ID);
    int op1 = get_location(t.lexeme);

    expect(LBRACE);

    struct InstructionNode* noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;

    struct InstructionNode* jump = new InstructionNode;
    jump->type = JMP;
    jump->next = nullptr;
    jump->jmp_inst.target = noop;

    struct InstructionNode* inst = parse_case_list(jump, op1);

    if (inst != nullptr)
    {
        struct InstructionNode* temp = inst;
        while (temp != nullptr && temp->next != nullptr) temp = temp->next;
        if (temp != nullptr) temp->next = noop;
    }

    expect(RBRACE);


    return inst;
}

struct InstructionNode* parse_output_stmt()
{
    struct InstructionNode* inst = new InstructionNode;
    inst->type = OUT;
    inst->next = nullptr;

    expect(OUTPUT);
    Token t = expect(ID);
    inst->output_inst.var_index = get_location(t.lexeme);
    expect(SEMICOLON);

    return inst;
}

struct InstructionNode* parse_input_stmt()
{
    struct InstructionNode* inst = new InstructionNode;
    inst->type = IN;
    inst->next = nullptr;

    expect(INPUT);
    Token t = expect(ID);
    inst->input_inst.var_index = get_location(t.lexeme);
    expect(SEMICOLON);

    return inst;
}

struct InstructionNode* parse_stmt()
{
    switch(lexer.peek(1).token_type)
    {
        case ID: return parse_assign_stmt();
        case WHILE: return parse_while_stmt();
        case IF: return parse_if_stmt();
        case SWITCH: return parse_switch_stmt();
        case FOR: return parse_for_stmt();
        case OUTPUT: return parse_output_stmt();
        case INPUT: return parse_input_stmt();
        default: exit(1);
    }
    return nullptr;
}

struct InstructionNode* parse_stmt_list()
{
    struct InstructionNode* first = parse_stmt();
    if(lexer.peek(1).token_type != RBRACE)
    {
        struct InstructionNode* rest = parse_stmt_list();
        InstructionNode *g = first;
        while(g->next != nullptr) g = g->next;
        g->next = rest;
    }
    return first;
}

struct InstructionNode* parse_body()
{
    expect(LBRACE);
    struct InstructionNode* body = parse_stmt_list();
    expect(RBRACE);
    return body;
}

void parse_num_list()
{
    Token t;
    t = expect(NUM);
    inputs.push_back(stoi(t.lexeme));
    if(lexer.peek(1).token_type != END_OF_FILE) parse_num_list();
}

void parse_input()
{
    parse_num_list();
}
