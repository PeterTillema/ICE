#include "defines.h"
#include "ast.h"

NODE *push2(NODE *top, element_t data) {
    NODE *tempNode = (NODE *) calloc(1, sizeof(NODE));

    tempNode->data = data;
    tempNode->prev = top;
    top->sibling = tempNode;

    return tempNode;
}

NODE *insertData(NODE *top, element_t data, uint8_t index) {
    NODE *tempNode = (NODE *) calloc(1, sizeof(NODE));
    uint8_t temp;

    tempNode->data = data;
    for (temp = 1; temp < index; temp++) {
        top = top->prev;

        if (top == NULL) {
            return NULL;
        }
    }
    if (top == NULL) {
        return NULL;
    }
    top->prev->sibling = tempNode;
    tempNode->child = top;

    return tempNode;
}

NODE *parseNode(NODE *top) {
    return NULL;
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