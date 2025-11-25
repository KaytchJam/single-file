#include <stdio.h>

/** Struct that holds two integers and two "methods" */
typedef struct Operation {
    int a;
    int b;
    int (*op1)(struct Operation*);
    int (*op2)(struct Operation*);
} Operation;

typedef int (*BinaryOp)(Operation*);

/** Add the two integers in Operation */
int add(Operation* op) {
    return op->a + op->b;
}

/** Multiply thw two integers in Operation */
int mul(Operation* op) {
    return op->a * op->b;
}

/** """Constructor""" for `struct Operation` */
Operation new_operation(int a, int b) {
    Operation op = {
        a,
        b,
        add,
        mul
    };
    
    return op;
}

int main() {
    Operation add_op = new_operation(10, 20);
    printf("%d\n", add_op.op1(&add_op));
    printf("%d\n", add_op.op2(&add_op));
    return 0;
}