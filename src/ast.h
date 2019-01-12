#ifndef AST_H
#define AST_H

typedef unsigned int num_t;
typedef uint8_t vari_t;
typedef uint8_t op_t;

typedef struct {
    uint8_t function;
    uint8_t amountOfArgs;
    uint8_t function2;
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
    uint8_t   mask;
    operand_t operand;
} element_t;

typedef struct node {
    element_t data;
    struct node *prev;
    struct node *child;
    struct node *sibling;
} NODE;

NODE *push2(NODE*, element_t);
NODE *insertData(NODE*, element_t, uint8_t);
NODE *parseNode(NODE*);
NODE *reverseNode(NODE*);

#endif