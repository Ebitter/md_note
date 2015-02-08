//6.1 运算符重载--Fraction类
//Fraction.h
#ifndef FRACTION_H
#define FRACTION_H
#include<iostream>
using std::istream;
using std::ostream;

class Fraction
{	
	friend istream& operator>>(istream&,Fraction&);//全局函数重载>>
	friend ostream& operator<<(ostream&,const Fraction&);//全局函数重载<<
	friend Fraction operator!(const Fraction&);//全局函数重载一元运算符
   //全局函数重载二元运算符
	friend Fraction operator+(const Fraction&,const Fraction&);
	friend Fraction& operator++(Fraction&);//全局函数重载前缀++运算符
	friend Fraction operator++(Fraction&,int);//全局函数重载后缀++运算符
public:
	Fraction(int=0,int=1);//默认构造函数
	Fraction(double);//根据double构造Fraction	
	void set(int,int);
	Fraction operator-()const;//成员函数重载一元运算符
	Fraction operator-(const Fraction&)const;//成员函数重载二元运算符
	operator double()const;//类型转换
	Fraction& operator--();//成员函数重载前缀--算符
	Fraction operator--(int);//成员函数重载后缀--算符
private:
	int molecular;//分子
	int denominator;//分母
	static int gcd(int,int);//求两个整数的最大公约数
};
#endif

//Fraction.cpp
#include "Fraction.h"
#include<cmath>

//求两个整数的最大公约数
int Fraction::gcd(int a,int b)
{
	while(b)
	{
		int t=a;
		a=b;
		b=t%b;
	}
	return a;
}
//默认构造函数
Fraction::Fraction(int m,int d)
{
	set(m,d);
}
//根据double构造Fraction
Fraction::Fraction(double db)
{
	int m=0;
	int d=1;
	const double precision=0.000001; 
	double tmp=fabs(db);
	while(1)
	{
		if(m*1.0/d>tmp) 
			++d;
		if(m*1.0/d<tmp)
			++m;
		if(fabs(m*1.0/d-tmp)<precision) break;
	}
	if(db<0)
		m=-m;
	molecular=m;
	denominator=d;
}	
void Fraction::set(int m,int d)
{
	if(d==0)
		d=1;
	int t=gcd(m,d);
	molecular=m/t;
	denominator=d/t;
}
//成员函数重载一元运算符
Fraction Fraction::operator-()const
{
	return Fraction(-molecular,denominator);
}
//成员函数重载二元运算符
Fraction Fraction::operator-(const Fraction& other)const
{
	int m=other.denominator*molecular-denominator*other.molecular;
	int d=denominator*other.denominator;
	return Fraction(m,d);
}
//类型转换
Fraction::operator double()const
{
	return molecular*1.0/denominator;
}
//成员函数重载前缀--算符
Fraction& Fraction::operator--()
{
	molecular-=denominator;
	return *this;
}
//成员函数重载后缀--算符
Fraction Fraction::operator--(int)
{
	Fraction tmp=*this;
	molecular-=denominator;
	return tmp;
}

//全局函数重载>>
istream& operator>>(istream& in,Fraction& fraction)
{
	int m=0;
	int d=1;
	in>>m;
	in.ignore();
	in>>d;
	fraction.set(m,d);
	return in;
}
//全局函数重载<<
ostream& operator<<(ostream& out,const Fraction& fraction)
{
	if(double(fraction)<0)
		out<<"-";
	out<<fabs(fraction.molecular);
	if(fraction.molecular!=0&&fraction.denominator!=1)
		out<<"/"<<fabs(fraction.denominator);
	return out;
}
//全局函数重载一元运算符
Fraction operator!(const Fraction& fraction)
{
	Fraction tmp;
	tmp.set(fraction.denominator,fraction.molecular);
	return tmp;
}
//全局函数重载二元运算符
Fraction operator+(const Fraction& fraction1,const Fraction& fraction2)
{
	int m=fraction2.denominator*fraction1.molecular+
              fraction1.denominator*fraction2.molecular;
	int d=fraction1.denominator*fraction2.denominator;
	return Fraction(m,d);
}
//全局函数重载前缀++运算符
Fraction& operator++(Fraction& fraction)
{
	fraction.molecular+=fraction.denominator;
	return fraction;
}
//全局函数重载后缀++运算符
Fraction operator++(Fraction& fraction,int)
{	
	Fraction tmp=fraction;
	fraction.molecular+=fraction.denominator;
   return tmp;
}
//test.cpp
#include<iostream>
#include"Fraction.h"
using std::cin;
using std::cout;
using std::endl;

int main()
{
   //测试默认构造函数
	Fraction a;
	cout<<"a="<<a<<endl;
	//根据double构造Fraction的构造函数
	Fraction b(0.333333);
	cout<<"b="<<b<<endl;
	//测试set和gcd函数
	a.set(-25,35);
	cout<<"a.set(-25,35)之后"<<endl;
	cout<<"a="<<a<<endl;
	//测试成员函数重载的一元运算符-
	cout<<"-a="<<a.operator -()<<endl;
	cout<<"-a="<<-a<<endl;
	//测试成员函数重载的二元运算符-
	cout<<"a-b="<<a.operator -(b)<<endl;
	cout<<"a-b="<<a-b<<endl;
	//测试全局函数重载的一元运算符!
	cout<<"a的倒数="<<operator!(a)<<endl;
	cout<<"a的倒数="<<!a<<endl;
	//测试全局函数重载的二元运算符+
	cout<<"a+b="<<operator +(a,b)<<endl;
	cout<<"a+b="<<a+b<<endl;
	//测试流提取运算符>>
	cout<<"输入一个分数，分子和分母使用一个字符分隔"<<endl;
	cin>>a;
	cout<<"a="<<a<<endl;
	//测试类型转换double
	double d1=a;//隐式转换
	double d2=static_cast<double>(a);//显式转换
	cout<<"d1="<<d1<<endl;
	cout<<"d2="<<d2<<endl;
	//测试全局函数重载的前缀++和后缀++
	cout<<"b=++a之前"<<endl;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;
	b=++a;
	cout<<"b=++a之后"<<endl;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;
	cout<<"b=a++之前"<<endl;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;
	b=a++;
   cout<<"b=a++之后"<<endl;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;	
	cout<<"++++a="<<++++a<<endl;
	//测试成员函数重载的前缀--和后缀--
	cout<<"b=--a之前"<<endl;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;
	cout<<"b=--a之后"<<endl;
	b=--a;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;
	cout<<"b=a--之前"<<endl;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;
	b=a--;
	cout<<"b=a--之后"<<endl;
	cout<<"a="<<a<<endl;
	cout<<"b="<<b<<endl;	
	cout<<"----a="<<----a<<endl;

	return 0;
}
