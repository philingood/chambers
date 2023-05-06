#include <stdio.h>
#include <math.h>
#include "hol3.h"

static double Qtek, k1;

/* во всех программах где только можно массивы объявлены как
   static, чтобы исключить инициализацию при каждом обращении */

double Bcompl(int ii)
/* возвращает значение комплекса В второй формулы Иевлева */
{
/* передается переменная температура ПОТОКА */
static double a[]={8.3605e-3,4.6072e-4,3.4524e-4};
	/* коэффициенты полинома  B(Tct/T) */
return horner(Tct[ii]/T[ii],3,(double*)a);
}

static double Pr058(int ii)
{
/* возвращает значение критерия Прандтля в степени 0.58
 * в зависимости от избытка окислителя или по умолчанию */

double ret = 0.8465; /* по умолчанию для большинства топлив */

static double a4[]={0.73,0.736,0.744,0.752,0.76,0.766,0.773,0.778};
static double a9[]={0.723,0.729,0.734,0.742,0.749,0.753,0.76,0.764,0.768};
static double alphs[]={0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.,1.1,1.2};

const static double pp=0.58;

switch(i1+1){   /* выбрать по типу охладителя */
	case 4:ret=cs_sp(8,alphs+1,(double*)a4,Aok[ii]);
	       ret=pow(ret,pp);
	       break;
	case 9:ret=cs_sp(9,alphs,(double*)a9,Aok[ii]);
	       ret=pow(ret,pp);
	       break;
	default:break;
	}
return ret;
}

void cQl(int ii)
/* возвращает значение лучистого теплового потока в зависимости от
   его величины в камере и номера (координаты) сечения */
{
double dd, q1, d1 = d[ii];
static double a[]={1.477,-1.371,0.442,-0.048};

if(ii==0)		/* у головки */
	q1=Ql0*0.25;
else if(d1==d[0] && ii<nkr)
			/* на протяжении камеры сгорания */
	q1=Ql0;
else if(ii<nkr)		/* на входе в сопло */
	q1=Ql0*(1.-Dkr/d1);
else if(d1<=4*Dkr)	/* на выходе до относительного диаметра 4 */
	{
	dd=d[ii]/Dkr;
	q1=horner(dd,4,a);
	q1*=Ql0;
	}
else q1=0;		/* дальше можно считать, что лучистого нет */
if(q1<0.)		/* - возникает от погрешности полинома */
	q1=0.0;
Ql[ii] = q1;
}

void calc_preQ(void)
/* предварительное вычисление теплового потока -- на первой итерации */
{
double q_la_zero(double);
int ii;
double la1,la2,tau,k2=k/(k-1),qq,lat;
double dd , ppk = pow(pk,0.85),  ddkr= pow(Dkr,0.15);

k1=(k-1)/(k+1);

akr=2*k*Rg*Tk/(k+1);
akr=sqrt(akr);    		/* скорость звука в критике */

for(ii=0;ii<n;ii++){
	/* задаем отрезок поиска решения: до- или сверхзвуковой */
	 if(ii<nkr)		/* дозвуковой */
		{
		la1=0.01;
		la2=1.;
		}
	 else if(ii==nkr)	/* критическое сечение */
		{
		La[ii] =1.;
		lat = 1.;
		}
	 else   {		/* сверхзвуковой */
		la1=1.;
		la2=sqrt((k+1)/(k-1))-0.01;
		}
	 dd=d[ii]/Dkr;
	 Qtek=1/dd;
	 dd=pow(dd,1.82);
	 Qtek*=Qtek;
	 if(ii!=nkr)		/* надо расчитывать */
		{
                 lat=dihotom(la1,la2,0.001,q_la_zero);
		 La[ii] = lat;
		}

         tau=(1-k1*lat*lat);
	 T[ii]=Tk*tau;
	 P[ii]=pk*pow(tau,k2);
	 qq=tau*ppk;
	 qq/=dd;
	 qq/=ddkr;
	 qq/=Pr058(ii);
	 ACq[ii]=qq*1e3;
	 }

/* задание начальной температуры и вычисление первичного потока */

for(ii=0;ii<n;ii++)
	Tct[ii]=1000.;
for(ii=0;ii<n;ii++)
	set_Q(ii);
}


void set_Q(int ii)
/* расчет теплового потока по S и B */
{
double qq, bb;
	qq = SS(ii);
	bb = Bcompl(ii);
        if(qq>0.)       /* можно расчитывать, если комплекс S имеет смысл */
		Q[ii]=ACq[ii]*bb*qq;
        else{           /* иначе приходится интерполировать */
                if(ii==0)               /* у головки  */
			Q[ii]=Q[ii+1];
                else if(ii==n-1)        /* выхлопное сечение */
			Q[ii]=Q[ii-1]*0.5;
		else
			Q[ii]=(Q[ii-1]+Q[ii+1])*0.5;
             }
        cQl(ii);        /* расчитать лучистый поток */
}

double q_la_zero(double la)
/* функция для использования методом дихотомии при поиске решения
 * уравнения Q(la) == 0 */
{
double a,b,q;
a=1.-k1*la*la;
b=1./(k-1.);
q=pow(a,b);
a=(k+1)*0.5;
q*=pow(a,b);
q*=la;
return q-Qtek;
}

/*---------------------------------------------------------*/

void calc_geom(void)
/* вычисляет первичные параметры тракта */
{
double	Bk;	/* ширина гофра на неогневой стенке */
int ii,jj;

double tt1,tt2;

/* ПРАВИЛО: площадь боковой поверхности камеры между
  i и j сечениями записана в dS[min(i,j)] */

for(ii=vm;ii<om;ii++){
        if(tse[ii]!=3)  /* не щель */
                tN[ii] = (d[ii]+2.*Delo[ii])*PI / np[ii];  /* шаг */

        tt1=Delr[ii];           /* толщина ребра */
        switch(tse[ii]){        /* в зависимости от типа тракта */
                        /* трубки */
		case 2: tt1*=2.; /* толщина трубки */
                        /* рeбра */
                case 4:         /* простая транспирация
                                   подобна щели */
                        /* щель */
                case 0:

                        Ak[ii]=(tN[ii]-tt1)*cos(Bet[ii]);       /* зазор */
			if(Ak[ii] <= 0)
				error(1,26,ii);
                        fS[ii]=Ak[ii]*Hk[ii]*np[ii];            /* проходное */
                        tN[ii]=Ak[ii]+Delr[ii];                 /* шаг */
                        Dg[ii]=2*Hk[ii]*Ak[ii]/(Ak[ii]+Hk[ii]); /* гидр. диаметр */
			break;

                        /* гофры */
                case 1:
			Ak[ii] = Hk[ii]*sin(Gam[ii])*0.5;
			if(Ak[ii] <= 0)
				error(1,26,ii);
			Bk = tN[ii] - 2.*Ak[ii];
			if(Bk <= 0)
				error(1,27,ii);
			fS[ii]=np[ii]*tN[ii]*Hk[ii]*(1.-Delr[ii]/tN[ii])*(1.-Delo[ii]/Hk[ii])*0.5;
			tt1=tN[ii]+Delr[ii]-2.*Bk;
			tt1/=2.*(Hk[ii]-Delo[ii]);
                        tt1*=tt1;
                        tt1+=1.;
                        tt1=sqrt(tt1);
			tt2=1.+2.*((Hk[ii]-Delo[ii])/(tN[ii]-Delr[ii]));
			Dg[ii]=2.*Hk[ii]*(1-Delo[ii]/Hk[ii])/tt2/tt1;
			break;

		case 3: fS[ii]=PI*d[ii]*Hk[ii]*(1.+(2.*Delo[ii]+Hk[ii])/d[ii]);
			Dg[ii]=2.*Hk[ii];
			break;

                case 5:         /* МКТТ */
			Ak[ii]=(tN[ii]-tt1)*cos(Bet[ii]);       /* зазор */
			if(Ak[ii] <= 0)
				error(1,26,ii);
			if(ii==0)	/* половинку! */
				fS[ii]=(x[1]-x[0])*Hk[ii]*np[ii]*0.5;
			else
				fS[ii]=(x[ii]-x[ii-1])*Hk[ii]*np[ii];
			tN[ii]=Ak[ii]+Delr[ii];
                        Dg[ii]=2*Hk[ii];
			break;

		default: error(1,4);
		}
	Dg02[ii] = pow(Dg[ii],0.2);
	roW[ii]=G1[ii]/fS[ii];
	if(ii==0)
		roW[0] *= 0.5;

	if(ii<om-1){
	/* площадь боковой поверхности конуса */
		jj = ii + 1;
		tt1=x[ii]-x[jj];
		tt2=d[ii]-d[jj];
		tt2*=0.5;
		tt1 = hypot(tt1,tt2);
		tt1*=(d[ii]+d[jj])*0.5;
		dS[ii]=PI*tt1;
		}
	}
/*	теперь проверяем все сечения на соответствие рекомендациям	*/

/*	толщина ребра или гофра 	*/
for(ii=vm; ii<om;ii++)
	{
	if(tse[ii]==0)		/* ребра */
		{
		if(Delr[ii]<0.001 || Delr[ii]>0.0015)
			{
			error(0,28);
			break;
			}
		}
	else if(tse[ii]==1)	/* гофры */
		{
		if(Delr[ii] < 0.0003 || Delr[ii]>0.0005)
			{
			error(0,29);
			break;
			}
		}
	}
/*      шаг	*/
for(ii=vm; ii<om;ii++)
	{
	if(tse[ii]==0 || tse[ii]==2)		/* ребра и трубки */
		{
		if(tN[ii]<0.0035 || tN[ii]>0.0065)
			{
			error(0,31);
			break;
			}
		}
	else if(tse[ii]==1)	/* гофры */
		{
		if(tN[ii] < 0.0035 || tN[ii]>0.007)
			{
			error(0,30);
			break;
			}
		}
	}
/*	высота тракта	*/
for(ii=vm; ii<om;ii++)
	{
	if(Hk[ii]<0.0012 || Hk[ii]>0.0035)
			{
			error(0,32);
			break;
			}
	}
/*	относительный путь фильтрации 	*/
for(ii=vm; ii<om;ii++)
	if(tse[ii]==5)
		{
		if(tN[ii]/Hk[ii] < 2.8 || tN[ii]/Hk[ii] > 11.3)
			{
			error(1,37,ii,tN[ii]/Hk[ii]);
			}
		}
}
