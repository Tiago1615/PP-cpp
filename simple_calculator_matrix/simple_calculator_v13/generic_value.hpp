// generic_value.hpp 
// author: Antonio C. Domínguez Brito <antonio.dominguez@ulpgc.es>  
// creation date: November 16th 2025
// Description: this is the header file of a generic value type
  
#ifndef GENERIC_VALUE_HPP
  #define GENERIC_VALUE_HPP
  
  #include <iostream>
  #include <cmath>
  #include <typeinfo>

  #include "matrix.hpp"
  using namespace mat_lib;

  template<typename T>
  class generic_value_interface
  {
    public:

      using element_t=T;
      using function1_t=element_t(element_t);
      using function2_t=element_t(element_t,element_t);

      virtual ~generic_value_interface() {}
      
      virtual generic_value_interface* make_copy() const =0;

      virtual generic_value_interface* make_transpose() const =0;
      virtual generic_value_interface* make_negative() const =0;
      
      virtual bool equal(const generic_value_interface* v) const =0;

      virtual generic_value_interface* add(const generic_value_interface* v) const =0;
      virtual generic_value_interface* substract(const generic_value_interface* v) const =0;
      virtual generic_value_interface* multiply(const generic_value_interface* v) const =0;
      virtual generic_value_interface* divide(const generic_value_interface* v) const =0;
      virtual generic_value_interface* mod(const generic_value_interface* v) const =0;

      virtual generic_value_interface* call_function(function1_t* f) const =0;
      virtual generic_value_interface* call_function(
        function2_t* f,
        const generic_value_interface* v
      ) const =0;

      virtual void to_stream(ostream& os) const =0;

  };

  template<typename T>
  class scalar_value;

  template<typename T>
  class matrix_value;

  template<typename T>
  class generic_value 
  {
    public:

      using interface_t=generic_value_interface<T>;
      using element_t=typename interface_t::element_t;

      using scalar_t=scalar_value<element_t>;
      using matrix_t=matrix_value<element_t>;

      generic_value(): ptr__{nullptr} {}

      generic_value(const generic_value& v) { if(v.ptr__) ptr__=v.ptr__->make_copy(); }
      generic_value(generic_value&& v): ptr__{v.ptr__} { v.ptr__=nullptr; }

      generic_value(interface_t* v): ptr__{v} {}

      generic_value(const typename scalar_t::value_t& v)
      : ptr__{
          static_cast<interface_t*>(
            new scalar_t(v)
          )
        } 
      {}

      generic_value(const typename matrix_t::value_t& v)
      : ptr__{
          static_cast<interface_t*>(
            new matrix_t(v)
          )
        } 
      {}

      generic_value(typename matrix_t::value_t&& v)
      : ptr__{
          static_cast<interface_t*>(
            new matrix_t(move(v))
          )
        } 
      {}

      ~generic_value() { delete ptr__; }

      generic_value& operator=(const generic_value& v) 
      {
        if(this!=&v)
        {
          delete ptr__;
          ptr__=((v.ptr__)? v.ptr__->make_copy(): nullptr);
        }
        return *this;
      }

      generic_value& operator=(generic_value&& v) 
      {
        delete ptr__;
        ptr__=v.ptr__; v.ptr__=nullptr;
        return *this;
      }

      generic_value operator~() const 
      { 
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->make_transpose(); 
        return gv; 
      }

      generic_value operator-() const  
      { 
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->make_negative(); 
        return gv; 
      }

      bool operator==(const generic_value& v) const
      { 
        throw_if_nullptr__();

        return ptr__->equal(v.ptr__); 
      }

      generic_value operator+(const generic_value& v) const
      { 
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->add(v.ptr__); 
        return gv; 
      }

      generic_value operator-(const generic_value& v) const
      { 
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->substract(v.ptr__); 
        return gv; 
      }

      generic_value operator*(const generic_value& v) const
      { 
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->multiply(v.ptr__); 
        return gv; 
      }

      generic_value operator/(const generic_value& v) const
      { 
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->divide(v.ptr__); 
        return gv; 
      }

      generic_value operator%(const generic_value& v) const
      { 
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->mod(v.ptr__); 
        return gv; 
      }

      generic_value call_function(typename interface_t::function1_t* f) const
      {
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->call_function(f); 
        return gv; 
      }

      generic_value call_function(
        typename interface_t::function2_t* f,
        const generic_value& v
      ) const 
      {
        throw_if_nullptr__();

        generic_value gv;
        gv.ptr__=ptr__->call_function(f,v.ptr__); 
        return gv; 
      }

      void to_stream(ostream& os) const 
      { 
        if(ptr__) ptr__->to_stream(os);
        else os<<"null generic value!";
      }

      template<typename U>
      typename U::value_t get()
      {
        throw_if_nullptr__();

        U* u=dynamic_cast<U*>(ptr__);
        if(!u) 
        {
          ostringstream ostr;
          ostr<<__func__<<": cannot return a "<<typeid(U).name()<<"!";
          throw logic_error(ostr.str());
        }
        
        return u->value;
      }

    private:

      interface_t* ptr__;

      void throw_if_nullptr__() const { if(!ptr__) throw runtime_error("null generic value"); }
  };

  template<typename T>
  ostream& operator<<(ostream& os, const generic_value<T>& v)
  { v.to_stream(os); return os; }

  template<typename T>
  struct scalar_value: public generic_value_interface<T>
  {
      using interface_t=generic_value_interface<T>;
      using element_t=typename generic_value_interface<T>::element_t;

      using scalar_t=typename generic_value<T>::scalar_t;
      using matrix_t=typename generic_value<T>::matrix_t;

      using value_t=element_t;

      scalar_value(): value{0} {}
      scalar_value(element_t v): value{v} {}

      interface_t* make_copy() const override
      { return new scalar_value(value); }

      interface_t* make_transpose() const override { return new scalar_t{value}; }
      interface_t* make_negative() const override { return new scalar_t{-value}; }
      
      bool equal(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m)
        {
          if(m->value.size()!=1) return false;
          return (value==m->value[0][0]);
        }
        else return (value==static_cast<const scalar_t*>(v)->value);

      }

      interface_t* add(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m)
        {
          if(m->value.size()!=1) throw logic_error("cannot add a scalar and a matrix!");
          return new scalar_t(value+m->value[0][0]);
        }
        else 
          return new scalar_t{value+static_cast<const scalar_t*>(v)->value};
      }

      interface_t* substract(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m)
        {
          if(m->value.size()!=1) throw logic_error("cannot add a scalar and a matrix!");
          return new scalar_t(value-m->value[0][0]);
        }
        else 
          return new scalar_t(value-static_cast<const scalar_t*>(v)->value);
      }

      interface_t* multiply(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m)
        {
          matrix_t* mm=static_cast<matrix_t*>(v->make_copy());
          mm->value*=value;
          return mm;
        }
        else
          return new scalar_t(value*static_cast<const scalar_t*>(v)->value);
      }

      interface_t* divide(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m)
        {
          if(m->value.size()!=1) throw logic_error("cannot divide a scalar by a matrix!");
          if(m->value[0][0]==0) throw logic_error("cannot divide by zero!");
          return new scalar_t(value/m->value[0][0]);
        }
        else
        {
          auto vv=static_cast<const scalar_t*>(v)->value;
          if(vv==0) throw logic_error("cannot divide by zero!");
          return new scalar_t(value/vv);
        }
      }

      interface_t* mod(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m)
        {
          if(m->value.size()!=1) throw logic_error("cannot divide a scalar by a matrix!");
          if(m->value[0][0]==0) throw logic_error("cannot divide by zero!");
          return new scalar_t(fmod(value,m->value[0][0]));
        }
        else
        {
          auto vv=static_cast<const scalar_t*>(v)->value;
          if(vv==0) throw logic_error("cannot divide by zero!");
          return new scalar_t(fmod(value,vv));
        }
      }

      interface_t* call_function(typename interface_t::function1_t* f) const override
      { return new scalar_t(f(value)); }

      interface_t* call_function(
        typename interface_t::function2_t* f, 
        const interface_t* v
      ) const override
      { 
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m) 
          throw logic_error("second argument cannot be a matrix!");
        else 
          return new scalar_t(f(value,static_cast<const scalar_t*>(v)->value)); 
      }

      void to_stream(ostream& os) const override { os<<value; }

      void throw_if_nullptr(const interface_t* ptr) const 
      { if(!ptr) throw runtime_error("null generic value");}

      element_t value;
  };

  template<typename T>
  struct matrix_value: public generic_value_interface<T>
  {
      using interface_t=generic_value_interface<T>;
      using element_t=typename generic_value_interface<T>::element_t;

      using scalar_t=typename generic_value<T>::scalar_t;
      using matrix_t=typename generic_value<T>::matrix_t;

      using value_t=matrix<element_t>;

      matrix_value(): value{} {}
      matrix_value(const matrix<T>& v): value{v} {}
      matrix_value(matrix<T>&& v): value{v} {}

      interface_t* make_copy() const override
      { return new matrix_value(value); }

      interface_t* make_transpose() const override { return new matrix_t{~value}; }
      interface_t* make_negative() const override { return new matrix_t{-value}; }

      bool equal(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const scalar_t* s=dynamic_cast<const scalar_t*>(v);
        if(s)
        {
          if(value.size()!=1) return false;
          return (value[0][0]==s->value);
        }
        else 
          return (value==static_cast<const matrix_t*>(v)->value);
      }

      interface_t* add(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const scalar_t* s=dynamic_cast<const scalar_t*>(v);
        if(s)
        {
          if(value.size()!=1) throw logic_error("cannot add a matrix and a scalar!");
          return new scalar_t(s->value+value[0][0]);
        }
        else 
          return new matrix_t(value+static_cast<const matrix_t*>(v)->value);
      }

      interface_t* substract(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const scalar_t* s=dynamic_cast<const scalar_t*>(v);
        if(s)
        {
          if(value.size()!=1) throw logic_error("cannot substract a matrix and a scalar!");
          return new scalar_t(s->value-value[0][0]);
        }
        else 
          return new matrix_t(value-static_cast<const matrix_t*>(v)->value);
      }

      interface_t* multiply(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const scalar_t* s=dynamic_cast<const scalar_t*>(v);
        if(s)
        {
          matrix_t* mm=new matrix_t{value};
          mm->value*=s->value;
          return mm;
        }
        else
          return new matrix_t(value*static_cast<const matrix_t*>(v)->value);
      }

      interface_t* divide(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m) 
          throw logic_error("cannot divide by a matrix");
        else 
        {
          auto vv=static_cast<const scalar_t*>(v)->value;
          if(vv==0) throw logic_error("cannot divide by zero!");
          return new matrix_t(value/vv);
        }
      }

      interface_t* mod(const interface_t* v) const override
      {
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m) 
          throw logic_error("cannot divide by a matrix");
        else 
        {
          auto vv=static_cast<const scalar_t*>(v)->value;
          if(vv==0) throw logic_error("cannot divide by zero!");
          return new matrix_t(value%vv);
        }
      }

      interface_t* call_function(typename interface_t::function1_t* f) const
      { 
        auto mm=matrix<element_t>(value.rows(),value.columns());
        for(size_t i=0; i<value.rows(); i++)
          for(size_t j=0; j<value.columns(); j++) mm[i][j]=f(value[i][j]);
        return new matrix_t(mm); 
      }

      interface_t* call_function(
        typename interface_t::function2_t* f, 
        const interface_t* v
      ) const
      { 
        throw_if_nullptr(v);

        const matrix_t* m=dynamic_cast<const matrix_t*>(v);
        if(m) 
          throw logic_error("second argument cannot be a matrix!");
        else
        {
          auto mm=matrix<element_t>(value.rows(),value.columns());
          auto vv=static_cast<const scalar_t*>(v)->value;
          for(size_t i=0; i<value.rows(); i++)
            for(size_t j=0; j<value.columns(); j++) mm[i][j]=f(value[i][j],vv);
          return new matrix_t(mm); 
        }
      }
      
      void to_stream(ostream& os) const override { os<<value; }

      void throw_if_nullptr(const interface_t* ptr) const
      { if(!ptr) throw runtime_error("null generic value"); }

      matrix<element_t> value;
  };

#endif // GENERIC_VALUE_HPP



