#include <stdio.h>
/* проверка на запредельность */

#define MAXNUM	99
#define DBL	1
#define INT	2

#define MAXTOPL	17	/* сколько всего пар топлив есть */

extern double	k,	/* показатель процесса */
	Tk,		/* температура сгорания */
	pk,		/* давление в камере */
	Dkr,		/* критика */
	akr,		/* ск. звука в критике */
	*x,		/* координата */
	*d,	        /* диаметр */
	*T,		/* температура газа */
	*P,
	*Tct,		/* температура стенки со стороны газа */
	*Tctg,		/* температура стенки со стороны жидкости */
	Rg,		/* газовая постоянная */
	*Aok,		/* избыток окислителя */
	Aok1,
	*La,		/* коэффициент скорости */
	*Q,		/* конвективный тепловой поток */
	*Ql,		/* лучевой тепловой поток */
	Ql0,		/* лучистый тепловой поток В КАМЕРЕ */
	*Tg,		/* температура жидкости */
	*G1,		/* расходы 1 и 2 охладителей */
	*To1,		/* температуры 1 и 2 охладителей */
	*Alp,		/* ЭФФЕКТИВНЫЙ к-т теплоотдачи */
	maxT[MAXTOPL],
	*ACq,		/* часть Qk */
	*Dg,		/* гидр. диаметр */
	*Dg02,		/* то же в степени 0.2 */
	*fS,		/* сечение тракта */
	*roW,
	*dS,
	*Tg0,
	*Re,
	*Eta,
	*tN,
	*Ak,		/* ширина тракта */
	*Hk,		/* высота тракта */
	*Delr,		/* толщина ребра или гофра */
	*Delo,		/* толщина огневой стенки */
	*Deln,		/* толщина неогневой стенки */
	*Gam,		/* угол наклона гофра */
        *Bet,           /* угол закрутки ребер */
        *dP;            /* потери давления */

extern int tse [MAXNUM],	/* тип охладителя */
	   mato[MAXNUM],	/* материал огневой стенки */
	   matn[MAXNUM];	/* материал неогневнй стенки */

extern int	n,	/* количество участков разбиения */
	nkr,		/* номер критического сечения */
	i1,		/* номер пары горючего */
	i2,		/* номер охладителя */
	maxiter,	/* максимальное количество итераций */
	*vx,
	*tx,
	*ptx,
	*ox, 	/* вход, тип, выход 1 охладителя */
	*np,
	vm,
	om,
	uo;	/* текущий участок охлаждения */

extern FILE * fout;

/* глобальные функции */
void error(int,int,...);
double Rgc(int);
double SS(int);
void graph(void), clg(void);
double Tog(int);

double Ela(double,int);

double Lam(double,int), Cp(double,int), Mu(double,int), Ro(double, int);
void set_Q(int);

#define getDBL() *(double*)get_token(DBL)
#define getINT() *(int*)get_token(INT)
void * get_token(int);

#define PI		3.1415926

#define GRAD_RAD        0.0174533	/* к-т перехода к радианам */

/* математика */
double dihotom(double,double,double,double (*f)());
double horner(double x, int n, double * ai);
double sp(int pts, double * sx, double * sy, double * css, double x1);
int cs(int pts, double * sx, double * sy, double * css);
double cs_sp(int n, double * x, double * y, double x1);
double lin(int pts, double * sx, double * sy, double x1);

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif