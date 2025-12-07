/*
	simple_calculator_v11.cpp - Simple calculator (elenventh version, proposed exercise 9)

  This program implements a basic expression calculator.
  Input from cin, output from cout.
  The gramman for input is:

  Statement:
    Help
    Constant
    Assign 
    Expression
    Print
    Precision
    SetPrecision
    Quit

  Print:
    ;
    
  Quit:
    quit

  Precision:
    precision

  Precision:
    set precision Number

  Constant:
    const Name = Expression

  Assign:
    Name = Expression

  Expression:
    Term
    Term + Expression
    Term - Expression

  Term:
    Primary
    Primary * Term 
    Primary / Term 
    Primary % Term 

  Primary:
    { List }
    { Columns }
    Function
    Number
    Name
    ( Expression )
    - Primary
    + Primary
    ~ Primary

  Columns:
    { List }
    { List } , Columns

  List:
    Expression 
    Expression , List

  Function:
    FunctionName ( Expression )
    FunctionName ( Expression , Expression )

  FunctionName:
    sin
    cos
    tan
    asin
    acos
    atan
    exp
    pow
    ln
    log10
    log2

  Number:
    floating-point-literal

  Name:
    a string of letters and numbers
  
  Input comes from cin through the Token_stream called ts.
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include <stack>
#include <cmath>
#include <sstream>
#include <map>
#include <vector>
#include <iomanip>
#include <ios>
using namespace std;

#include "generic_value.hpp"
using gv=generic_value<double>;

#define DEBUG_FUNC false

#define PROGRAM_NAME "simple_calculator"
constexpr size_t version=13;

inline void error(const string& s)
{
	throw runtime_error(s);
}

inline void error(const string& s, const string& s2) { error(s+s2); }

inline void error(char c, const string& s2) 
{ ostringstream ostr; ostr<<c<<s2; error(ostr.str()); }

struct Token 
{
  enum id
  {
    none,
    quit,
    print,
    number,
    name_token,
    const_token,
    char_token,
    help_token,
    function_token,
    precision_token,
    set
  };

  id kind;
  char symbol;
  typename gv::element_t value;
  string name;
  double (*function)(double);

  Token() 
  : kind(id::none), symbol(0), value(0), name(), function(nullptr)
  {}

  Token(id tk) 
  : kind(tk), symbol(0), value(0), name(), function(nullptr)
  {}

  Token(char ch) 
  : kind(id::char_token), symbol(ch), value(0), name(), function(nullptr)
  {}

  Token(typename gv::element_t val)
  : kind(id::number), symbol(0), value(val), name(), function(nullptr)
  {}

  Token(const string& str) 
  : kind(id::name_token), symbol(0), value(0), name(str), function(nullptr) 
  {}

  Token(const string& str, double (*the_function)(double)) 
  : kind(id::function_token), symbol(0), value(0), name(str), function(the_function) 
  {}

  bool is_symbol(char c) const { return ((kind==id::char_token) && (symbol==c)); }
  bool is_number(typename gv::element_t v) const { return ((kind==id::number) && (value==v)); }
  bool is_name(const string& str) const { return ((kind==id::name_token) && (name==str)); }
  bool is_function() const { return (kind==id::function_token); }
};

class Token_stream 
{ 
  private:

    stack<Token> buffer; 
    
  public: 
    
    Token_stream() { } 
    Token get(); 
    void unget(Token t) { buffer.push(t); } 
    void ignore();
};

Token Token_stream::get()
{
  if(!buffer.empty()) 
  { 
    auto t=buffer.top(); 
    buffer.pop(); 
    return t; 
  }

  char ch;
  //cin >> ch;
  do { cin.get(ch); } while(isspace(ch));
  switch (ch) 
  {
    case '(': case ')': 
    case '{': case '}':
    case '+': case '-': 
    case '*': case '/': 
    case '%': case '~': 
    case '=': case ',': 
      return Token(ch);

    case ';':
      return Token(Token::id::print);

    case '.': case '0': case '1': case '2': 
    case '3': case '4': case '5': case '6': 
    case '7': case '8': case '9':
    {	
      cin.unget();

      double val;
      cin>>val;
      if(!cin) { cin.clear(); error("Bad number"); }

      return Token(val);
    }
    default:
    	if (isalpha(ch)) 
      {
        string s;
        s+=ch;
        while(cin.get(ch) && (isalpha(ch) || isdigit(ch))) s+=ch;
        cin.unget();

        if(s=="quit") return Token(Token::id::quit);
        if(s=="const") return Token(Token::id::const_token);
        if(s=="help") return Token(Token::id::help_token);
        if(s=="set") return Token(Token::id::set);
        if(s=="precision") return Token(Token::id::precision_token);

        if(s=="sin") return Token(s,sin);
        if(s=="cos") return Token(s,cos);
        if(s=="tan") return Token(s,tan);
        if(s=="asin") return Token(s,asin);
        if(s=="acos") return Token(s,acos);
        if(s=="atan") return Token(s,atan);
        if(s=="exp") return Token(s,exp);

        if(s=="pow") return Token(s,nullptr); // WARNING: this is the exception, pow has two arguments
                                            //
        if(s=="ln") return Token(s,log);
        if(s=="log10") return Token(s,log10);
        if(s=="log2") return Token(s,log2);

        return Token(s);
    	}
    	error("Bad token");
  }
}

void Token_stream::ignore()
{
  while(!buffer.empty())
  {
    auto t=buffer.top(); 
    buffer.pop();
    if(t.kind==Token::id::quit) return;
  }

  char ch;
  while (cin>>ch)
    if (ch==';') return;
}

struct Value 
{
  string name;
  gv value;
  bool is_const;

  Value() :name{}, value{double(0)}, is_const{false} {}

  Value(const string& n, const gv& v, bool is_constant=false) 
    :name(n), value(v), is_const(is_constant) 
  {}
};

struct Defined_function
{
  vector<string> params;
  vector<Token> expression;
};

map<string,Value> names;
map<string,Defined_function> functions;

gv get_value(const string& s)
{
  auto iter=names.find(s);
  if(iter!=names.end()) return iter->second.value;

  error("get: undefined name ",s);
}

void set_value(const string& s,const gv& v)
{
  auto iter=names.find(s);
  if(iter!=names.end())
  {
    if(iter->second.is_const) error("set: const name ",s);
    iter->second.value=v;
    return;
  }

  error("set: undefined name ",s);
}

bool is_constant(const string& s)
{
  auto iter=names.find(s);
  return (
    (iter!=names.end()) && 
    (iter->second.is_const)
  );
}

bool is_declared(const string& s) { return (names.find(s)!=names.end()); }

void define_name(const string& s, const gv& d, bool constant=false)
{ names[s]=Value(s,d,constant); }

Token_stream ts;

constexpr int default_precision=6;
int precision=default_precision;

gv expression();
gv define_function(const string& name, const vector<Token>& param_tokens);
gv evaluate_function(const string& name, const vector<gv>& args);
bool last_statement_was_function_definition = false;

gv function_name()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(!t.is_function()) error("function name expected");

  Token tt=ts.get();
  if(!tt.is_symbol('(')) error("'(' expected");
  gv v=expression();
  tt=ts.get();
  if(tt.is_symbol(')')) 
  {
    if(t.function) return v.call_function(t.function);
    else error(t.name," needs two arguments");
  }
  else if(!tt.is_symbol(',')) error("')' expected");
  {
    gv vv=expression();
    tt=ts.get();
    if(tt.is_symbol(')')) 
    {
      if(t.name=="pow") return v.call_function(pow,vv); 
      else error(t.name," needs only one argument");
    }
    else error("')' expected");
  }
}

vector<typename gv::matrix_t::value_t::element_t> list()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  vector<typename gv::matrix_t::value_t::element_t> row; 
  Token t;
  do
  {
    auto v=expression();
    row.push_back(v.get<typename gv::scalar_t>());
    t=ts.get();
  } while(t.is_symbol(','));
  ts.unget(t);
  return row;
}

gv columns()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  vector<
    vector<typename gv::matrix_t::value_t::element_t> 
  > rows;

  Token t,tt;
  do
  {
    t=ts.get();
    if(!t.is_symbol('{')) error("'{' expected");
    
    tt=ts.get();
    if(tt.is_symbol('}')) 
      rows.push_back(vector<typename gv::matrix_t::value_t::element_t>()); 
    else 
    {
      ts.unget(tt);

      rows.push_back(list());
      t=ts.get();
      if(!t.is_symbol('}')) error("'}' expected");
    }
    t=ts.get();
  } while(t.is_symbol(','));
  ts.unget(t);

  return gv(typename gv::matrix_t::value_t(rows));
}

gv primary()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();

  if(t.is_function()) { ts.unget(t); return function_name(); }
  else if(t.kind==Token::id::char_token)
  {
    if(t.is_symbol('('))
    {
      gv v=expression();
      t=ts.get();
      if(!t.is_symbol(')')) error("')' expected");
      return v;
    }
    else if(t.is_symbol('{'))
    {
      Token tt=ts.get();
      if(tt.is_symbol('{')) 
      { 
        ts.unget(tt); 
        gv v=columns();
        t=ts.get();
        if(!t.is_symbol('}')) error("'}' expected");  
        return v; 
      }
      else if(tt.is_symbol('}')) return gv{typename gv::matrix_t::value_t()};  
      { 
        ts.unget(tt);
        gv v{typename gv::matrix_t::value_t(list())};
        t=ts.get();
        if(!t.is_symbol('}')) error("'}' expected");  
        return v;
      }
    }
    else if(t.is_symbol('-')) return -primary();
    else if(t.is_symbol('+')) return primary();
    else if(t.is_symbol('~')) return ~primary();
  }
  else if(t.kind==Token::id::number) return gv(t.value);
  else if(t.kind==Token::id::name_token) {
    string fname = t.name;
    Token next = ts.get();

    // ¿Es una llamada a función de usuario? f( ... )
    if (next.is_symbol('(') && functions.find(fname) != functions.end()) {
      // Leer argumentos: f(arg1, arg2, ...)
      vector<gv> args;
      Token tok = ts.get();

      if (!tok.is_symbol(')')) {
        ts.unget(tok);
        while (true) {
          args.push_back(expression());
          Token sep = ts.get();
          if (sep.is_symbol(')')) break;
          if (!sep.is_symbol(',')) error("',' or ')' expected in argument list");
        }
      }

      return evaluate_function(fname, args);
    }

    // No es función de usuario -> tratamos como variable normal
    ts.unget(next);
    return get_value(fname);
  }

  error("primary expected");
}

gv term()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  gv left = primary();
  while(true) 
  {
    Token t = ts.get();

    if(t.is_symbol('*')) left=left*primary();
    else if(t.is_symbol('/')) left=left/primary();
    else if(t.is_symbol('%')) left=left%primary();
    else { ts.unget(t); return left; }
  }
}

gv expression()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  gv left = term();
  while(true) 
  {
    Token t = ts.get();

    if(t.is_symbol('+')) left=left+term();
    else if(t.is_symbol('-')) left=left-term();
    else { ts.unget(t); return left; }
  }
}

gv evaluate_function(const string& fname, const vector<gv>& args)
{
  #if DEBUG_FUNC
    cout << __func__ << endl;
  #endif

  auto it = functions.find(fname);
  if (it == functions.end()) error("undefined function ", fname);

  Defined_function& def = it->second;

  if (args.size() != def.params.size())
    error("wrong number of arguments in call of function ", fname);

  // 1) Guardar valores anteriores de los parámetros (si existen)
  map<string, Value> old_values;
  for (size_t i = 0; i < def.params.size(); ++i) {
    const string& pname = def.params[i];
    auto itv = names.find(pname);
    if (itv != names.end()) {
      old_values[pname] = itv->second;
    }
    // Definir/actualizar parámetro como variable normal (no const)
    names[pname] = Value(pname, args[i], false);
  }

  // 2) Inyectar tokens del cuerpo en el Token_stream
  // Queremos que se lean: body[0], body[1], ..., body[n-1], ';'
  // Como unget() es LIFO, metemos primero el ';' (print) y luego el cuerpo al revés
  ts.unget(Token(Token::id::print));  // sentinel al final del cuerpo

  for (int i = int(def.expression.size()) - 1; i >= 0; --i) {
    ts.unget(def.expression[i]);
  }

  // 3) Evaluar el cuerpo como una expresión normal
  gv result = expression();

  // 4) Consumir el sentinel ';' que quedó en el stream
  Token t = ts.get();
  if (t.kind != Token::id::print) {
    // Por si acaso, lo devolvemos si no era ';'
    ts.unget(t);
  }

  // 5) Restaurar entorno de variables
  for (size_t i = 0; i < def.params.size(); ++i) {
    const string& pname = def.params[i];
    auto itold = old_values.find(pname);
    if (itold != old_values.end()) {
      names[pname] = itold->second;
    } else {
      names.erase(pname);
    }
  }

  return result;
}

gv define_function(const string& name, const vector<Token>& param_tokens)
{
#if DEBUG_FUNC
  cout << __func__ << endl;
#endif

  // 1) Parsear la lista de parámetros a partir de los tokens
  // param_tokens contiene: x , y , z ... (sin el '(' ni el ')')
  vector<string> params;
  if (!param_tokens.empty()) {
    bool expect_name = true;
    for (size_t i = 0; i < param_tokens.size(); ++i) {
      const Token& t = param_tokens[i];
      if (expect_name) {
        if (t.kind != Token::id::name_token)
          error("parameter name expected in definition of function ", name);
        params.push_back(t.name);
        expect_name = false;
      } else {
        if (!t.is_symbol(','))
          error("',' expected in parameter list of function ", name);
        expect_name = true;
      }
    }
    if (expect_name) {
      error("trailing ',' in parameter list of function ", name);
    }
  }

  // 2) Leer el '=' ya lo ha consumido quien llama (statement())

  // 3) Leer el cuerpo hasta ';'
  vector<Token> body;
  while (true) {
    Token tok = ts.get();
    if (tok.kind == Token::id::print) break;  // ';'
    body.push_back(tok);
  }

  // 4) Guardar (o redefinir) la función
  functions[name] = Defined_function{params, body};

  // Indicamos que el último statement fue una definición
  last_statement_was_function_definition = true;

  // El valor de retorno no importa mucho, el print lo trataremos aparte
  return gv(0.0);
}

gv assign()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error ("name expected in assign");
  string name = t.name;
  if (is_constant(name)) error(name," constant cannot be modified"); 
  t=ts.get();

  if(!t.is_symbol('=')) error("= missing in assign of " ,name);

  gv v=expression();

  if(is_declared(name)) 
    set_value(name,v);
  else
    define_name(name,v);

  return v;
}

gv constant_assign()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error("name expected in const assign");
  string name = t.name;
  if(is_declared(name)) error(name," has already been defined"); 
  t=ts.get();

  if(!t.is_symbol('=')) error("= missing in assign of " ,name);

  gv v=expression();

  define_name(name,v,true);

  return v;
}

gv statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();

  switch(t.kind)
  {
    case Token::id::const_token:
      return constant_assign();
      break;

    case Token::id::name_token:
      {
        Token name_token = t;
        Token next = ts.get();

        // ¿Puede ser una definición de función? f(...)
        if (next.is_symbol('(')) {
          // Leemos todo hasta la ')' correspondiente para ver qué hay después
          vector<Token> between_parens;   // tokens entre '(' y ')', más la ')'
          int level = 1;

          while (level > 0) {
            Token x = ts.get();
            between_parens.push_back(x);
            if (x.is_symbol('(')) ++level;
            else if (x.is_symbol(')')) --level;
          }
          // Ahora between_parens incluye TODO desde el primer token tras '(' hasta la ')'

          Token after = ts.get(); // token justo después de los paréntesis

          if (after.is_symbol('=')) {
            // DEFINICIÓN DE FUNCIÓN: f(x,y)=...

            // Quitamos la ')' del final para quedarnos solo con los parámetros
            if (!between_parens.empty() && between_parens.back().is_symbol(')')) {
              between_parens.pop_back();
            }

            return define_function(name_token.name, between_parens);
          } else {
            // NO es definición (es una expresión que empieza por f(...))
            // Devolvemos todo al stream para que lo procese expression()
            ts.unget(after);
            for (int i = int(between_parens.size()) - 1; i >= 0; --i)
              ts.unget(between_parens[i]);
            ts.unget(Token('('));
            ts.unget(name_token);
            return expression();
          }
        }

        // Si no había '(', puede ser asign o expresión normal
        if (next.is_symbol('=')) {
          ts.unget(next);
          ts.unget(name_token);
          return assign();
        } else {
          ts.unget(next);
        return expression();
        }    ts.unget(name_token);
      }
      break;

    default:
      { ts.unget(t); return expression(); }
  }
}

void clean_up_mess()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
	ts.ignore();
}

void help()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  cout
    <<" "<<PROGRAM_NAME<<" - v"<<version<<" - MUSIANI - Programación y Prototipado"
    <<"\n"
    <<"\n This  is  a simple  calculator  which  accepts  aritmetic"
    <<"\n expressions  with  numeric  literals and simbolic  values"
    <<"\n like constants and variables."
    <<"\n"
    <<"\n Allows also the use of the following fundamental functions:"
    <<"\n"
    <<"\n   sin, cos, tan, asin, acos, atan"
    <<"\n   exp, pow, ln, log10, log2"
    <<"\n"
    <<"\nIt is a matricial calculator, and accepts matrix literals with the following"
    <<"\nnotation (spaces, line feeds and carry returns are skipped):"
    <<"\n"
    <<"\n   {{1,2},{-2,-1}};"
    <<"\n   or"
    <<"\n   {"
    <<"\n     { 122.000000, 244.000000, 366.000000 },"
    <<"\n     { 244.000000, 488.000000, 732.000000 },"
    <<"\n     { 366.000000, 732.000000, 1098.000000 }"
    <<"\n   }"
    <<"\n   or"
    <<"\n   { 0.540302, 0.004426, 0.004426, -0.999961, -0.013277 }"
    <<"\n"
    <<"\n Notation is very simple, as shown in the following examples:"
    <<"\n "
    <<"\n   2+3*7-(8-3.2)-1/3; (aritmetic expression)"
    <<"\n   a=3; (assign of a value to a variable)"
    <<"\n   const pi=3.141592; (assign of a value to a constant)"
    <<"\n   2*pi-a; (arithmetic expression with variables and constants)"
    <<"\n   sin(2*pi/4); (arithmetic expression with functions)"
    <<"\n   const e=exp(1); (arithmetic expression with functions)" 
    <<"\n   ln(2*e/pi); (arithmetic expression with functions)"
    <<"\n   pow(3,2); (arithmetic expression with functions)"
    <<"\n   v={1,1,2,3,5,8,13,21,34} (assign a line vector to a varible)"
    <<"\n   exp(v) (arithmetic expression with functions and a line vector)"
    <<"\n   a={"
    <<"\n     { 122.000000, 244.000000, 366.000000 },"
    <<"\n     { 244.000000, 488.000000, 732.000000 },"
    <<"\n     { 366.000000, 732.000000, 1098.000000 }"
    <<"\n   } (a literal matrix assigned to a varible)"<<"\n"
    <<"\n"
    <<"\n Mind that all expressions should be finished with a symbol ';'."
    <<"\n For finishing the execution type \"quit\"."
    <<"\n"
    <<"\n Additional commands: "
    <<"\n"
    <<"\n   precision; (shows how many fractional digits are used for showing calculator's results)"
    <<"\n   set precision <numeric_expression>; (changes calculator's precision)"
    <<"\n"
  ; 
}

void precision_statement()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
           
  cout
    <<" precision digits: "<<precision<<"\n"
  ;
}

void set_precision()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  Token t=ts.get();
  if(t.kind!=Token::id::precision_token) error("precision keyword expected");

  gv d=expression();
  precision=d.get<gv::scalar_t>();

  cout
    <<" precision set to "<<precision<<" digits\n"
  ;
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
         
  while(true) 
  try 
  {
    cout<<prompt;
    Token t=ts.get();
    while (t.kind==Token::id::print) t=ts.get();
    if(t.kind==Token::id::quit) return;
    if(t.kind==Token::help_token) { help(); continue; }
    if(t.kind==Token::precision_token) { precision_statement(); continue; }
    if(t.kind==Token::set) { set_precision(); continue; }

    ts.unget(t);
    auto the_result=statement();
    cout<<fixed<<setprecision(precision)<<result<<the_result<<endl;
  }
  catch(runtime_error& e) 
  {
    cerr<<e.what()<< endl;
    clean_up_mess();
  }
  catch(logic_error& e) 
  {
    cerr<<e.what()<< endl;
    clean_up_mess();
  }
}

int main()
try 
{
  #if DEBUG_FUNC
    cout<<__func__<<std::endl;
  #endif // DEBUG_FUNC
  help();

  calculate();
  return 0;
}
catch (exception& e) {
  cerr<<"exception: "<<e.what()<<endl;
  char c;
  while((cin>>c) && (c!=';')) ;
  return 1;
}
catch (...) {
  cerr << "exception\n";
  char c;
  while((cin>>c) && (c!=';')) ;
  return 2;
}
