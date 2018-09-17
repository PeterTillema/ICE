#ifndef AST_H
#define AST_H

typedef uint24_t num_t;
typedef uint8_t vari_t;

typedef struct {
    uint8_t op;
    uint8_t precedence;
    uint8_t index;
} op_t;

typedef struct {
    uint8_t function;
    uint8_t function2;
    uint8_t amountOfArgs;
    uint8_t index;
} func_t;

typedef union {
    num_t num;
    vari_t var;
    op_t op;
    func_t func;
} operand_t;

typedef struct {
    uint8_t   isString;
    uint8_t   type;
    operand_t operand;
} element_t;

typedef struct node {
    element_t data;
    struct node *prev;
    struct node *child;
    struct node *sibling;
} NODE;

void printNodeRecursively(NODE*, uint8_t);
NODE *push(NODE*, element_t);
NODE *pop(NODE *top, element_t*);
NODE *insertData(NODE*, element_t, uint8_t);
NODE *stackToOutput(NODE*, NODE**);
uint8_t parseNode(NODE*);
NODE *reverseNode(NODE*);
NODE *optimizeNode(NODE*);
void freeNode(NODE*);

#endif