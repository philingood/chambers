#include <stdlib.h>
#include <math.h>
#include <conio.h>

#include "hol3.h"

int      tse[MAXNUM],    /* тип охладителя */
        mato[MAXNUM],    /* материал огневой стенки */
        matn[MAXNUM];    /* материал неогневнй стенки */

double prec = 1;
			/* запоминание предыдущих значений температуры
			   жидкости для коррекции в процессе приближения */

static int calc_T1(int tvx, int tox)
/* расчет по сечениям от tvx до tox */
{
double dT,Tr,Cp1,Cp2,Q0,Qsr;

int ii,dk,jj,it,it1,Iter;
double KK(double,int);
void recalc_Alp(int,double,int);
double tt1,tt2, lm;
double sTr, ss;

if(tvx>tox) dk=-1; else dk=1;
			/* куда двигаться от входа к выходу */

Iter=1;
/* для определенности заполняем массив температур жидкости */
for(ii=tvx;ii!=tox+dk;ii+=dk)
	{
	Tg[ii] = To1[uo];
	Eta[ii] = 1.;
	}
for(it=0;it<maxiter && Iter;it++)
  {        /* цикл по итерациям */
  ii = tvx;
  do		/* вычисление температур подогрева во всех сечениях */
		/* начиная со следующего от сечения входа до сечения выхода */
		/* температура жидкости в сечении входа остается неизменной */
    {
    jj = ii;
    ii += dk;
    cprintf("И%2d T%2d\b\b\b\b\b\b\b",it+1,ii+1);
         /* расчет подогрева */
    Qsr=(Q[ii]+Ql[ii]+Q[jj]+Ql[jj])*0.5;
        /* средний тепловой поток */
    ss = dS[min(ii,jj)];
    dT=Qsr*ss;
    dT/=G1[jj];          /* почти подогрев */
    if(tse[ii] != 5)
	Tr=Tg[jj];      /* от этой температуры греется */
    else
	Tr=To1[uo];     /* при МКТТ - нет предв. подогрева! */
    for(it1=0;it1<1000;it1++)
	{
	Cp1=Cp(Tr,tx[uo]);
        sTr=Tr;
	tt1=dT/Cp1;
	if(tse[ii] != 5)
		Tr=tt1+Tg[jj];
	else
		Tr=tt1+To1[uo];

        Cp2=Cp(Tr,tx[uo]);
	if(fabs(Cp1-Cp2)<2.) break;

	}

    if(it1==1000)
        Tr=(Tr+sTr)*0.5;
    if(Tr>maxT[tx[uo]])
	error(0,12,Tr,ii+1);
    Tg[ii]=Tr;          /* температура в следующем сечении
			   или - для МКТТ - в этом сечении.
			   Соответственно, в сечении конца
			   участка охлаждения (0) т-ра
			   не заносится и равна 0 */

    } while (ii!=tox);	/* пока не дошли до конца участка */

/* вычисляем все alpha, Tctg, Tct */
   ii = tvx - dk;
   do			/* вычисление температур стенки и к-та теплоотдачи */
			/* от жидкости в стенку во всех сечениях */
     {
      ii += dk;
      cprintf("И%2d A%2d\b\b\b\b\b\b\b",it+1,ii+1);
      Qsr = Q[ii]+Ql[ii];
      recalc_Alp(ii,Qsr,dk);
     } while (ii!=tox);

   Iter=0;
   ii = tvx - dk;
   do        /* вычисляем все тепловые потоки во всех сечениях */
     {
	ii += dk;
	cprintf("И%2d Q%2d\b\b\b\b\b\b\b",it+1,ii+1);
	Q0=Q[ii];
	if(Q0==0)
		 Q0 = 1e3;       /* для исключения деления на 0 */
	set_Q(ii);

     } while(ii!=tox);
/* критерий окончания: тепловой поток, снимаемый жидкостью, должен
   менее чем на 1% отличаться от теплового потока в стенку!	*/

   ii = tvx - dk;
   do
     {
	ii += dk;
	Q0 = Q[ii]+Ql[ii];	/* в стенку */
	Qsr = Alp[ii]*(Tctg[ii]-Tg[ii])*1000.;
	prec = fabs(Q0-Qsr)/Q0;
	if(prec>0.001)
		Iter=1;
     } while(ii!=tox);
  }              /* конец цикла по итерациям */
ii=tvx;
if(tse[ii] < 4)
  do			/* вычисляем числа Рейнольдса и падение давления */
			/* во всех сечениях от следующего после входа */
			/* до выхода. В случае петли надо взять предыдущее*/
			/* падение */
  {
	jj = ii;
	ii += dk;
        Re[ii]=roW[ii];
        Re[ii]*=Dg[ii];
        Re[ii]*=pow(KK(Tg[ii],tx[uo]),2.5);
        tt2=Cp(Tg[ii],tx[uo]);
        Re[ii]/=tt2;
        tt1=Lam(Tg[ii],tx[uo]);
        Re[ii]/=pow(tt1,1.5);
        tt2 = roW[ii] / Ro(Tg[ii],tx[uo]);      /* скорость течения */
	tt1 = fabs(x[jj] - x[ii]);              /* длина участка */
        if(Re[ii] < 3e3)                        /* к-т потерь */
                lm = 64. / Re[ii];
        else if(Re[ii] < 1e5)
                lm = 0.3164/pow(Re[ii],0.25);
        else
                lm = 0.0032+0.221/pow(Re[ii],0.237);
	dP[ii] = lm * tt1 / Dg[ii] * roW[ii]*tt2 *0.5;
  } while(ii!=tox);
return it;
}

int calc_T(void)
/* драйвер уточнения температур и теплового потока */
{
int ret;
int i,j;

if(ptx[uo]==0){
	Tg[vx[uo]]=To1[uo];
	ret = calc_T1(vx[uo],ox[uo]);
	}
else{
	j = i = vx[uo];
	/*if (ptx[uo]-i > 0) j++; else j--;*/
	Tg[ptx[uo]]=To1[uo];
	ret = calc_T1(ptx[uo],j);
	/* вычисляем температуру жидкости на входе в основную часть */
	Tg[i] = (Tg[j]*G1[j]+To1[uo]*G1[i])/(G1[i]+G1[j]);
	ret += calc_T1(i,ox[uo]);
	/* корректируем Re и падение давления */
	Re[i] = Re[j];
	dP[i] = dP[j];
	}
return ret;
}

double KK(double T,int i)
{
double a1,a2,a3,a4;

a1=Cp(T,i);
a2=Mu(T,i);
a3=Lam(T,i);
a4=a1/a2;
a1=pow(a4,0.4);
a2=pow(a3,0.6);
a4=a1*a2;
return a4;
}

void recalc_Alp(int ii,double Qsr, int dk)
{
double tt3,He,Dele,Hp,tt1,aa1,aa2,aa3,aa4,aa0,mHp,mHe,t4,t3,t5,tt2,dze;
double fpr, Pr, St;
int iter, jj;

if(tse[ii] != 5)
  {
  tt3=KK(Tg[ii],tx[uo]);
  Alp[ii]=tt3*pow(roW[ii],0.8);
  Alp[ii]/=Dg02[ii];
  Alp[ii]*=0.023;
  switch(tse[ii])
       {        /* пересчитываем в реальное значение */
       case 0:
               He=Ak[ii]*0.5;
               Dele=2*Deln[ii];
               Hp=Hk[ii];
               break;
       case 1:
               He=(tN[ii]-Ak[ii]+2*Hk[ii]*sin(Gam[ii]))*0.5;
               Dele=Deln[ii];
               Hp=Hk[ii]/cos(Bet[ii]);
               break;
       case 2:
               He=Dele=0;
               Hp=Hk[ii]+Ak[ii]*0.5;
               break;
       case 3:
               He=Dele=Hp=0;
               break;

       default: break;
       }
  }
else    {       /* для межканальной транспирации */
	if(ii==0)
		aa0 = (x[1]-x[0])*0.5;
	else
		{
		jj = ii+dk;
		aa0 = fabs(x[ii]-x[jj]);                 /* отрезок сечения */
		}
	aa2 = fabs(d[ii]-d[jj]);		 /* разница диаметров */
	aa1 = atan2(aa2,aa0);                    /* угол наклона */
	aa1 = cos(aa1);                          /* его косинус */
	fpr = aa0 * Hk[ii] * np[ii] / aa1;	 /* проходное сечение */
	t3 = (Tg[ii]+To1[uo])*0.5;		/* т-ра жидкости */
	t4 = Mu(t3,tx[uo]);			/* вязкость */
	aa4 = 2.0*G1[ii]*Hk[ii]/fpr/t4;      	/* Re */
	if(ii==0)	/* если взяли полсечения, через них,
			   очевидно, идет полрасхода! */
		aa4 *= 0.5;
	if(aa4 <20. || aa4 > 8000.)
		error(1,35,ii,aa4);
	Re[ii] = aa4;
	t5 = Cp(t3,tx[uo]);			/* теплоемкость */
	aa3 = Lam(t3,tx[uo]);
	Pr = t4 * t5 / aa3 ;
	if(Pr < 0.7 || Pr > 7.)
		error(1,36,ii,Pr);
	St = 0.569*pow(Re[ii],-0.2)*pow(Pr,-0.7);
		/*  эпсилон L */
	if(matn[ii] != 2)
		St *= Ela(t3,matn[ii])/Ela(t3,2); /* эпсилон лямбда */
	Alp[ii] = St * roW[ii] * t5;
        }

tt1=Tg[ii]+Tct[ii];
tt1/=2;
/* в эти формулы alpha надо подставлять в кДж/(м2*К), а
  толщины ребра, эквивалентного ребра и стенок в миллиметрах */
if(tse[ii]<4)
	{
	aa1=Delr[ii] * 1000.;
	aa2=Dele * 1000.;
	aa3=Hp * 1000.;
	aa4=He * 1000.;
	aa0=Alp[ii]/1000.;
	mHp=2.*aa0/Ela(tt1,mato[ii])/aa1;
	t3=sqrt(mHp);
	mHp=t3*aa3;
	mHe=2.*aa0/Ela(tt1,matn[ii])/aa2;
	t4=sqrt(mHe);
	mHe=t4*aa4;
	tt1=tanh(mHe);
	tt2=tanh(mHp);
	t5=t3/t4;
	}
switch(tse[ii])
       {
       case 1: t5*=0.5;
       case 0: dze=((1.+t5*tt2/tt1)/(1.+t5*tt1*tt2));break;
       default: dze=1.;
       }
if(dze < 1.) dze = 1.;
if(tse[ii]<3)        /* не щелевой тракт */
       {
       aa0=2.*Hp*tt2/mHp*dze;
       aa0-=Delr[ii];
       aa0/=tN[ii];
       aa0/=cos(Bet[ii]);
       aa0+=1.;
       }
else if(tse[ii]==4)	/* простая транспирация */
	{
	/* параметр Deln[ii] для транспирации - ПОРИСТОСТЬ */
	aa0 = Ela(Tg[ii],matn[ii])*(1-Deln[ii]*3./2.)/Lam(Tg[ii],tx[uo]);
	}
else aa0=1.;
Eta[ii]=max(1.,aa0);
Alp[ii]*=Eta[ii];        /* реальный к-т */

/* перерасчет теплового потока в этом сечении */
/* производится начиная с жидкости до стенки */

Tctg[ii]=Tg[ii]+Qsr/Alp[ii]/1000.;
/* стенки со стороны жидкости. Alp == кВт/м2 К! */

tt2=Tct[ii];
for(iter=0;iter<1000;iter++)
  {
	tt1=tt2;				/* предыдущая температура */
	tt2=Ela(((tt2+Tctg[ii])*0.5),mato[uo]);	/* теплопроводность стенки */
	tt2=Tctg[ii]+Qsr*Delo[ii]/tt2;		/* новая температура */
	tt3=fabs(tt1-tt2);			/* разница */
	if(tt3 < .1)
		break;
  }
Tct[ii]=tt2;
}
