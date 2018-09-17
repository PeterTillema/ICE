#include "defines.h"
#include "parse.h"
#include "ast.h"

#include "operator.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "output.h"
#include "routines.h"
#include "prescan.h"

extern const function_t implementedFunctions[AMOUNT_OF_FUNCTIONS];

#ifndef NDEBUG
void printNodeRecursively(NODE *top, uint8_t depth) {
    if (top != NULL) {
        uint8_t a;
        
        for (a = 0; a < depth; a++) {
            dbg_sprintf(dbgout, " ");
        }
        dbg_sprintf(dbgout, "Type = %d, operand = $%06X\n", top->data.type, top->data.operand.num);
        if (top->child != NULL) {
            for (a = 0; a < depth; a++) {
                dbg_sprintf(dbgout, " ");
            }
            dbg_sprintf(dbgout, "Data:\n");
        }
        printNodeRecursively(top->child, depth + 2);
        if (top->sibling != NULL) {
            for (a = 0; a < depth; a++) {
                dbg_sprintf(dbgout, " ");
            }
            dbg_sprintf(dbgout, "Sibling:\n");
            top = top->sibling;
            printNodeRecursively(top, depth);
        }
    }
}
#endif

NODE *push(NODE *top, element_t data) {
    NODE *tempNode = (NODE*)calloc(1, sizeof(NODE));
    
    tempNode->data = data;
    tempNode->prev = top;
    if (top != NULL) {
        top->sibling = tempNode;
    }
    
    return tempNode;
}

NODE *pop(NODE *top, element_t *data) {
    NODE *tmp = top;
    
    *data = top->data;
    top = top->prev;
    free(tmp);
    
    return top;
}

NODE *insertData(NODE *top, element_t data, uint8_t index) {
    NODE *tempNode = (NODE*)calloc(1, sizeof(NODE));
    uint8_t temp;
    
    tempNode->data = data;
    for (temp = 1; temp < index; temp++) {
        if ((top = top->prev) == NULL) {
            return NULL;
        }
    }
    if (top == NULL) {
        return NULL;
    }
    if (top->prev != NULL) {
        top->prev->sibling = tempNode;
    }
    tempNode->child = top;
    tempNode->prev = top->prev;
    
    return tempNode;
}

NODE *stackToOutput(NODE *outputNode, NODE **stackNode) {
    element_t tempElement;
    
    while (*stackNode != NULL) {
        uint8_t amountOfArgs = 2;
        
        *stackNode = pop(*stackNode, &tempElement);
        if (tempElement.type == TYPE_FUNCTION) {
            uint8_t argsShouldHave = implementedFunctions[tempElement.operand.func.index].amountOfArgs;
            uint8_t function = tempElement.operand.func.function;
            
            amountOfArgs = tempElement.operand.func.amountOfArgs;
            if (argsShouldHave != 255 && argsShouldHave != amountOfArgs) {
                return NULL;
            }
            
            // Hack for L1(..)
            if (function == 0x0F) {
                tempElement.operand.func.function = tLBrace;
            }
            
            // Ignore parenthesis function
            if (function == tLParen) {
                continue;
            }
        }
        if ((outputNode = insertData(outputNode, tempElement, amountOfArgs)) == NULL) {
            return NULL;
        }
    }
    *stackNode = NULL;
    
    return outputNode;
}

uint8_t parseNode(NODE *top) {
    uint8_t type = top->data.type, res = VALID;
    uint24_t operand = top->data.operand.num;
    
    if (top == NULL) {
        return E_SYNTAX;
    }
    
    expr.outputReturnRegister = REGISTER_HL;
    
    if (type <= TYPE_STRING) {
        if (type != TYPE_STRING) {
            expr.outputIsNumber = true;
            expr.outputNumber = operand;
        }
        LD_HL_IMM(operand, type);
    } else if (type == TYPE_VARIABLE) {
        expr.outputIsVariable = true;
        OutputWriteWord(0x27DD);
        OutputWriteByte(operand);
        reg.HLIsNumber = false;
        reg.HLIsVariable = true;
        reg.HLVariable = operand;
    } else if (type == TYPE_OPERATOR) {
        res = parseOperator(top);
    } else if (type == TYPE_FUNCTION) {
        res = parseFunction(top);
    } else {
        res = E_SYNTAX;
    }
    
    return res;
}

NODE *reverseNode(NODE *top) {
    NODE *next = NULL;
    NODE *curr = top;
    NODE *prev = NULL;
    
    while (curr != NULL) {
        next = curr->sibling;
        curr->sibling = prev;
        curr->prev = next;
        prev = curr;
        curr = next;
    }
    
    return prev;
}

NODE *optimizeNode(NODE *top) {
    if (top != NULL) {
        // Optimize child and siblings
        top->child = optimizeNode(top->child);
        top->sibling = optimizeNode(top->sibling);
        
        // Optimize extra parenthesis with a number in it
        if (top->data.type == TYPE_FUNCTION && top->data.operand.func.function == tLParen && top->child->data.type == TYPE_NUMBER) {
            top->data.operand.num = top->child->data.operand.num;
            top->data.type = TYPE_NUMBER;
            free(top->child);
            top->child = NULL;
        }
        
        // Optimize operator with 2 numbers as arguments
        if (top->data.type == TYPE_OPERATOR && top->data.operand.op.op != tStore && 
            top->child->data.type == TYPE_NUMBER && top->child->sibling->data.type == TYPE_NUMBER && 
            !top->child->data.isString && !top->child->sibling->data.isString) {
            top->data.operand.num = executeOperator(top);
            top->data.type = TYPE_NUMBER;
            free(top->child->sibling);
            free(top->child);
            top->child = NULL;
        }
        
        // Optimize function with only numbers as arguments
        if (top->data.type == TYPE_FUNCTION && implementedFunctions[top->data.operand.func.index].numbersArgs) {
            NODE *child = top->child;
            uint8_t loop;
            uint8_t function = top->data.operand.func.function;
            uint8_t function2 = top->data.operand.func.function2;
            
            for (loop = 0; loop < top->data.operand.func.amountOfArgs; loop++) {
                if (child == NULL || child->data.type != TYPE_NUMBER) {
                    goto doNotDeleteFunction;
                }
                child = child->sibling;
            }
            top->data.operand.num = executeFunction(top);
            top->data.type = TYPE_NUMBER;
            freeNode(top->child);
            top->child = NULL;
doNotDeleteFunction:;
        }
    }
    
    return top;
}

void freeNode(NODE *top) {
    if (top->sibling != NULL) {
        freeNode(top->sibling);
    }
    if (top->child != NULL) {
        freeNode(top->child);
    }
    if (top != NULL) {
        free(top);
    }
}