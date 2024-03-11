#!/bin/bash

source trace_bash

alert     "hello from line $LINENO"
critical  "hello from line $LINENO"
error     "hello from line $LINENO"
warning   "hello from line $LINENO"
notice    "hello from line $LINENO"
info      "hello from line $LINENO - test DEBUG+1 to stdout via: env -i PATH=\$PATH TRACE_LVLS=-1 `basename $0`"
log       "hello from line $LINENO"
debug     "hello from line $LINENO"
debug -l1 "hello from line $LINENO"
debug -l2 "hello from line $LINENO"
debug -l3 "hello from line $LINENO"
debug -l4 "hello from line $LINENO"
debug -l5 "hello from line $LINENO"
debug -l6 "hello from line $LINENO"
debug -l7 "hello from line $LINENO"
debug -l8 "hello from line $LINENO"
debug -l9 "hello from line $LINENO"
fatal     "hello from line $LINENO"
