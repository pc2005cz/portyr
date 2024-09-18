#!/bin/bash

#CFLAGS="-m32 -march=i486"
#CROSS_COMPILE="i486-buildroot-linux-gnu-"
CC=${CROSS_COMPILE}gcc

${CC} ${CFLAGS} portyr.c -o portyr
${CC} ${CFLAGS} portyr.c -o portyr_direct -DDIRECT_IO
