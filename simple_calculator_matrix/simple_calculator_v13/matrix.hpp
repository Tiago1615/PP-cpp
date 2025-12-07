// matrix.hpp 
// author: Antonio C. Domínguez Brito <antonio.dominguez@ulpgc.es>  
// creation date: september 20th 2020
// Description: This is the header file of class matrix which is a 
// 2D matrix

#ifndef MATRIX_HPP
  #define MATRIX_HPP
  
  #include <initializer_list>
  #include <exception>
  #include <stdexcept>
  #include <system_error>
  #include <iostream>
  #include <fstream>
  #include <sstream>
  #include <cstring>
  #include <string>
  #include <complex>
  #include <cmath>
  #include <vector>
  
  using namespace std;
  
  namespace mat_lib
  {
  
    template<typename T>
    class matrix
    {
      public:

        //using element_t=double;
        using element_t=T;
  
        static_assert(
          is_integral<T>::value || 
          is_floating_point<T>::value ||
          is_same<T,complex<float>>::value ||
          is_same<T,complex<double>>::value ||
          is_same<T,complex<long double>>::value,
          "\n"
          "[ERROR] >>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< [ERROR]\n"
          "[ERROR] >>>> BAD TYPE PROVIDED FOR TEMPLATE mat_lib::matrix! <<<< [ERROR]\n"
          "[ERROR] >>>> ACCEPTED TYPES:                                 <<<< [ERROR]\n"
          "[ERROR] >>>>   - All numeric primitive types                 <<<< [ERROR]\n"
          "[ERROR] >>>>   - std::complex<float>                         <<<< [ERROR]\n"
          "[ERROR] >>>>   - std::complex<double>                        <<<< [ERROR]\n"
          "[ERROR] >>>>   - std::complex<long double>                   <<<< [ERROR]\n"
          "[ERROR] >>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< [ERROR]\n"
        );
  
        matrix() // default constructor
        : rows__{0},
          columns__{0},
          elements__{nullptr}
        {}
  
        matrix(size_t rows, size_t columns)
        : rows__{rows},
          columns__{columns},
          elements__{new element_t[rows*columns]}
        {}
  
        matrix(
          initializer_list<
            initializer_list<element_t>
          > init
        );
  
        matrix(initializer_list<element_t> init)
        : rows__{1}, 
          columns__{init.size()},
          elements__{new element_t[rows__*columns__]}
        { for(size_t i=0; i<init.size(); i++) elements__[i]=init.begin()[i]; }

        matrix(const vector<vector<element_t>>&  rows);
  
        matrix(const vector<element_t>& row)
        : rows__{1}, 
          columns__{row.size()},
          elements__{new element_t[rows__*columns__]}
        { for(size_t i=0; i<row.size(); i++) elements__[i]=row[i]; }
  
        matrix(const matrix& m) // copy constructor
        : elements__{new element_t[m.size()]} 
        { copy_elements__(m); }
  
        matrix(matrix&& m)// move constructor
        : rows__{m.rows__}, 
          columns__{m.columns__},
          elements__{m.elements__}  
        { 
          m.elements__=nullptr;  
          m.rows__=m.columns__=0;
        }
  
        explicit matrix(const string& file_name); // constructor from a file
  
        ~matrix() { delete [] elements__; }
  
        matrix& operator=(const matrix& m) // copy assigment
        {
          if(this==&m) return *this;
  
          if(size()!=m.size())
          {
            delete [] elements__;
            elements__=((m.size())? new element_t[m.size()]: nullptr);
          }
          copy_elements__(m);
          return *this;
        }
  
        matrix& operator=(matrix&& m) // move assigment
        {
          delete [] elements__;
          
          elements__=m.elements__;  
          rows__=m.rows__; columns__=m.columns__;
  
          m.elements__=nullptr;  
          m.rows__=m.columns__=0;
  
          return *this;
        }
  
        size_t size() const { return rows__*columns__; }
        size_t rows() const { return rows__; }
        size_t columns() const { return columns__; }
  
        element_t at(size_t i, size_t j) const; 
  
        element_t* operator[](size_t i) { return &(elements__[row_offset__(i)]); }
        const element_t* operator[](size_t i) const { return &(elements__[row_offset__(i)]); }
  
        bool operator==(const matrix& m) const;
  
        matrix& operator+=(const matrix& m);
        matrix& operator-=(const matrix& m);
  
        matrix& operator*=(element_t scalar)
        {
          for(size_t i=0; i<rows__; i++)
            for(size_t j=0; j<columns__; j++) elements__[offset__(i,j)]*=scalar;
  
          return *this;
        }
  
        matrix& operator/=(element_t scalar);
        matrix& operator%=(element_t scalar);
  
        matrix make_transpose() const
        {
          matrix t(columns__,rows__);
  
          for(size_t i=0; i<t.rows(); i++)
            for(size_t j=0; j<t.columns(); j++)
              t[i][j]=elements__[offset__(j,i)];
  
          return t;
        }
  
        void save_as(const string& file_name) const;
  
      private:
  
        size_t rows__;
        size_t columns__; 
        element_t* elements__;
  
        bool check_initilizer_list__(const initializer_list<initializer_list<element_t>>& init)
        {
          auto column_size=init.begin()[0].size();
          for(size_t i=1; i<init.size(); i++)
            if(init.begin()[i].size()!=column_size) return false;
          return true;
        }

        bool check_rows__(const vector<vector<element_t>>& rows)
        {
          auto column_size=rows[0].size();
          for(size_t i=1; i<rows.size(); i++)
            if(rows[i].size()!=column_size) return false;
          return true;
        }
  
        void copy_elements__(const matrix& m)
        { 
          rows__=m.rows__; columns__=m.columns__;
          for(size_t i=0; i<rows__; i++) 
            for(size_t j=0; j<columns__; j++) elements__[offset__(i,j)]=m.elements__[offset__(i,j)]; 
        }
  
        size_t row_offset__(size_t i) const { return i*columns__; }
        size_t offset__(size_t i,int j) const { return row_offset__(i)+j; }
    };
  
    template<typename T>
    matrix<T>::matrix(
      initializer_list<
        initializer_list<typename matrix<T>::element_t>
      > init)
    {
      if(!check_initilizer_list__(init))
      {
        ostringstream str_stream;
        str_stream<<"column length mismatch! ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
  
      rows__=init.size(); columns__=init.begin()->size();
      elements__=new element_t[rows__*columns__];
  
      for(size_t i=0; i<init.size(); i++)
        for(size_t j=0; j<init.begin()[i].size(); j++)
          elements__[offset__(i,j)]=init.begin()[i].begin()[j];
    }

    template<typename T>
    matrix<T>::matrix(const vector<vector<typename matrix<T>::element_t>>& rows)
    {
      if(!check_rows__(rows))
      {
        ostringstream str_stream;
        str_stream<<"column length mismatch! ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
  
      rows__=rows.size(); columns__=rows[0].size();
      elements__=new element_t[rows__*columns__];
  
      for(size_t i=0; i<rows.size(); i++)
        for(size_t j=0; j<rows[i].size(); j++)
          elements__[offset__(i,j)]=rows[i][j];
    }
  
    template<typename T>
    matrix<T>::matrix(const string& file_name) // constructor from a file
    : rows__{0},
      columns__{0},
      elements__{nullptr}
    {
      ifstream ifs(file_name);
      if(!ifs)
      {
        ostringstream str_stream;
        str_stream<<"cannot open file \""<<file_name<<"\"! ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw logic_error(str_stream.str());
      }
  
      ifs>>rows__>>columns__;
      elements__=new element_t[rows__*columns__];
  
      for(size_t i=0; i<rows__; i++)
        for(size_t j=0; j<columns__; j++)
          ifs>>elements__[offset__(i,j)];
    }
  
    template<typename T>
    typename matrix<T>::element_t matrix<T>::at(size_t i, size_t j) const 
    { 
      if((i>=rows__) || (j>=columns__)) 
      {
        ostringstream str_stream;
        str_stream<<"out_of range exception ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw out_of_range(str_stream.str());
      }
  
      return elements__[offset__(i,j)];
    }
  
    template<typename T>
    bool matrix<T>::operator==(const matrix<T>& m) const
    {
      if((rows__!=m.rows()) || (columns__!=m.columns())) return false; 
  
      for(size_t i=0; i<rows__; i++)
        for(size_t j=0; j<columns__; j++) 
          if(elements__[offset__(i,j)]!=m[i][j]) return false;
      
      return true;
    }
  
    template<typename T>
    matrix<T>& matrix<T>::operator+=(const matrix<T>& m)
    {
      if((rows()!=m.rows()) || (columns()!=m.columns())) 
      {
        ostringstream str_stream;
        str_stream<<"size mismatch! cannot add matrices ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
  
      for(size_t i=0; i<rows__; i++)
        for(size_t j=0; j<columns__; j++) elements__[offset__(i,j)]+=m[i][j];
        
      return *this;
    }
  
    template<typename T>
    matrix<T>& matrix<T>::operator-=(const matrix<T>& m)
    {
      if((rows()!=m.rows()) || (columns()!=m.columns())) 
      {
        ostringstream str_stream;
        str_stream<<"size mismatch! cannot substract matrices ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
  
      for(size_t i=0; i<rows__; i++)
        for(size_t j=0; j<columns__; j++) elements__[offset__(i,j)]-=m[i][j];
        
      return *this;
    }
  
    template<typename T>
    matrix<T>& matrix<T>::operator/=(element_t scalar)
    {
      if(scalar==0) 
      {
        ostringstream str_stream;
        str_stream<<"divide by zero! bad scalar provided ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
      for(size_t i=0; i<rows__; i++)
        for(size_t j=0; j<columns__; j++) elements__[offset__(i,j)]/=scalar;
  
      return *this;
    }

    template<typename T>
    matrix<T>& matrix<T>::operator%=(element_t scalar)
    {
      if(scalar==0) 
      {
        ostringstream str_stream;
        str_stream<<"divide by zero! bad scalar provided ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
      for(size_t i=0; i<rows__; i++)
        for(size_t j=0; j<columns__; j++) 
          elements__[offset__(i,j)]=fmod(elements__[offset__(i,j)],scalar);
  
      return *this;
    }
  
    template<typename T>
    void matrix<T>::save_as(const string& file_name) const
    {
      ofstream ofs(file_name);
      if(!ofs)
      {
        ostringstream str_stream;
        str_stream<<"cannot open file \""<<file_name<<"\"! ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw logic_error(str_stream.str());
      }
  
      ofs<<rows__<<" "<<columns__<<"\n";
      for(size_t i=0; i<rows__; i++)
      {
        for(size_t j=0; j<columns__; j++) ofs<<elements__[offset__(i,j)]<<" ";
        ofs<<"\n";
      }
    }
  
    template<typename T>
    matrix<T> operator+(const matrix<T>& a, const matrix<T>& b)
    {
      if((a.rows()!=b.rows()) || (a.columns()!=b.columns())) 
      {
        ostringstream str_stream;
        str_stream<<"size mismatch! cannot add matrices ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
  
      matrix<T> c{a};
      return c+=b;
    }
  
    template<typename T>
    matrix<T> operator-(const matrix<T>& a, const matrix<T>& b)
    {
      if((a.rows()!=b.rows()) || (a.columns()!=b.columns())) 
      {
        ostringstream str_stream;
        str_stream<<"size mismatch! cannot substract matrices ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
  
      matrix<T> c{a};
      return c-=b;
    }
  
    template<typename T>
    inline matrix<T> operator~(const matrix<T>& m) { return m.make_transpose(); }
  
    template<typename T>
    inline matrix<T> operator-(const matrix<T>& m)
    {
      matrix<T> r{m}; 
  
      for(size_t i=0; i<r.rows(); i++)
        for(size_t j=0; j<r.columns(); j++) r[i][j]=-m[i][j];
  
      return r; 
    }
  
    template<typename T>
    matrix<T> operator*(const matrix<T>& a, const matrix<T>& b)
    {
      if(a.columns()!=b.rows())
      {
        ostringstream str_stream;
        str_stream<<"size mismatch! cannot multiple matrices ("
          <<__func__<<"() in "<<__FILE__<<":"<<__LINE__<<")";
        throw invalid_argument(str_stream.str());
      }
  
      matrix<T> c(a.rows(),b.columns());
  
      for(size_t i=0; i<c.rows(); i++)
        for(size_t j=0; j<c.columns(); j++)
        {
          c[i][j]=0;
          for(size_t k=0; k<a.columns(); k++)
            c[i][j]+=a[i][k]*b[k][j];
        }
  
      return c;
    }
  
    template<typename T>
    inline matrix<T> operator*(const matrix<T>& m, typename matrix<T>::element_t scalar) 
    { matrix<T> r{m}; r*=scalar; return r; }
  
    template<typename T>
    inline matrix<T> operator*(typename matrix<T>::element_t scalar, const matrix<T>& m) 
    { matrix<T> r{m}; r*=scalar; return r; }
  
    template<typename T>
    inline matrix<T> operator/(const matrix<T>& m, typename matrix<T>::element_t scalar) 
    { matrix<T> r{m}; r/=scalar; return r; }

    template<typename T>
    inline matrix<T> operator%(const matrix<T>& m, typename matrix<T>::element_t scalar) 
    { matrix<T> r{m}; r%=scalar; return r; }
  
    template<typename T>
    ostream& operator<<(ostream& os, const matrix<T>& m)
    {
      size_t i,j;
      if(!m.size()) { return os<<"\n{ }"; }
  
      if(m.rows()>1) os<<"\n{ ";
      for(i=0; i<m.rows(); i++)
      {
        os<<"{";
        for(j=0; j<m.columns()-1; j++) os<<m[i][j]<<", ";
        os<<m[i][j]<<"}";
  
        if(i<m.rows()-1) os<<",\n  ";
      }
      if(m.rows()>1)os<<" }";
  
      return os;
    }
  
  } // namespace mat_lib

#endif //MATRIX_HPP
