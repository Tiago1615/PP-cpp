// testing_generic_value.hpp 
// author: Antonio C. Domínguez Brito <antonio.dominguez@ulpgc.es>  
// creation date: november 16th 2025
// Description: This is a program for testing generic_value classes 

#include <iostream>
using namespace std;

#include "generic_value.hpp"

using gv=generic_value<double>;

int main(int arc, char* argv[])
try
{
  cout<<"This a program for testing generic_value classes"<<endl;

  gv a(
    matrix<double>{
      {1,  2,  3,  4},
      {5,  6,  7,  8},
      {9, 10, 11, 12}
    }
  ); 
  cout<<"a="<<a<<endl;

  gv b(.123456); 
  cout<<"b="<<b<<endl;

  cout<<"a/b="<<a/b<<endl;
  cout<<"a%3="<<a%gv(3)<<endl;

  cout<<"b*a="<<b*a<<endl;
  cout<<"b*a%3="<<b*a%3<<endl;

  cout<<"(10+b)%3="<<(gv(10)+b)%3<<endl;

  gv c(
    matrix<double>{
      {12, 11, 10, 9},
      {8, 7, 6, 5},
      {4, 3, 2, 1}
    }
  ); 
  cout<<"c="<<c<<endl;

  gv d(3);
  cout<<"d="<<d<<endl;

  cout<<"d*b="<<d*b<<endl;

  cout<<"~a="<<~a<<endl;
  cout<<"-a="<<-a<<endl;

  cout<<"a*(~a)="<<a*~a<<endl;

  cout<<"a*4+a="<<a*gv(4)+a<<endl;
  cout<<"4*a+a="<<gv(4)*a+a<<endl;

  cout<<"a+c="<<a+c<<endl;

  cout<<"a-c="<<a-c<<endl;

  gv i(
    matrix<double>{
      {1, 0, 0},
      {0, 1, 0},
      {0, 0, 1},
    }
  );
  cout<<"i="<<i<<endl;
  cout<<"i*i="<<i*i<<endl;

  auto k=a-c+gv(3)*a;
  cout<<"k=a-c+3*a="<<k<<endl;

  //cout<<"cos(a)="<<a.call_function(cos)<<endl;
  //cout<<"cos(b)="<<b.call_function(cos)<<endl;
  //cout<<"pow(a,2)="<<a.call_function(pow,gv(2))<<endl;
  //cout<<"pow(b,a)="<<b.call_function(pow,a)<<endl;

  return 0; 
}
catch(exception& e)
{ cerr<<"\n[EXCEPTION] "<<e.what()<<endl; }
