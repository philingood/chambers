#include <graphics.h>
#include <conio.h>

#include "hol3.h"

static int maxx,maxy;

void pascal graphic(int n,double x[],double y[],double xmax,double xmin,double ymax,double ymin,int cl)
{ int x1,y1;
  double x0,y0;
  static int x2=0,y2=0;
  register i;

  x0=xmax-xmin;
  y0=ymax-ymin;
  if(cl) setcolor(cl);
  for(i=0;i<n;i++)
   {
    x1=(int)((x[i]-xmin)/x0*maxx);
    y1=maxy-(int)((y[i]-ymin)/y0*maxy);
    if(i==0)
        x2=x1,y2=y1;
    line(x1,y1,x2,y2);
    x2=x1;y2=y1;
   }
}

static double ymin,ymax;

static void maxmin(double *y)
{
int ii;
ymax=-1e40;     ymin=-ymax;
for(ii=0;ii<n;ii++)
        {
	if(ymin>y[ii]) ymin=y[ii];
	if(ymax<y[ii]) ymax=y[ii];
	}
}

void graph(void)
{
int gm=0,gd=DETECT;
int t;
int cl=15;
double Tmax,Tmin;

if(gm==0)
 {
 initgraph(&gd,&gm,"");
 if(graphresult()!=grOk) return;

 maxx=getmaxx();
 maxy=getmaxy();
 t=maxx-80;
 }
else cleardevice();

maxmin(d);
ymax*=2;
setlinestyle(SOLID_LINE,0xffff,3);
graphic(n,x,d,x[n-1],x[0],ymax,ymin,cl++);
setlinestyle(SOLID_LINE,0xffff,1);
outtextxy(t,40,"Профиль");

cl=9;           /* цвет вывода следующего графика */
maxmin(La);
graphic(n,x,La,x[n-1],x[0],ymax,ymin,cl++);
outtextxy(t,50,"Скорость");

maxmin(T);
Tmax=ymax;
Tmin=ymin;
maxmin(Tg);
if(Tmin>ymin) Tmin=ymin;
graphic(n,x,T,x[n-1],x[0],Tmax,Tmin,cl++);
outtextxy(t-10,60,"Температура");

maxmin(P);
ymax*=1.1;
graphic(n,x,P,x[n-1],x[0],ymax,ymin,cl++);
outtextxy(t,70,"Давление");

maxmin(Q);
setlinestyle(SOLID_LINE,0xffff,3);
graphic(n,x,Q,x[n-1],x[0],ymax,ymin,cl);
setlinestyle(SOLID_LINE,0xffff,1);
outtextxy(t,80,"Тепло");

maxmin(Tct);
graphic(n,x,Tct,x[n-1],x[0],Tmax,Tmin,13);
outtextxy(t,90,"Стенка");

maxmin(Tg);
graphic(n,x,Tg,x[n-1],x[0],Tmax,Tmin,15);
outtextxy(t-10,100,"Охладитель");
}

void clg(void)
{
getch();
closegraph();
}

