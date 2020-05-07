infix
=====

In-Fix Notation Expression Evaluator.

COMPILE:
  MinGW:
    gcc -Wall -o infix.exe infix.c

Execute:
  Example 1:
    Command Line:
      infix "1+1/3"
    Output:
      1.33333333333333
  Example 2:
    Command Line:
      infix --param t 0 "2 * PI" 7 "cos(t) + t * sin(t)" "sin(t) - t * cos(t)"
    Output:
      0: 1 0
      1.0471975511966: 1.40689968211711 0.34242662818614
      2.0943951023932: 1.31379936423422 1.91322295498104
      3.14159265358979: -1 3.14159265358979
      4.18879020478639: -4.12759872846843 1.22836969860876
      5.23598775598299: -4.03449841058554 -3.48401928177593
      6.28318530717959: 0.999999999999998 -6.28318530717959
