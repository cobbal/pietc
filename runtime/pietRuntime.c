//
//  pietRuntime.c
//  pietc
//
//  Created by Andrew Cobb on 3/17/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>

#if 0 // fake malloc and free

#define malloc fakemalloc
#define free fakefree

void* fakemalloc(int size) {
    static char space[1000];
    static void* freeSpace = space;
    void* ret = freeSpace;
    freeSpace += size;
    return ret;
}

void fakefree(void* ptr) {
}

#endif

typedef int32_t stackValue;

typedef struct _listNode {
    stackValue car;
    struct _listNode * cdr;
} *list, **stack;


static list cons(stackValue car, list cdr) {
    list newList = malloc(sizeof(*newList));
    newList->car = car;
    newList->cdr = cdr;
    return newList;
}

static stackValue car(list lst) {
    return lst->car;
}

static list cdr(list lst) {
    return lst->cdr;
}

void push(stack stk, stackValue val) {
    //fprintf(stderr, "push %p\n", stk);
    *stk = cons(val, *stk);
}

stackValue pop(stack stk) {
    //fprintf(stderr, "pop %p %p\n", stk, *stk);
    if (*stk == 0) {
        return 0;
    }
    list oldList = *stk;
    *stk = cdr(oldList);
    int ret = car(oldList);
    free(oldList);
    return ret;
}

stackValue peek(stack stk) {
    // fprintf(stderr, "peek %p\n", stk);
    return car(*stk);
}

stack new_stack() {
    stack ret = malloc(sizeof(*ret));
    *ret = 0;
    // fprintf(stderr, "newstack %p\n", ret);
    return ret;
}

void roll_stack(stack stk, stackValue rollCount, stackValue depth) {
    if (depth <= 0) {
        return;
    }
    
    // ensure 0 ≤ rollCount < depth
    rollCount %= depth;
    rollCount += depth;
    rollCount %= depth;
    
    if (rollCount == 0) {
        return;
    }
    
    list bottom = *stk;
    list roller = *stk;
    for (;depth > 1; depth--) {
        bottom = cdr(bottom);
    }
    for (;rollCount > 1; rollCount--) {
        roller = cdr(roller);
    }
    list temp = roller->cdr;
    roller->cdr = bottom->cdr;
    bottom->cdr = *stk;
    *stk = temp;
}

void putint(stackValue val) {
    printf("%d ", val);
}

stackValue getint() {
    int a;
    printf("<input int: ");
    scanf("%d", &a);
    puts(">");
    return a;
}

void printList(list list) {
    if (list) {
        printList(cdr(list));
        fprintf(stderr, "%d ", car(list));
    }
}

void logOp(int operation) {
    switch(operation) {
        case 1:
            fprintf(stderr, "PUSH");
            break;
        case 2:
            fprintf(stderr, "POP");
            break;
        case 3:
            fprintf(stderr, "ADD");
            break;
        case 4:
            fprintf(stderr, "SUBTRACT");
            break;
        case 5:
            fprintf(stderr, "MULTIPLY");
            break;
        case 6:
            fprintf(stderr, "DIVIDE");
            break;
        case 7:
            fprintf(stderr, "MOD");
            break;
        case 8:
            fprintf(stderr, "NOT");
            break;
        case 9:
            fprintf(stderr, "GREATER");
            break;
        case 10:
            fprintf(stderr, "POINTER");
            break;
        case 11:
            fprintf(stderr, "SWITCH");
            break;
        case 12:
            fprintf(stderr, "DUPLICATE");
            break;
        case 13:
            fprintf(stderr, "ROLL");
            break;
        case 14:
            fprintf(stderr, "IN(NUM)");
            break;
        case 15:
            fprintf(stderr, "IN(CHAR)");
            break;
        case 16:
            fprintf(stderr, "OUT(NUM)");
            break;
        case 17:
            fprintf(stderr, "OUT(CHAR)");
            break;
        case 0:
        default:
            fprintf(stderr, "ERROR (op %d)", operation);
            break;
    }
}

void logStuff(int operation, char * from, char * to, stack stk) {
    fprintf(stderr, "stack = ");
    printList(*stk);
    fprintf(stderr, "\n%s -> %s operation ", from, to);
    logOp(operation);
    fprintf(stderr, "\n");
}