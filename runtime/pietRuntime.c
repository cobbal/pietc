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
    static void* free_space = space;
    void* ret = free_space;
    free_space += size;
    return ret;
}

void fakefree(void* ptr) {
}

#endif

typedef int32_t stack_value;

typedef struct _list_node {
    stack_value car;
    struct _list_node * cdr;
} *list, **stack;


static list cons(stack_value car, list cdr) {
    list newList = malloc(sizeof(*newList));
    newList->car = car;
    newList->cdr = cdr;
    return newList;
}

static stack_value car(list lst) {
    return lst->car;
}

static list cdr(list lst) {
    return lst->cdr;
}

void push(stack stk, stack_value val) {
    //fprintf(stderr, "push %p\n", stk);
    *stk = cons(val, *stk);
}

stack_value pop(stack stk) {
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

stack_value peek(stack stk) {
    // fprintf(stderr, "peek %p\n", stk);
    return car(*stk);
}

stack new_stack() {
    stack ret = malloc(sizeof(*ret));
    *ret = 0;
    // fprintf(stderr, "newstack %p\n", ret);
    return ret;
}

char stack_has_length(stack stk, int length) {
    list node = *stk;
    int i;
    for (i = 0; i < length; i++) {
        if (!node) {
            fprintf(stderr, "stack does _NOT_ have required length of %d\n", length);
            return 0;
        }
        node = cdr(node);
    }
    fprintf(stderr, "stack has required length of %d\n", length);
    return 1;
}

void roll_stack(stack stk, stack_value rollCount, stack_value depth) {
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
    if (!stack_has_length(stk, depth)) {
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

void putint(stack_value val) {
    printf("%d ", val);
}

stack_value getint() {
    int a;
    printf("<input int: ");
    scanf("%d", &a);
    puts(">");
    return a;
}

void print_list(list list) {
    if (list) {
        print_list(cdr(list));
        fprintf(stderr, "%d ", car(list));
    }
}

void log_op(int operation) {
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
            fprintf(stderr, "NOOP (op %d)", operation);
            break;
    }
}

void log_stuff(int operation, char * from, char * to, int dp, int cc, stack stk) {
    fprintf(stderr, "stack = ");
    print_list(*stk);
    fprintf(stderr, "\n%s -(%c%c)-> %s operation ", from, "RDLU"[dp], "lr"[cc], to);
    log_op(operation);
    fprintf(stderr, "\n");
}
