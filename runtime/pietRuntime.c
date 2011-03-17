//
//  pietRuntime.c
//  pietc
//
//  Created by Andrew Cobb on 3/17/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>

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
    *stk = cons(val, *stk);
}

stackValue pop(stack stk) {
    list oldList = *stk;
    *stk = cdr(oldList);
    int ret = car(oldList);
    free(oldList);
    return ret;
}

stackValue peek(stack stk) {
    return car(*stk);
}