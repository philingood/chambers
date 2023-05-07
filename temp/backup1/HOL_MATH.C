#include <alloc.h>
#include <math.h>

#include "hol3.h"


static int sign(double x)
{
return ((x<0)?-1:(x>0)?1:0);
}

double dihotom (double a, double b, double eps, double (*f)(double))
/* �襭�� �ࠢ����� f(x)=0 ��⮤�� ���⮬�� */
{
double x,r,eps1=1e-10;	/* ����� �㬮� */
int i;

if(a>b){r=a;a=b;b=r;}
i=sign((*f)(a));
while((b-a)>eps)
        {
	x=(a+b)*0.5;
	r=(*f)(x);
	if(fabs(r)<eps1) break;
	if(sign(r)==i) a=x; else b=x;
	}
return x;
}

double horner(double x, int n, double * a)
{ int i;
  double y;
  for(i=--n,y=a[i];i>0;)
	y=y*x+a[--i];
  return y;
}

/* ���᫥��� ᯫ����� */

double sp(int pts, double * sx, double * sy, double * css, double x1)
{ int i;
  double p,b,d,r,q;
  if(x1<sx[0]) /* ����࠯����� ����� */
        {
	p=sx[1]-sx[0];
	q=sx[0]-x1;
	b=sy[1]-sy[0];
	r=sy[0]-b*q/p;
	return r;
	}
  if(x1>sx[pts-1])     /* ����࠯����� ���। */
        {
	i=pts-2;
	p=sx[i+1]-sx[i];
	q=x1-sx[i];
	b=sy[i+1]-sy[i];
	r=sy[i]+b*q/p;
	return r;
	}
  for(i=1;(x1>sx[i]) && (i<pts-1);i++);
  b=sx[i-1];
  q=sx[i]-b;
  r=x1-b;
  p=css[i];
  d=css[i+1];
  b=(sy[i]-sy[i-1])/q-(d+2*p)*q/3;
  d=(d-p)/q*r;
  p=sy[i-1]+r*(b+r*(p+d/3));
  return p;
}

int cs(int pts, double * sx, double * sy, double * css)
{ int i,j,m;
  double a,b,r;
  double *kk;

  if((kk=(double*)calloc(pts+1,sizeof(double)))==NULL)
	error(1,13);

  kk[0]=0;
  for(i=0;i<=pts;i++)
	css[i]=0;
  for(i=1;i<pts;i++)
        {
	j=i-1;
	m=j-1;
	a=sx[i]-sx[j];
	b=sx[j]-sx[m];
	r=2*(a+b)-b*css[j];
	css[i]=a/r;
	kk[i]=(3.0*((sy[i]-sy[j])/a-(sy[j]-sy[m])/b)-b*kk[j])/r;
	}
  css[pts-1]=kk[pts-1];
  for(i=pts-2;i>0;i--) css[i]=kk[i]-css[i]*css[i+1];
  free(kk);
  return 1;
}

double cs_sp(int n, double * x, double * y, double x1)
{
double *css;
double ret;

if((css=(double*)calloc(n+1,sizeof(double)))==NULL)
	error(1,13);
cs(n,x,y,css);
ret=sp(n,x,y,css,x1);
free(css);
return ret;
}

double lin(int pts, double * sx, double * sy, double x1)
/* �������� ���௮���� */
{
int i;
double p,q,b,r;

if(x1<sx[0])	/* ����࠯����� ����� */
	i=1;
else if(x1>sx[pts-1])	/* ����࠯����� ���। */
	i=pts-1;
else {
	for(i=1;i<pts;i++)
		if(sx[i]>x1)
			break;
     }
i--;
p=sx[i+1]-sx[i];
q=sy[i+1]-sy[i];
b=x1-sx[i];
r=sy[i]+b*q/p;
return r;
}
