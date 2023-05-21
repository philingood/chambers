#include <stdio.h>
// #include <process.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
// #include <conio.h>
#include <stdarg.h>
#include <string.h>
#include <windows.h>

// #include <graphics.h>

// #include "TOPLS.C"
// #include "CALC_T.C"
// #include "CALC_Q.C"
// #include "HOL_MATH.C"
// #include "HOL_PRN.C"
#include "hol3.h"
// #include "grafic.c"

/* сначала считываются координата, диаметр и к-т избытка окислителя
   память под остальные переменные выделяется потом */

double k, /* показатель процесса */
	Tk,	  /* температура сгорания */
	pk,	  /* давление в камере */
	Dkr,  /* критика */
	akr,  /* ск. звука в критике */
	*x,	  /* координата */
	*d,	  /* диаметр */
	*T,	  /* температура газа */
	*P,
	*Tct,  /* температура стенки со стороны газа */
	*Tctg, /* температура стенки со стороны жидкости */
	Rg,	   /* газовая постоянная */
	*Aok,  /* избыток окислителя в пристенке */
	Aok1,  /* в ядре */
	*La,   /* коэффициент скорости */
	*Q,	   /* конвективный тепловой поток */
	*Ql,   /* лучевой тепловой поток */
	Ql0,   /* лучистый тепловой поток В КАМЕРЕ */
	*Tg,   /* температура жидкости */
	*G1,   /* расходы 1 и 2 охладителей */
	*To1,  /* температуры 1 и 2 охладителей */
	*Alp,  /* ЭФФЕКТИВНЫЙ к-т теплоотдачи */
	*ACq,  /* часть Qk */
	*Dg,   /* гидр. диаметр */
	*Dg02,
	*fS, /* сечение тракта */
	*roW,
	*dS,
	*Tg0,
	*Re,
	*Eta,
	*tN,
	*Ak,   /* ширина тракта */
	*Hk,   /* высота тракта */
	*Delr, /* толщина ребра или гофра */
	*Delo, /* толщина огневой стенки */
	*Deln, /* толщина неогневой стенки */
	*Gam,  /* угол наклона гофра */
	*Bet,  /* угол закрутки ребер */
	*dP;   /* потери давления */

int n,	 /* количество участков разбиения */
	nkr, /* критическое сечение */
	i1,	 /* номер пары горючего */
	i2,
	maxiter = 10, /* итераций по умолчанию */
	*vx,
	*tx,
	*ox,  /* вход, тип, выход охладителя */
	*ptx, /* петля охладителя */
	*np,  /* количество ребер */
	vm,	  /* минимальный номер участка охлаждения */
	om,	  /* максимальный номер участка охлаждения */
	uo;	  /* текущий участок охлаждения */

FILE *fout, *fin;

double maxT[] = {
	/* максимально допустимые температуры */
	350, /* AK */
	340, /* AK-27 */
	580, /* керосин Т-1 */
	460, /* тонка */
	580, /* НДМГ */
	420, /* N2O4 */
	340, /* аэрозин */
	373, /* вода */
	330, /* АК-40 */
	325, /* ММГ */
	150, /* O2ж */
	95,	 /* F2ж */
	420, /* H2O2 */
	490, /* спирт 93.5% */
	400, /* метан */
	// 360, /* C10H16 */
	460, /* керосин */
	1200 /* H2ж и газ*/
};

extern double prec;

// void main(int argc, char ** argv)
int main(int argc, char **argv)
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	// SetConsoleCP(1251);
	// SetConsoleOutputCP(1251);

	int ii, iuo;
	char bf[100];

	printf(" Программа расчета наружного проточного охлаждения ЖРД\n\r"
		   " ХОЛОД - 3 PC. (c) Затонский А.В. МГТУ Э-1 %s\n\r",
		   __DATE__);
	/*
	printf(" ЭТА ВЕРСИЯ ПРЕДНАЗНАЧЕНА ДЛЯ ОТРАБОТКИ АЛГОРИТМА И ПРОВЕРКИ\n\r"
		   " БАЗЫ ДАННЫХ ТЕПЛОФИЗИЧЕСКИХ СВОЙСТВ КОМПОНЕНТОВ ТОПЛИВА.\n\r"
		   " РЕКОМЕНДУЕТСЯ ПОЛЬЗОВАТЬСЯ С ОСТОРОЖНОСТЬЮ И СООБЩИТЬ\n\r"
		   " О ВСЕХ ЗАМЕЧЕННЫХ НЕДОСТАТКАХ.\n\r");
	*/

	if (argc < 2)
	{
		/*
			printf(" Вызов: HOLOD3 <имя файла данных> <max итераций>\n\r");
			return;
		*/
		printf(" Имя файла с данными: ");
		gets(bf);
		printf(" Максимальное количество итераций: ");
		scanf("%d", &maxiter);
	}
	else
		strcpy(bf, argv[1]);

	if (argc > 2)
	{
		maxiter = atoi(argv[2]);
		if (maxiter < 10)
		{
			error(0, 22, maxiter);
			maxiter = 10;
		}
		if (maxiter > 100)
		{
			error(0, 23, maxiter);
			maxiter = 100;
		}
	}
	if ((fin = fopen(bf, "rt")) == NULL)
	{
		printf(" Не могу открыть файл данных %s\n\r", bf);
		return 0;
	}
	if ((fout = _wfopen(L"holod-win64.rez", L"wt, ccs=UTF-8")) == NULL)
	{
		printf(" Не могу открыть файл результатов: вывод будет\n\r"
			   " роизводиться на экран. Нажмите ENTER для про-\n\r"
			   " должения или ESC для прерывания программы\n\r");
		if (getchar() == 27)
			exit(0);
		fout = stdout;
	}
	/* Внимание! Две следующие строки требуют компоновки с библиотекой
	   GRAPHADV.LIB == GRAPHICS.LIB + CGA.OBJ + EGAVGA.OBJ.
	   Чтобы компоновать с GRAPHICS.LIB надо закомментировать их, а
	   в рабочую директорию подписать CGA.BGI / EGAVGA.BGI        */
	//  if(registerbgidriver(CGA_driver)<0) return;           /* !! эта   */
	//  if(registerbgidriver(EGAVGA_driver)<0) return;        /* !! и эта */
	read_data(); /* чтение данных */
	fclose(fin);
	calc_preQ();
	calc_geom();
	for (iuo = uo, uo = 0; uo < iuo; uo++)
	{
		printf("Расчет участка охлаждения # %d ", uo + 1);
		ii = calc_T();
		printf("\r");
		fprintf(fout, " Расчет сечения %d\n\r", uo + 1);
		prec *= 100;
		sprintf(bf, " Для достижения %f%%-ной точности понадобилось %d итерации.\n\r", prec, ii);
		fwprintf(fout, L"\n%ls\n", bf);
		printf("%s", bf);
	}
	print_data();
	// printf(" Нажмите любую клавишу для просмотра графиков или \n\r"
	// 	   "               ESC для отмены...\n\r",
	// 	   prec, ii);
	fclose(fout);
	// if(getch()!=27){
	// 	graph();
	// 	clg();
	// 	}
	system("pause");
	return 0;
}

void error(int mode, int i, ...)
/* выдать ошибку и выйти, если mode */
{
	/* список возможных ошибок */
	const wchar_t *err_arr[] = {
		/*1*/ L" неожиданный конец файла ",
		/*2*/ L" неправильный номер топливной пары",
		/*3*/ L" температура стенки за пределами допустимого интервала для этого топлива",
		/*4*/ L" неверный номер типа сечения",
		/*5*/ L" неверный тип первого охладителя",
		/*6*/ L" неверные параметры входа-выхода первого охладителя",
		/*7*/ L" неверный тип первого охладителя",
		/*8*/ L" неверные параметры входа-выхода первого охладителя",
		/*9*/ L" пересеклись контуры разных охладителей",
		/*10*/ L" при противотоке контуры пересеклись",
		/*11*/ L" неверный номер материала огневой стенки",
		/*12*/ L" перегрев жидкости до %.2f K в сечении %d ",
		/*13*/ L" недостаточно памяти ",
		/*14*/ L" задан неверный тип охладителя %d ",
		/*15*/ L" сечение входа на участке %d лежит внутри участка %d ",
		/*16*/ L" сечение выхода на участке %d лежит внутри участка %d ",
		/*17*/ L" задан нулевой расход охладителя на участке %d ",
		/*18*/ L" неверно задана петля тракта на участке %d ",
		/*19*/ L" сечение разворота петли на участке %d лежит внутри участка %d ",
		/*20*/ L" температура охлажителя корректировалась по предыдущей ",
		/*21*/ L" здесь допустимы значения 0 и 1 ",
		/*22*/ L" задано недопустимо мало (%d) итераций: исправлено на 10 ",
		/*23*/ L" задано недопустимо много (%d) итераций: исправлено на 100 ",
		/*24*/ L" задано нулевое количество ребер в сечении %d",
		/*25*/ L" задано недопустимое значение угла наклона гофра %f в сечении %d",
		/*26*/ L" на участке %d не размещаются ребра (гофры, трубки): уменьшите количество",
		/*27*/ L" на участке %d не размещаются гофры: уменьшите количество или угол наклона",
		/*28*/ L" рекомендуемая толщина ребра 1,0 ... 1,5 мм",
		/*29*/ L" рекомендуемая толщина гофра 0,3 ... 0,5 мм",
		/*30*/ L" рекомендемый шаг гофров 3,5 ... 7,0 мм",
		/*31*/ L" рекомендуемый шаг ребер и трубок 2,0(3,5) ... (6,0)6,5 мм",
		/*32*/ L" рекомендуемая высота тракта 1,2 ... 3,5 мм",
		/*33*/ L" для сечения %d указан номер %d",
		/*34*/ L" в сечении %d указан ненулевой угол закрутки (щели?)",
		/*35*/ L" в сечении %d недопустимое значение Re = %f",
		/*36*/ L" в сечении %d недопустимое значение Pr = %f",
		/*37*/ L" в сечении %d недопустимый относительный путь фильтрации %f",
		NULL};
	const char *ermes[2] = {"Предупреждение", "Ошибка"};
	va_list ap;
	wchar_t bf[200];
	swprintf(bf, L"%ls: ", ermes[mode]);
	va_start(ap, i);
	vswprintf(&bf[200], err_arr[i - 1], ap);
	va_end(ap);
	wcscat(bf, L"!");
	// strcat(bf, "!");
	fwprintf(fout, L"%ls\n", bf);
	printf("\n\r%s\n\r", bf);
	if (mode)
		exit(-(i - 1));
}

void read_data(void)
{
	int ii, iuo, jj, is_del, nsch;
	double tk1;
	int alloc_memory(int);

	const wchar_t *toplpara[] = {/* названия топливных пар */
						/*01*/ L"АК + керосин Т-1",
						/*02*/ L"АК + тонка",
						/*03*/ L"АК-27 + тонка",
						/*04*/ L"АК-27 + НДМГ",
						/*05*/ L"АК-40 + керосин Т-1",
						/*06*/ L"АК-40 + тонка",
						/*07*/ L"N2O4 + ММГ",
						/*08*/ L"N2O4 + НДМГ",
						/*09*/ L"N204 + аэрозин-50",
						/*10*/ L"Кислород + спирт 93.5%",
						/*11*/ L"Кислород + керосин Т-1",
						/*12*/ L"Кислород + метан",
						// /*12*/	"Кислород + углевод.горючее",
						/*13*/ L"Кислород + керосин",
						/*14*/ L"Кислород + НДМГ",
						/*15*/ L"Кислород + водород",
						/*16*/ L"Фтор + водород",
						/*17*/ L"H2O2 + НДМГ",
						NULL};

	const wchar_t *tpls[] = {/* названия охладителей */
					/*01*/ L"АК",
					/*02*/ L"АК-27",
					/*03*/ L"керосин Т-1",
					/*04*/ L"тонка",
					/*05*/ L"НДМГ",
					/*06*/ L"N2O4",
					/*07*/ L"аэрозин",
					/*08*/ L"вода",
					/*09*/ L"АК-40",
					/*10*/ L"ММГ",
					/*11*/ L"O2ж",
					/*12*/ L"F2ж",
					/*13*/ L"H2O2",
					/*14*/ L"спирт 93.5%",
					/*15*/ L"метан", /* теперь тут метан */
					// /*15*/	"C10H16",
					/*16*/ L"керосин",
					/*17*/ L"H2ж",
					NULL};

	const wchar_t *tsech[] = {/* названия типов сечений */
					 /*0*/ L"Ребра",
					 /*1*/ L"Гофры",
					 /*2*/ L"Трубки",
					 /*3*/ L"Щель",
					 /*4*/ L"Tрансп",
					 /*5*/ L"МКТТ"};

	const wchar_t *maters[] = {/* названия материалов стенок */
					  /*0*/ L"ВТ-00",
					  /*1*/ L"Тi-сплав",
					  /*2*/ L"12X18H10T",
					  /*3*/ L"08X17H5M3",
					  /*4*/ L"2X18H12C4TЮ",
					  /*5*/ L"БрХ-08",
					  /*6*/ L"W-сплав",
					  /*7*/ L"Ni-сплав",
					  /*8*/ L"Nb-сплав"};

	const wchar_t *numbers[] = {/* числительные для информации
				  об участках охлаждения */
					   L"первo",
					   L"вторo",
					   L"третье",
					   L"четверто",
					   L"пято",
					   L"шесто",
					   L"седьмо",
					   L"восьмо",
					   L"девято",
					   L"десято",
					   L"следующе"};

	fwprintf(fout, L"\n--- Исходные данные ---\n\n");

	i1 = getINT();
	i1--;
	if (i1 < 0 || i1 > MAXTOPL)
		error(1, 2);
	fwprintf(fout, L" Топливо %ls\n", toplpara[i1]);

	k = getDBL();
	fwprintf(fout, L" Показатель процесса %4.2f\n", k);

	Aok1 = getDBL();
	fwprintf(fout, L" Коэффициент избытка окислителя в ядре потока %6.3f\n", Aok1);

	n = getINT();
	fwprintf(fout, L" Количество сечений разбиения %d\n", n);

	is_del = getINT();
	if (is_del != 0 && is_del != 1)
		error(1, 21);
	fwprintf(fout, L" Задаются %ls сечений\n\n", ((is_del) ? "отрезки" : "координаты"));

	if (!alloc_memory(0))
		error(1, 13);

	Dkr = 1e8;
	fwprintf(fout, L"/-----------------------------------------------\\\n");
	fwprintf(fout, L"| Номер | Координата,мм |   Диаметр,мм |   Aok  |\n");
	fwprintf(fout, L"|-------+---------------+--------------+--------|\n");
	if (is_del)
		x[0] = 0;
	for (ii = 0; ii < n; ii++)
	{
		if (!is_del) /* координаты */
			x[ii] = getDBL();
		else if (ii > 0) /* отрезки */
			x[ii] += getDBL();
		d[ii] = getDBL();
		Aok[ii] = getDBL();
		fwprintf(fout, L"|    %2d |  %12.4f | %12.4f | %6.3f |\n",
				ii + 1, x[ii] * 1e3, d[ii] * 1e3, Aok[ii]);
		if (d[ii] < Dkr)
		{
			Dkr = d[ii];
			nkr = ii;
		}
	}

	fwprintf(fout, L"\\-----------------------------------------------/\n\n");
	fwprintf(fout, L" Критическое сечение номер %d диаметр %.4f м\n", nkr + 1, Dkr);

	Rg = Rgc(-1);
	fwprintf(fout, L" Газовая постоянная у смесительной головки %.3f Дж/(кг*K)\n", Rg);

	tk1 = Tog(0);
	fwprintf(fout, L" Температура в пристенке у смесительной головки %6.1f K\n", tk1);

	Tk = Tog(1);
	fwprintf(fout, L" Температура в ядре у смесительной головки %6.1f K\n", Tk);

	pk = getDBL();
	fwprintf(fout, L" Давление в камере %.4f МПа\n", (pk / 1e6));

	Ql0 = getDBL();
	fwprintf(fout, L" Задан лучистый поток в камере %.2f кВт/м2\n", Ql0);
	Ql0 *= 1000.;

	fwprintf(fout, L"\n Параметры охлаждающего тракта: \n\n");

	uo = getINT();
	iuo = uo;
	fwprintf(fout, L" Количество участков охлаждения %d\n", uo);

	if (!alloc_memory(1))
		error(1, 13);

	for (uo = 0; uo < iuo; uo++)
	{

		fwprintf(fout, L" Параметры %lsго участка:\n", numbers[max(uo, 10)]);

		vx[uo] = getINT(); /* сечение входа */
		tx[uo] = getINT(); /* тип сечения */
		if (tx[uo] != 0 && tx[uo] != 1 && tx[uo] != 2)
			error(1, 5);
		ox[uo] = getINT(); /* сечение выхода */
		if (ox[uo] == vx[uo])
			error(1, 6);
		ptx[uo] = getINT(); /* петля */

		if (ptx[uo])
		{						   /* если есть петля, */
			ii = ptx[uo] - ox[uo]; /* надо проверить, что */
			jj = ptx[uo] - vx[uo]; /* контуры не пересекаются */
			if (ii * jj < 0)
				error(1, 18, uo + 1);
		}

		To1[uo] = getDBL(); /* температура охладителя */
							/* пересчитываем охладители в реальные номера */
		if (tx[uo] == 2)
			tx[uo] = 8; /* вода */
		else
			switch (i1)
			{
			case 0:
				tx[uo] = (tx[uo] == 1) ? 1 : 3;
				break;
			case 1:
				tx[uo] = (tx[uo] == 1) ? 1 : 4;
				break;
			case 2:
				tx[uo] = (tx[uo] == 1) ? 2 : 4;
				break;
			case 3:
				tx[uo] = (tx[uo] == 1) ? 2 : 5;
				break;
			case 4:
				tx[uo] = (tx[uo] == 1) ? 9 : 3;
				break;
			case 5:
				tx[uo] = (tx[uo] == 1) ? 9 : 4;
				break;
			case 6:
				tx[uo] = (tx[uo] == 1) ? 6 : 10;
				break;
			case 7:
				tx[uo] = (tx[uo] == 1) ? 6 : 5;
				break;
			case 8:
				tx[uo] = (tx[uo] == 1) ? 6 : 7;
				break;
			case 9:
				tx[uo] = (tx[uo] == 1) ? 11 : 14;
				break;
			case 10:
				tx[uo] = (tx[uo] == 1) ? 11 : 3;
				break;
			case 11:
				tx[uo] = (tx[uo] == 1) ? 11 : 15;
				break;
			case 12:
				tx[uo] = (tx[uo] == 1) ? 11 : 16;
				break;
			case 13:
				tx[uo] = (tx[uo] == 1) ? 11 : 5;
				break;
			case 14:
				tx[uo] = (tx[uo] == 1) ? 11 : 17;
				break;
			case 15:
				tx[uo] = (tx[uo] == 1) ? 12 : 17;
				break;
			case 16:
				tx[uo] = (tx[uo] == 1) ? 13 : 5;
				break;
			default:
				tx[uo] = -1;
				break;
			}
		tx[uo]--;
		if (tx[uo] < 0)
			error(1, 14, tx[uo]);
		fwprintf(fout, L" Охладитель: %ls входит в %d сечение, выходит из %d.\n Tемпература на входе %.1f K\n",
				tpls[tx[uo]], vx[uo], ox[uo], To1[uo]);
		if (ptx[uo])
			fwprintf(fout, L" Тракт охлаждения имеет петлю между %d и %d сечениями\n",
					vx[uo], ptx[uo]);
		vx[uo]--;
		ox[uo]--;
		if (ptx[uo])
			ptx[uo]--;
		for (jj = 0; jj < uo; jj++)
		{
			vm = max(vx[uo], ox[uo]);
			om = min(vx[uo], ox[uo]);
			if (ptx[uo])
			{
				vm = max(vm, ptx[uo]);
				om = min(om, ptx[uo]);
			}
			if (vx[jj] < vm && vx[jj] > om)
				error(1, 15, jj + 1, ii + 1);
			if (ox[jj] < vm && ox[jj] > om)
				error(1, 16, jj + 1, uo + 1);
			if (ptx[jj])
				if (ptx[jj] < vm && ptx[jj] > om)
					error(1, 19, jj + 1, uo + 1);
		}
	} /* закончено считывание всех участков */
	for (ii = 0; ii < n; ii++)
		Tg[ii] = 0;

	fwprintf(fout, L" Параметры охлаждающего тракта всех участков охлаждения\n");
	for (vm = 1000, om = -vm, jj = 0; jj < iuo; jj++)
	{
		vm = min(vm, vx[jj]);
		vm = min(vm, ox[jj]);
		om = max(om, vx[jj]);
		om = max(om, ox[jj]);
		if (ptx[jj])
		{
			vm = min(vm, ptx[jj]);
			om = max(om, ptx[jj]);
		}
	}
	fwprintf(fout, L" Считываются сечения от %d до %d\n", vm + 1, ++om);

	fwprintf(fout, L"%c\n", 15);
	fwprintf(fout, L"/-----+------+-------+-------+-------+-------+------------+------------+-------+-------+-------+-------\\\n");
	fwprintf(fout, L"|Номер| Тип  |Кол-во | Высота|Толщина|Толщина|   Материал |   Материал | Угол  | Угол  |Толщина|Расход |\n");
	fwprintf(fout, L"|сеч. | сеч. |ребер/ | канала| ребра |огневой|   огневой  |   наружной |наклона| закр. |внешней|охлади-|\n");
	fwprintf(fout, L"|     |      |гофров/|       |       | стенки|    стенки  |    стенки  | гофра | ребер | стенки| теля  |\n");
	fwprintf(fout, L"|     |      |трубок |       |       |       |            | (материала |градус.|градус.|  мм   |       |\n");
	fwprintf(fout, L"|     |      |       |  мм   |  мм   |   мм  |            |   матрицы) |(alpha)|(beta) |(пор-ь)| кг/с  |\n");
	fwprintf(fout, L"|-----+------+-------+-------+-------+-------+------------+------------+-------+-------+-------+-------|\n");

	for (ii = vm; ii < om; ii++)
	{
		nsch = getINT(); /* номер сечения */
		if (nsch != ii + 1)
			error(0, 33, ii + 1, nsch);
		tse[ii] = getINT(); /* тип сечения */
		if (tse[ii] < 0 || tse[ii] > 5)
			error(1, 4);
		np[ii] = getINT(); /* количество ребер */
		if (np[ii] <= 0 && (tse[ii] != 3 || tse[ii] != 4))
			error(1, 24, ii);
		Hk[ii] = getDBL() / 1000;	/* высота ребра */
		Delr[ii] = getDBL() / 1000; /* толщина ребра */
		Delo[ii] = getDBL() / 1000; /* толщина огневой стенки */
		mato[ii] = getINT();		/* материал огневой стенки */
		if (mato[ii] < 0 || mato[ii] > 8)
			error(1, 11);
		matn[ii] = getINT(); /* материал неогневой или поры */
		if (matn[ii] < 0 || matn[ii] > 8)
			error(1, 11);
		Gam[ii] = getDBL(); /* угол гофра или alpha */
		if (tse[ii] == 3 && (Gam[ii] <= 10. || Gam[ii] >= 80.))
			error(1, 25, Gam[ii], ii);
		Bet[ii] = getDBL(); /* угол закрутки или beta */
		if (tse[ii] == 3 && Bet[ii] != 0)
			error(1, 34, ii);
		Deln[ii] = getDBL() / 1000; /* толщина неогневой стенки или пористость */
		G1[ii] = getDBL();			/* расход */
		if (G1[ii] == 0)
			error(1, 17, ii + 1);
		fwprintf(fout, L"|%5d|%6s|%7d|%7.3f|%7.3f|%7.3f|%12s|%12s|%7.2G|%7.2G|%7.3f|%7.4f|\n",
				ii + 1,
				tsech[tse[ii]],
				np[ii],
				Hk[ii] * 1e3,
				Delr[ii] * 1e3,
				Delo[ii] * 1e3,
				maters[mato[ii]],
				maters[matn[ii]],
				Gam[ii],
				Bet[ii],
				Deln[ii] * 1e3,
				G1[ii]);
		if (Gam[ii] >= PI) /* это - не радианы! */
			Gam[ii] *= GRAD_RAD;
		if (Bet[ii] >= PI)
			Bet[ii] *= GRAD_RAD;
	}

	fwprintf(fout, L"\\-----------------------------------------------------------------------------------------------------/\n");
	fwprintf(fout, L"%c\n", 18);
	fwprintf(fout, L"--- Исходные данные считаны---\n\n");
}

void *get_token(int type)
{
	static char bf[200];
	static char *p = NULL;
	static double d;
	static int i;

	if (p == NULL || *p == 0)
	{
		do
			fgets(bf, 200, fin);
		while (!feof(fin) && *bf == '*');
		if (*bf && !feof(fin))
			p = bf;
		else
			error(1, 1);
	}
	if (type == DBL)
	{
		d = strtod(p, &p);
		while (*p && !isdigit(*p) && *p != '.' && *p != '+' && *p != '-')
			p++;
		return &d;
	}
	else if (type == INT)
	{
		i = (int)strtol(p, &p, 10);
		while (*p && !isdigit(*p) && *p != '.' && *p != '+' && *p != '-')
			p++;
		return &i;
	}
	return NULL;
}

// static int alloc_memory(int mode)
int alloc_memory(int mode)
/* выделение памяти */
{
	int nn;

	if (mode == 0)
	{ /* под сечения камеры */
		nn = n + 1;
		if ((x = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((d = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((T = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((P = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Tct = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Tctg = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Aok = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((La = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Q = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Ql = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Tg = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Alp = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((ACq = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Dg = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Dg02 = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((fS = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((roW = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((dS = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Tg0 = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((np = (int *)calloc(nn, sizeof(int))) == NULL)
			return 0;
		if ((Re = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Eta = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((tN = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Ak = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Hk = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Delr = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Delo = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Deln = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Gam = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((Bet = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((dP = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
		if ((G1 = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
	}
	else
	{ /* под участки охлаждения */
		nn = uo + 1;
		if ((vx = (int *)calloc(nn, sizeof(int))) == NULL)
			return 0;
		if ((ox = (int *)calloc(nn, sizeof(int))) == NULL)
			return 0;
		if ((tx = (int *)calloc(nn, sizeof(int))) == NULL)
			return 0;
		if ((ptx = (int *)calloc(nn, sizeof(int))) == NULL)
			return 0;
		if ((To1 = (double *)calloc(nn, sizeof(double))) == NULL)
			return 0;
	}
	return 1;
}
