// In-Fixed Notation Expression Evaluator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define NAME_MAX 32
#define VAR_MAX 10
#define FUNC_MAX 16

#define LENGTHOF(array) (sizeof((array))/sizeof((array)[0]))

struct variable {
  char name[NAME_MAX];
	double value;
};

static struct variable gvar[VAR_MAX] = {
	{ "PI", M_PI },
};

struct function {
	char name[NAME_MAX];
	int arguments;
	char *body;
	union native_func {
		double (*func0)(void);
		double (*func1)(double);
		double (*func2)(double,double);
	} native;
};

static struct function gfunc[FUNC_MAX] = {
	{ "sqrt",  1, NULL, { (double (*)(void)) sqrt } },
	{ "cos",   1, NULL, { (double (*)(void)) cos } },
	{ "sin",   1, NULL, { (double (*)(void)) sin } },
	{ "tan",   1, NULL, { (double (*)(void)) tan } },
	{ "pow",   2, NULL, { (double (*)(void)) pow } },
	{ "exp",   1, NULL, { (double (*)(void)) exp } },
	{ "log",   1, NULL, { (double (*)(void)) log } },
	{ "floor", 1, NULL, { (double (*)(void)) floor } },
	{ "ceil",  1, NULL, { (double (*)(void)) ceil } },
	{ "atan",  1, NULL, { (double (*)(void)) atan } },
	{ "atan2", 2, NULL, { (double (*)(void)) atan2 } },
	{ "acos",  1, NULL, { (double (*)(void)) acos } },
	{ "asin",  1, NULL, { (double (*)(void)) asin } },
	{ "fabs",  1, NULL, { (double (*)(void)) fabs } },
};

struct variable * variable_append(struct variable *vs, int nvs, char *name, double value)
{
	int i;
	for (i = 0; i < nvs; i++) if (!vs[i].name[0]) break;
	if (i < nvs) {
		strncpy(vs[i].name, name, NAME_MAX - 1);
		vs[i].value = value;
		return vs + i;
	}
	return NULL;
}

struct variable * variable_find(struct variable *vs, int nvs, char *name)
{
	int i;
	for (i = 0; i < nvs; i++)
		if (strcmp(vs[i].name, name) == 0)
			return vs + i;
	return NULL;
}

struct function * function_find(struct function *fs, int nfs, char *name)
{
	int i;
	for (i = 0; i < nfs; i++)
		if (strcmp(fs[i].name, name) == 0)
			return fs + i;
	return NULL;
}

int debug = 0;
int depth = 0;

char * eatspace(char *s, char **p)
{
	while (*s && isspace(*s)) s++;
	if (p) *p = s;
	return s;
}

// :memo: *p is end pointer for output. 
double fact(char *s, char **p);
double term(char *s, char **p);
double expr(char *s, char **p);

double expr(char *s, char **p)
// ::= term | '+' term | '-' term | expr '+' term | expr '-' term
// === ( '+' | '-' )? term ( ( '+' | '-' ) term )*
{
	double v;
if (debug & 2) fprintf(stderr, "%*s<expr(0X%p):%s\n", (depth++)*2, "", s, s);
	s = eatspace(s, p);
	if (*s == '+') {
		v = term(++s, p);
	} else if (*s == '-') {
		v = - term(++s, p);
	} else {
		v = term(s, p);
		if (s == *p) {
			fprintf(stderr, "expr:error: no first term.\n");
if (debug & 2) fprintf(stderr, "%*s>expr(0X%p):%s\n", (--depth)*2, "", s, s);
			return v;
		}
	}
	s = eatspace(*p, p);
	while (*s) {
		switch (*s) {
		case '+':
			v += term(++s, p);
			eatspace(s, &s);
			if (s == *p) {
				fprintf(stderr, "expr:error: term expected.\n");
			}
			break;
		case '-':
			v -= term(++s, p);
			eatspace(s, &s);
			if (s == *p) {
				fprintf(stderr, "expr:error: term expected.\n");
			}
			break;
		default:
			// end of expression
if (debug & 4) fprintf(stderr, "expr:midstream end.\n");
if (debug & 2) fprintf(stderr, "%*s>expr(0X%p):%s\n", (--depth)*2, "", s, s);
			return v;
		}
		s = eatspace(*p, p);
	}
if (debug & 2) fprintf(stderr, "%*s>expr(0X%p):%s\n", (--depth)*2, "", s, s);
	return v;
}

double term(char *s, char **p)
// ::= fact | term '*' fact | term '/' fact
// === fact ( ( '*' | '/' ) fact )*
{
	double v;
if (debug & 2) fprintf(stderr, "%*s<term(0X%p):%s\n", (depth++)*2, "", s, s);
	s = eatspace(s, p);
	v = fact(s, p);
	if (s == *p) {
	fprintf(stderr, "term:error: no first factor.\n");
if (debug & 2) fprintf(stderr, "%*s>term(0X%p):%s\n", (--depth)*2, "", s, s);
		return 0;
	}
	s = eatspace(*p, p);
	while (*s) {
		switch (*s) {
		case '*':
			v *= fact(++s, p);
			eatspace(s, &s);
			if (s == *p) {
				fprintf(stderr, "term:error: fact expected.\n");
			}
			break;
		case '/':
			v /= fact(++s, p);
			eatspace(s, &s);
			if (s == *p) {
				fprintf(stderr, "term:error: fact expected.\n");
			}
			break;
		default:
if (debug & 4) fprintf(stderr, "term:midstream end.\n");
if (debug & 2) fprintf(stderr, "%*s>term(0X%p):%s\n", (--depth)*2, "", s, s);
			return v;
		}
		s = eatspace(*p, p);
	}
if (debug & 2) fprintf(stderr, "%*s>term(0X%p):%s\n", (--depth)*2, "", s, s);
	return v;
}

double fact(char *s, char **p)
// ::= number | '(' expr ')' | variable | function
{
	double v = 0;
if (debug & 2) fprintf(stderr, "%*s<fact(0X%p):%s\n", (depth++)*2, "", s, s);
	s = eatspace(s, p);
	if (!*s) {
		// input end.
if (debug & 1) fprintf(stderr, "fact:input end.\n");
		// :memo: strchr can search '\0'.
	} else if (strchr(".0123456789", *s)) {
		// Number
		v = strtod(s, p);
if (debug & 4) fprintf(stderr, "fact:Number:*p=0X%p\n", *p);
	} else if (*s == '(') {
if (debug & 4) fprintf(stderr, "fact:LParen:*p=0X%p\n", *p);
		v = expr(++s, p);
		s = eatspace(*p, p);
		if (*s == ')') {
if (debug & 4) fprintf(stderr, "fact:RParen:*p=0X%p\n", *p);
			*p = ++s;
		} else {
			fprintf(stderr, "fact:error: right paren expected.\n");
		}
	} else if (*s == '_' || isalpha(*s)) {
		// Identifier
		char name[NAME_MAX] = { 0 };
if (debug & 4) fprintf(stderr, "fact:Identifier.\n");
		{ // get identifier name
			char *t = s + 1, *e = NULL;
			while (*t == '_' || isalnum(*t)) t++;
			if (t - s >= NAME_MAX) {
				fprintf(stderr, "fact:error: name too long.\n");
				e = t;
				t = s + NAME_MAX - 1;
			}
			strncpy(name, s, t - s);
			if (e) t = e;
			s = t;
		}
		// check if name is id for function
		eatspace(s, &s);
		if (*s == '(') {
			// Function
			struct function *pf;
if (debug & 4) fprintf(stderr, "fact:Identifier:Function.\n");
			pf = function_find(gfunc, LENGTHOF(gfunc), name);
			if (pf) {
				// function found
				int j;
				double *args;
				args = (double*) malloc(pf->arguments * sizeof(double));
				for (j = 0; j < pf->arguments; j++) {
					args[j] = expr(++s, &s);
					if (*s != ',') {
						if (*s != ')') {
							fprintf(stderr, "fact:error: function arguments list end ')' not found.\n");
						} // else, correct end of argments list
						break;
					}
				}
				if (pf->arguments == 0) {
					// special case : no arguments
					j = -1;
					eatspace(++s, &s);
				}
				if (j == pf->arguments - 1 && *s == ')') {
					// correct arguments
					s++;
					if (pf->body) {
						// :todo: user defined function
					} else {
						// native function
						switch (pf->arguments) {
						case 0:
							v = pf->native.func0();
							break;
						case 1:
							v = pf->native.func1(args[0]);
							break;
						case 2:
							v = pf->native.func2(args[0], args[1]);
							break;
						default:
							// not implemented native functions taking 3 or more arguments.
							break;
						}
					}
				} else {
					fprintf(stderr, "fact:error: incorrect function arguments.\n");
				}
				free(args);
			} else {
				fprintf(stderr, "fact:error: function name \"%s\" not found.\n", name);
			}
		} else {
			// Variable
			struct variable *pv;
if (debug & 4) fprintf(stderr, "fact:Identifier:Variable.\n");
			pv = variable_find(gvar, LENGTHOF(gfunc), name);
			if (pv) {
				v = pv->value;
			} else {
				fprintf(stderr, "fact:error: variable \"%s\" is not defined.\n", name);
			}
		}
		*p = s;
	} else {
		// :todo: Unknown
if (debug & 4) fprintf(stderr, "factor:Unknown.\n");
		*p = s;
	}
if (debug & 2) fprintf(stderr, "%*s>fact(0X%p):%s\n", (--depth)*2, "", s, s);
	return v;
}

struct parameter {
	char name[32];
	double range[2];
	int samples;
};

static struct parameter param;

void parameter_set(struct parameter *pp, char *name, double r0, double r1, int ns)
{
	strncpy(pp->name, name, sizeof(pp->name));
	pp->name[sizeof(pp->name)-1] = '\0';
	pp->range[0] = r0;
	pp->range[1] = r1;
	pp->samples = ns;
}

double parameter_eval(struct parameter *pp, int i)
{
	return pp->range[0] + i * (pp->range[1] - pp->range[0]) / (pp->samples - 1);
}

int option_check(int c, char **v, char *s)
{
	int i, n = strlen(s);
	
	if (n > c) return 0;
	
	for (i = 0; i < n; i++) {
		char *t, *p = v[i];
		switch (s[i]) {
		case 'a':
			if (*p == '_' || isalpha(*p)) {
				do { p++; } while (*p == '_' || isalnum(*p));
				if (*p != '\0') return 0;
			} else return 0;
			break;
		case 'f':
			strtod(p, &t);
			if (p == t) return 0;
			break;
		case 'd':
			while (isdigit(*p)) p++;
			if (*p) return 0;
			break;
		case 's':
			break;
		default:
			return 0;
		}
	}
	
	return 1;
}

void option(int *pargc, char ***pargv)
{
	int argc = *pargc;
	char **argv = *pargv;
	char *argv0 = argv[0];
	while (argc > 1) {
		if (strstr(argv[1], "--") == argv[1]) {
			char *p = argv[1] + 2;
			if (strstr(p, "debug") == p) {
				if (argc > 2) {
					if (isdigit(argv[2][0])) {
						debug = atoi(argv[2]);
						argc--; argv++;
					} else {
						fprintf(stderr, "option:error: --debug's argument must be integer.\n");
					}
				} else {
					fprintf(stderr, "option:error: --debug needs integer argument.\n");
				}
			} else if (strstr(p, "param") == p) {
				if (option_check(argc - 2, argv + 2, "assd")) {
					char *e;
					parameter_set(&param, argv[2], expr(argv[3],&e), expr(argv[4],&e), atoi(argv[5]));
					argc -= 4;
					argv += 4;
				} else {
					fprintf(stderr, "option:error: --param arguments incorrect.\n");
				}
			} else {
				// unknown option. skip it.
			}
			argc--; argv++;
		} else {
			break;
		}
	}
	argv[0] = argv0;
	*pargc = argc;
	*pargv = argv;
}

int main(int argc, char *argv[])
{
	double val;
	char *s;
	int i;
	struct variable *pv = NULL;
	
	option(&argc, &argv);
	
	if (argc == 1) {
		printf("usage: infix [ option ] \"expression\" [ \"expression\" ...]\n");
		printf("option:\n");
		printf("\t--debug N\n");
		printf("\t\te.g. --debug 7  (all flags on)\n");
		printf("\t--param name start end samples\n");
		printf("\t\te.g. --param t 0 1 3  (means 0.0, 0.5, 1.0)\n");
		printf("\n");
		printf("predefined native functions:\n");
		printf("\tsqrt(x), cos(x), sin(x), tan(x), pow(x,y),\n");
		printf("\texp(x), log(x), floor(x), ceil(x),\n");
		printf("\tatan(x), atan2(y,x), acos(x), asin(x), fabs(x)\n");
		printf("\n");
		printf("predefined constants:\n");
		printf("\tPI\n");
		return 1;
	}
	
	if (!param.name[0]) {
		param.samples = 1;
	} else {
		pv = variable_append(gvar, VAR_MAX, param.name, 0);
	}
	
	for (i = 0; i < param.samples; i++) {
		int argc_save;
		char **argv_save;
		
		if (pv) {
			pv->value = parameter_eval(&param, i);
			printf("%.15g:", pv->value);
		}
		
		argc_save = argc;
		argv_save = argv;
		
		while (argc > 1) {
			s = argv[1];
if (debug & 2) fprintf(stderr, "argv[%d] = 0X%p\n", argv + 1 - argv_save, argv[1]);
			val = expr(s, &s);
			if (*s) {
				fprintf(stderr, "main:error: parsing stops on %d chars pos.\n", s - argv[1]);
				fprintf(stderr, "%s\n%*s^\n", argv[1], s-argv[1], "");
			}
			printf(" %.15g", val);
			argc--;
			argv++;
		}
		printf("\n");
		
		argc = argc_save;
		argv = argv_save;
	}
	
	return 0;
}
