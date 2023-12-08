#ifndef GLOBALS_H
#define GLOBALS_H

#ifdef LOCAL
#define EXTERN
#else
#define EXTERN extern
#endif

#define SIZE 10
EXTERN int num;
EXTERN int N;
EXTERN char s[SIZE];

#endif