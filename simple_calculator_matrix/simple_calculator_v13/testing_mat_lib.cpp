// testing_mat_lib.hpp 
// author: Antonio C. Domínguez Brito <antonio.dominguez@ulpgc.es>  
// creation date: october 3rd 2020
// Description: This is a problem for testing mat_lib library

#include <iostream>
using namespace std;

#include "matrix.hpp"
using namespace mat_lib;

using matrix_t=matrix<double>;
//using matrix_t=matrix<complex<double>>;

int main(int arc, char* argv[])
try
{
  cout<<"This is testing_mat_lib!"<<endl;

  matrix_t empty;
  cout<<"empty="<<empty<<endl;

  matrix_t a{
    {1,  2,  3,  4},
    {5,  6,  7,  8},
    {9, 10, 11, 12}
  };
  cout<<"a="<<a<<endl;

  matrix_t b{a};
  cout<<"b="<<b<<endl;

  a+=b;
  cout<<"a="<<a<<endl;

  matrix_t  c=3.5*(a-b);
  cout<<"c="<<c<<endl;

  matrix_t d=a*3-b;
  cout<<"d="<<d<<endl;

  matrix_t e=-a;
  cout<<"e="<<e<<endl;

  matrix_t f=a.make_transpose();
  cout<<"f="<<f<<endl;

  a/=3;
  cout<<"a="<<a<<endl;

  matrix_t a2=a*a.make_transpose();
  cout<<"a2="<<a2<<endl;

  matrix_t l{1, 2, 3, 4, 5, 6};
  cout<<"l="<<l<<endl;

  cout<<"l*lt="<<l*~l<<endl;

  cout<<"lt*l="<<~l*l<<endl;

  matrix_t at=~a;
  cout<<"at="<<at<<endl;
  cout<<"at*a="<<~a*a<<endl;
  cout<<"a*at="<<a*~a<<endl;

  a.save_as("a.matrix");
  at.save_as("at.matrix");
  (~l*l).save_as("ltl.matrix");

  matrix_t g{"ltl.matrix"};
  cout<<"g="<<g<<endl;

  cout<<~matrix_t{"at.matrix"}<<endl;

  cout<<"at="<<at<<endl;
  cout<<boolalpha<<(at==matrix_t{"at.matrix"})<<endl;
  return 0; 
}
catch(exception& e)
{ cerr<<"[EXCEPTION] "<<e.what()<<endl; }
