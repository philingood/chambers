#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <conio.h>
#include <stdarg.h>

#include <graphics.h> 

#include "hol3.h"

/* сначала считываются координата, диаметр и к-т избытка окислителя
   память под остальные переменные выделяется потом */

double	k,	/* показатель процесса */
	Tk,		/* температура сгорания */
	pk,		/* давление в камере */
	Dkr,		/* критика */
	akr,		/* ск. звука в критике */
	*x,		/* координата */
	*d,		/* диаметр */
	*T,		/* температура газа */
	*P,
	*Tct,		/* температура стенки со стороны газа */
	*Tctg,		/* температура стенки со стороны жидкости */
	Rg,		/* газовая постоянная */
	*Aok,		/* избыток окислителя в пристенке */
	Aok1,		/* в ядре */
	*La,		/* коэффициент скорости */
	*Q,		/* конвективный тепловой поток */
	*Ql,		/* лучевой тепловой поток */
	Ql0,		/* лучистый тепловой поток В КАМЕРЕ */
	*Tg,		/* температура жидкости */
	*G1,		/* расходы 1 и 2 охладителей */
	*To1,		/* температуры 1 и 2 охладителей */
	*Alp,		/* ЭФФЕКТИВНЫЙ к-т теплоотдачи */
	*ACq,		/* часть Qk */
	*Dg,		/* гидр. диаметр */
	*Dg02,
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

int	n,	/* количество участков разбиения */
	nkr,	/* критическое сечение */
	i1,	/* номер пары горючего */
	i2,
        maxiter=10,     /* итераций по умолчанию */
	*vx,
	*tx,
	*ox,	/* вход, тип, выход охладителя */
	*ptx,	/* петля охладителя */
	*np,	/* количество ребер */
	vm,	/* минимальный номер участка охлаждения */
	om,	/* максимальный номер участка охлаждения */
	uo;	/* текущий участок охлаждения */

FILE * fout, *fin;

double maxT[]={		/* максимально допустимые температуры */
	350,	/* AK */
	340,	/* AK-27 */
	580,	/* керосин Т-1 */
	460,	/* тонка */
	580,	/* НДМГ */
	420,	/* N2O4 */
	340,	/* аэрозин */
	373,	/* вода */
	330,	/* АК-40 */
	325,	/* ММГ */
	150,	/* O2ж */
	95,	/* F2ж */
	420,	/* H2O2 */
	490,	/* спирт 93.5% */
	360,	/* C10H16 */
	460,	/* керосин */
	1200	/* H2ж и газ*/
	};

extern double prec;

void main(int argc, char ** argv)
{
void read_data(void);
int calc_T(void);
void calc_geom(void),calc_preQ(void),print_data(void);
int iter(void);
int ii,iuo;
char bf[100];

// cprintf(" Программа расчета наружного проточного охлаждения ЖРД\n\r"
printf(" Программа расчета наружного проточного охлаждения ЖРД\n\r"
       " ХОЛОД - 3 PC. (c) Затонский А.В. МГТУ Э-1 %s\n\r",__DATE__);
/*
cprintf(" ЭТА ВЕРСИЯ ПРЕДНАЗНАЧЕНА ДЛЯ ОТРАБОТКИ АЛГОРИТМА И ПРОВЕРКИ\n\r"
       " БАЗЫ ДАННЫХ ТЕПЛОФИЗИЧЕСКИХ СВОЙСТВ КОМПОНЕНТОВ ТОПЛИВА.\n\r"
       " РЕКОМЕНДУЕТСЯ ПОЛЬЗОВАТЬСЯ С ОСТОРОЖНОСТЬЮ И СООБЩИТЬ\n\r"
       " О ВСЕХ ЗАМЕЧЕННЫХ НЕДОСТАТКАХ.\n\r");
*/

if(argc<2){
/*
	cprintf(" Вызов: HOLOD3 <имя файла данных> <max итераций>\n\r");
	return;
*/
	printf(" Имя файла с данными: ");
	gets(bf);
	printf(" Максимальное количество итераций: ");
	scanf("%d",&maxiter);
	}
else
	strcpy(bf,argv[1]);

if(argc>2)
	{
	maxiter = atoi(argv[2]);
	if(maxiter<10)
		{
		error(0,22,maxiter);
		maxiter = 10;
		}
	if(maxiter>100)
		{
		error(0,23,maxiter);
		maxiter=100;
		}
	}
if((fin=fopen(bf,"rt"))==NULL){
	// cprintf(" Не могу открыть файл данных %s\n\r",bf);
	printf(" Не могу открыть файл данных %s\n\r",bf);
	return;
	}
if((fout=fopen("holod3.rez","wt"))==NULL){
	// cprintf(" Не могу открыть файл результатов: вывод будет\n\r"
	printf(" Не могу открыть файл результатов: вывод будет\n\r"
	       " роизводиться на экран. Нажмите ENTER для про-\n\r"
	       " должения или ESC для прерывания программы\n\r");
	if(getchar()==27)
		exit(0);
	fout=stdout;
	}
 /* Внимание! Две следующие строки требуют компоновки с библиотекой
    GRAPHADV.LIB == GRAPHICS.LIB + CGA.OBJ + EGAVGA.OBJ.
    Чтобы компоновать с GRAPHICS.LIB надо закомментировать их, а
    в рабочую директорию подписать CGA.BGI / EGAVGA.BGI        */
//  if(registerbgidriver(CGA_driver)<0) return;           /* !! эта   */
//  if(registerbgidriver(EGAVGA_driver)<0) return;        /* !! и эта */
read_data();	/* чтение данных */
fclose(fin);
calc_preQ();
calc_geom();
for(iuo=uo,uo=0;uo<iuo;uo++){
	// cprintf("Расчет участка охлаждения # %d ",uo+1);
	printf("Расчет участка охлаждения # %d ",uo+1);
	ii=calc_T();
	// cprintf("\r");
	printf("\r");
	fprintf(fout," Расчет сечения %d\n\r",uo+1);
	prec *= 100;
	sprintf(bf," Для достижения %f%%-ной точности понадобилось %d итерации.\n\r",prec,ii);
	fprintf(fout,"\n%s\n",bf);
	// cprintf("%s",bf);
	printf("%s",bf);
	}
print_data();
// cprintf(" Нажмите любую клавишу для просмотра графиков или \n\r"
printf(" Нажмите любую клавишу для просмотра графиков или \n\r"
       "               ESC для отмены...\n\r",prec,ii);
fclose(fout);
if(getch()!=27){
	graph();
	clg();
	}
}

void error(int mode, int i,...)
/* выдать ошибку и выйти, если mode */
{
	/* список возможных ошибок */
char * err_arr[] = {
/*1*/	" неожиданный конец файла ",
/*2*/	" неправильный номер топливной пары",
/*3*/	" температура стенки за пределами допустимого интервала для этого топлива",
/*4*/	" неверный номер типа сечения",
/*5*/	" неверный тип первого охладителя",
/*6*/	" неверные параметры входа-выхода первого охладителя",
/*7*/	" неверный тип первого охладителя",
/*8*/	" неверные параметры входа-выхода первого охладителя",
/*9*/	" пересеклись контуры разных охладителей",
/*10*/  " при противотоке контуры пересеклись",
/*11*/	" неверный номер материала огневой стенки",
/*12*/	" перегрев жидкости до %.2f K в сечении %d ",
/*13*/	" недостаточно памяти ",
/*14*/	" задан неверный тип охладителя %d ",
/*15*/	" сечение входа на участке %d лежит внутри участка %d " ,
/*16*/	" сечение выхода на участке %d лежит внутри участка %d ",
/*17*/	" задан нулевой расход охладителя на участке %d ",
/*18*/	" неверно задана петля тракта на участке %d ",
/*19*/	" сечение разворота петли на участке %d лежит внутри участка %d ",
/*20*/	" температура охлажителя корректировалась по предыдущей ",
/*21*/	" здесь допустимы значения 0 и 1 ",
/*22*/	" задано недопустимо мало (%d) итераций: исправлено на 10 ",
/*23*/	" задано недопустимо много (%d) итераций: исправлено на 100 ",
/*24*/  " задано нулевое количество ребер в сечении %d",
/*25*/  " задано недопустимое значение угла наклона гофра %f в сечении %d",
/*26*/	" на участке %d не размещаются ребра (гофры, трубки): уменьшите количество",
/*27*/	" на участке %d не размещаются гофры: уменьшите количество или угол наклона",
/*28*/	" рекомендуемая толщина ребра 1,0 ... 1,5 мм",
/*29*/	" рекомендуемая толщина гофра 0,3 ... 0,5 мм",
/*30*/	" рекомендемый шаг гофров 3,5 ... 7,0 мм",
/*31*/	" рекомендуемый шаг ребер и трубок 2,0(3,5) ... (6,0)6,5 мм",
/*32*/	" рекомендуемая высота тракта 1,2 ... 3,5 мм",
/*33*/  " для сечения %d указан номер %d",
/*34*/  " в сечении %d указан ненулевой угол закрутки (щели??)",
/*35*/	" в сечении %d недопустимое значение Re = %f",
/*36*/	" в сечении %d недопустимое значение Pr = %f",
/*37*/	" в сечении %d недопустимый относительный путь фильтрации %f",
	NULL
	};
char bf[200];
char *ermes[2] = {"Предупреждение","Ошибка"};
va_list ap;
sprintf(bf,"%s: ", ermes[mode]);
va_start(ap, i);
vsprintf(&bf[strlen(bf)],err_arr[i-1], ap);
va_end(ap);
strcat(bf,"!");
fprintf(fout,"%s\n",bf);
// cprintf("\n\r%s\n\r",bf);
printf("\n\r%s\n\r",bf);
if(mode)
	exit(-(i-1));
}

void read_data(void)
{
int ii,iuo,jj, is_del, nsch;
double tk1;
int alloc_memory(int);

char * toplpara[]={		/* названия топливных пар */
/*01*/	"АК + керосин Т-1",
/*02*/	"АК + тонка",
/*03*/	"АК-27 + тонка",
/*04*/	"АК-27 + НДМГ",
/*05*/	"АК-40 + керосин Т-1",
/*06*/	"АК-40 + тонка",
/*07*/	"N2O4 + ММГ",
/*08*/	"N2O4 + НДМГ",
/*09*/	"N204 + аэрозин-50",
/*10*/	"Кислород + спирт 93.5%",
/*11*/	"Кислород + керосин Т-1",
/*12*/	"Кислород + метан",
// /*12*/	"Кислород + углевод.горючее",
/*13*/	"Кислород + керосин",
/*14*/	"Кислород + НДМГ",
/*15*/	"Кислород + водород",
/*16*/	"Фтор + водород",
/*17*/	"H2O2 + НДМГ",
	NULL};

char * tpls[]={			/* названия охладителей */
/*01*/	"АК",
/*02*/	"АК-27",
/*03*/	"керосин Т-1",
/*04*/	"тонка",
/*05*/	"НДМГ",
/*06*/	"N2O4",
/*07*/	"аэрозин",
/*08*/	"вода",
/*09*/	"АК-40",
/*10*/	"ММГ",
/*11*/	"O2ж",
/*12*/	"F2ж",
/*13*/	"H2O2",
/*14*/	"спирт 93.5%",
/*15*/	"метан", /* теперь тут метан */	
// /*15*/	"C10H16",
/*16*/	"керосин",
/*17*/	"H2ж",
	NULL};

char *tsech[]={         	/* названия типов сечений */
/*0*/	"Ребра",
/*1*/	"Гофры",
/*2*/	"Трубки",
/*3*/   "Щель",
/*4*/   "Tрансп",
/*5*/   "МКТТ"
	};

char *maters[]={		/* названия материалов стенок */
/*0*/	"ВТ-00",
/*1*/	"Тi-сплав",
/*2*/	"12X18H10T",
/*3*/	"08X17H5M3",
/*4*/	"2X18H12C4TЮ",
/*5*/	"БрХ-08",
/*6*/	"W-сплав",
/*7*/	"Ni-сплав",
/*8*/	"Nb-сплав"
	};

char *numbers[]={		/* числительные для информации
				   об участках охлаждения */
	"первo",
	"вторo",
	"третье",
	"четверто",
	"пято",
	"шесто",
	"седьмо",
	"восьмо",
	"девято",
	"десято",
	"следующе"
	};

fprintf(fout,"\n--- Исходные данные ---\n\n");

i1=getINT();
i1--;
if(i1<0 || i1>MAXTOPL) error(1,2);
fprintf(fout," Топливо %s\n",toplpara[i1]);

k=getDBL();
fprintf(fout," Показатель процесса %4.2f\n",k);

Aok1=getDBL();
fprintf(fout," Коэффициент избытка окислителя в ядре потока %6.3f\n",Aok1);

n=getINT();
fprintf(fout," Количество сечений разбиения %d\n",n);

is_del = getINT();
if(is_del!=0 && is_del!=1)
	error(1,21);
fprintf(fout," Задаются %s сечений\n\n",((is_del)?"отрезки":"координаты"));

if(!alloc_memory(0))
	error(1,13);

Dkr=1e8;
fprintf(fout,"/-----------------------------------------------\\\n");
fprintf(fout,"| Номер | Координата,мм |   Диаметр,мм |   Aok  |\n");
fprintf(fout,"|-------+---------------+--------------+--------|\n");
if(is_del)
	x[0] = 0;
for(ii=0;ii<n;ii++){
	if(!is_del)             	/* координаты */
		x[ii]=getDBL();
	else if(ii>0)			/* отрезки */
		x[ii]+=getDBL();
	d[ii]=getDBL();
	Aok[ii]=getDBL();
	fprintf(fout,"|    %2d |  %12.4f | %12.4f | %6.3f |\n",
		ii+1,x[ii]*1e3,d[ii]*1e3,Aok[ii]);
	if(d[ii]<Dkr) {Dkr=d[ii];nkr=ii;}
	}

fprintf(fout,"\\-----------------------------------------------/\n\n");
fprintf(fout," Критическое сечение номер %d диаметр %.4f м\n",nkr+1,Dkr);

Rg=Rgc(-1);
fprintf(fout," Газовая постоянная у смесительной головки %.3f Дж/(кг*K)\n",Rg);

tk1=Tog(0);
fprintf(fout," Температура в пристенке у смесительной головки %6.1f K\n",tk1);

Tk=Tog(1);
fprintf(fout," Температура в ядре у смесительной головки %6.1f K\n",Tk);

pk=getDBL();
fprintf(fout," Давление в камере %.4f МПа\n",(pk/1e6));

Ql0=getDBL();
fprintf(fout," Задан лучистый поток в камере %.2f кВт/м2\n",Ql0);
Ql0 *= 1000.;

fprintf(fout,"\n Параметры охлаждающего тракта: \n\n");

uo=getINT();
iuo=uo;
fprintf(fout," Количество участков охлаждения %d\n",uo);

if(!alloc_memory(1))
	error(1,13);

for(uo=0;uo<iuo;uo++){

//   fprintf(fout," Параметры %sго участка:\n",numbers[max(uo,10)]);
  fprintf(fout," Параметры %sго участка:\n",numbers[max(uo,10)]);


  vx[uo]=getINT();  			/* сечение входа */
  tx[uo]=getINT();                      /* тип сечения */
  if(tx[uo]!=0 && tx[uo]!=1 && tx[uo]!=2)
	error(1,5);
  ox[uo]=getINT();                      /* сечение выхода */
  if(ox[uo]==vx[uo])
	error(1,6);
  ptx[uo]=getINT();                     /* петля */

  if(ptx[uo]){				/* если есть петля, */
	  ii=ptx[uo]-ox[uo];            /* надо проверить, что */
	  jj=ptx[uo]-vx[uo];            /* контуры не пересекаются */
	  if(ii*jj < 0)
		error(1,18,uo+1);
	  }



  To1[uo]=getDBL();                             /* температура охладителя */
/* пересчитываем охладители в реальные номера */
  if(tx[uo]==2)
	tx[uo]=8;			/* вода */
  else switch(i1){
	case 0: tx[uo]=(tx[uo]==1)?1:3;break;
	case 1: tx[uo]=(tx[uo]==1)?1:4;break;
	case 2: tx[uo]=(tx[uo]==1)?2:4;break;
	case 3: tx[uo]=(tx[uo]==1)?2:5;break;
	case 4: tx[uo]=(tx[uo]==1)?9:3;break;
	case 5: tx[uo]=(tx[uo]==1)?9:4;break;
	case 6: tx[uo]=(tx[uo]==1)?6:10;break;
	case 7: tx[uo]=(tx[uo]==1)?6:5;break;
	case 8: tx[uo]=(tx[uo]==1)?6:7;break;
	case 9: tx[uo]=(tx[uo]==1)?11:14;break;
	case 10: tx[uo]=(tx[uo]==1)?11:3;break;
	case 11: tx[uo]=(tx[uo]==1)?11:15;break;
	case 12: tx[uo]=(tx[uo]==1)?11:16;break;
	case 13: tx[uo]=(tx[uo]==1)?11:5;break;
	case 14: tx[uo]=(tx[uo]==1)?11:17;break;
	case 15: tx[uo]=(tx[uo]==1)?12:17;break;
	case 16: tx[uo]=(tx[uo]==1)?13:5 ;break;
	default: tx[uo]=-1;break;
	}
  tx[uo]--;
  if(tx[uo]<0) error(1,14,tx[uo]);
  fprintf(fout," Охладитель: %s входит в %d сечение, выходит из %d.\n Tемпература на входе %.1f K\n",
	tpls[tx[uo]],vx[uo],ox[uo],To1[uo]);
  if (ptx[uo])
    fprintf(fout," Тракт охлаждения имеет петлю между %d и %d сечениями\n",
	    vx[uo],ptx[uo]);
  vx[uo]--;
  ox[uo]--;
  if(ptx[uo]) ptx[uo]--;
  for(jj=0;jj<uo;jj++){
	vm = max(vx[uo],ox[uo]);
	om = min(vx[uo],ox[uo]);
	if(ptx[uo]){
		vm=max(vm,ptx[uo]);
		om=min(om,ptx[uo]);
		}
	if(vx[jj]<vm && vx[jj]>om)
		error(1,15,jj+1,ii+1);
	if(ox[jj]<vm && ox[jj]>om)
		error(1,16,jj+1,uo+1);
	if(ptx[jj])
		if(ptx[jj] < vm && ptx[jj]>om)
			error(1,19,jj+1,uo+1);
	}
  } /* закончено считывание всех участков */
  for(ii=0;ii<n;ii++) Tg[ii]=0;

  fprintf(fout," Параметры охлаждающего тракта всех участков охлаждения\n");
  for(vm=1000,om=-vm,jj=0;jj<iuo;jj++){
	vm = min(vm,vx[jj]);
	vm = min(vm,ox[jj]);
	om = max(om,vx[jj]);
	om = max(om,ox[jj]);
	if(ptx[jj]){
		vm = min(vm,ptx[jj]);
		om = max(om,ptx[jj]);
		}
	}
  fprintf(fout," Считываются сечения от %d до %d\n",vm+1,++om);

  fprintf(fout,"%c\n",15);
  fprintf(fout,"/-----+------+-------+-------+-------+-------+------------+------------+-------+-------+-------+-------\\\n");
  fprintf(fout,"|Номер| Тип  |Кол-во | Высота|Толщина|Толщина|   Материал |   Материал | Угол  | Угол  |Толщина|Расход |\n");
  fprintf(fout,"|сеч. | сеч. |ребер/ | канала| ребра |огневой|   огневой  |   наружной |наклона| закр. |внешней|охлади-|\n");
  fprintf(fout,"|     |      |гофров/|       |       | стенки|    стенки  |    стенки  | гофра | ребер | стенки| теля  |\n");
  fprintf(fout,"|     |      |трубок |       |       |       |            | (материала |градус.|градус.|  мм   |       |\n");
  fprintf(fout,"|     |      |       |  мм   |  мм   |   мм  |            |   матрицы) |(alpha)|(beta) |(пор-ь)| кг/с  |\n");
  fprintf(fout,"|-----+------+-------+-------+-------+-------+------------+------------+-------+-------+-------+-------|\n");

    for(ii=vm;ii<om;ii++){
	nsch = getINT();	      /* номер сечения */
	if(nsch != ii+1)
		error(0,33,ii+1,nsch);
        tse[ii]=getINT();             /* тип сечения */
        if(tse[ii]<0 || tse[ii]>5)
                error(1,4);
        np[ii]=getINT();              /* количество ребер */
        if(np[ii]<=0 && (tse[ii]!=3 || tse[ii]!=4) )
                error(1,24,ii);
        Hk[ii]=getDBL()/1000;         /* высота ребра */
        Delr[ii]=getDBL()/1000;       /* толщина ребра */
        Delo[ii]=getDBL()/1000;       /* толщина огневой стенки */
        mato[ii]=getINT();            /* материал огневой стенки */
	if(mato[ii]<0 || mato[ii]>8) error(1,11);
        matn[ii]=getINT();            /* материал неогневой или поры */
	if(matn[ii]<0 || matn[ii]>8) error(1,11);
        Gam[ii]=getDBL();             /* угол гофра или alpha */
	if(tse[ii]==3 && (Gam[ii] <= 10. || Gam[ii] >=80.))
                error(1,25,Gam[ii],ii);
        Bet[ii]=getDBL();             /* угол закрутки или beta */
        if(tse[ii]==3 && Bet[ii] != 0)
                error(1,34,ii);
	Deln[ii]=getDBL()/1000;       /* толщина неогневой стенки или пористость */
        G1[ii]=getDBL();              /* расход */
	if(G1[ii]==0)
		error(1,17,ii+1);
	fprintf(fout,"|%5d|%6s|%7d|%7.3f|%7.3f|%7.3f|%12s|%12s|%7.2G|%7.2G|%7.3f|%7.4f|\n",
		ii+1,
		tsech[tse[ii]],
                np[ii],
		Hk[ii]*1e3,
		Delr[ii]*1e3,
		Delo[ii]*1e3,
		maters[mato[ii]],
		maters[matn[ii]],
		Gam[ii],
		Bet[ii],
		Deln[ii]*1e3,
		G1[ii]);
	if(Gam[ii] >= PI)	/* это - не радианы! */
		Gam[ii] *= GRAD_RAD;
	if(Bet[ii] >= PI)
		Bet[ii] *= GRAD_RAD;
	}

fprintf(fout,"\\-----------------------------------------------------------------------------------------------------/\n");
fprintf(fout,"%c\n",18);
fprintf(fout,"--- Исходные данные считаны---\n\n");
}

void * get_token(int type)
{
static char bf[200];
static char *p=NULL;
static double d;
static int i;

if(p==NULL || *p==0 )
        {
        do fgets(bf,200,fin); while(!feof(fin) && *bf=='*');
	if(*bf && !feof(fin))
		p=bf;
	else error(1,1);
	}
if(type==DBL)
        {
	d=strtod(p,&p);
	while(*p && !isdigit(*p) && *p!='.' && *p!='+' && *p!='-')p++;
	return &d;
	}
else if(type==INT)
        {
	i=(int)strtol(p,&p,10);
	while(*p && !isdigit(*p) && *p!='.' && *p!='+' && *p!='-')p++;
	return &i;
	}
return NULL;
}

static int alloc_memory(int mode)
/* выделение памяти */
{
int nn;

if(mode==0)
 {    /* под сечения камеры */
  nn = n+1;
  if((x=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((d=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((T=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((P=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Tct=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Tctg=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Aok=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((La=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Q=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Ql=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Tg=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Alp=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((ACq=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Dg=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Dg02=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((fS=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((roW=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((dS=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Tg0=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((np=(int*)calloc(nn,sizeof(int)))==NULL) return 0;
  if((Re=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Eta=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((tN=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Ak=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Hk=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Delr=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Delo=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Deln=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Gam=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((Bet=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((dP=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
  if((G1=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
 }
else{	/* под участки охлаждения */
  nn=uo+1;
  if((vx=(int*)calloc(nn,sizeof(int)))==NULL) return 0;
  if((ox=(int*)calloc(nn,sizeof(int)))==NULL) return 0;
  if((tx=(int*)calloc(nn,sizeof(int)))==NULL) return 0;
  if((ptx=(int*)calloc(nn,sizeof(int)))==NULL) return 0;
  if((To1=(double*)calloc(nn,sizeof(double)))==NULL) return 0;
}
return 1;
}
