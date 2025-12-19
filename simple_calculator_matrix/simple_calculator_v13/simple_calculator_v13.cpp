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
    inv

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
    set,
    show_env_token,
    save_env_token,
    load_env_token
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
        if (s == "show")return Token(Token::id::show_env_token);
        if (s == "save"){
          return Token(Token::id::save_env_token);
        }
        if (s == "load"){
          return Token(Token::id::load_env_token);
        }

        if(s=="sin") return Token(s,sin);
        if(s=="cos") return Token(s,cos);
        if(s=="tan") return Token(s,tan);
        if(s=="asin") return Token(s,asin);
        if(s=="acos") return Token(s,acos);
        if(s=="atan") return Token(s,atan);
        if(s=="exp") return Token(s,exp);

        if(s=="pow") return Token(s,nullptr); // pow tiene dos argumentos, por eso nullptr
        if (s=="inv") return Token(s, nullptr); // la inversa de una matriz, se trata aparte

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
bool is_function_declared(const string& s)
{
  auto pos = s.find('(');
  string fname = s.substr(0, pos);
  return (functions.find(fname) != functions.end());
}

void define_name(const string& s, const gv& d, bool constant=false)
{ names[s]=Value(s,d,constant); }

Token_stream ts;

constexpr int default_precision=6;
int precision=default_precision;

gv expression();
gv define_function(const string& name, const vector<Token>& param_tokens);
gv evaluate_function(const string& name, const vector<gv>& args);
gv statement();

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
    if (t.name == "inv"){
      if (v.is_matrix()){
        try{
          auto m = v.get<typename gv::matrix_t>();
          auto inv_m = m.make_inverse();
          return gv(inv_m);
        }
        catch (const exception& e){
          error(string("inv function error: ") + e.what());
        }
      }

      if (v.is_scalar()){
        auto n = v.get<typename gv::scalar_t>();
        if (n == 0) error("cannot invert zero");
        return gv(1.0/n);
      }

      error("inv function requires a scalar or a matrix");
    }
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

    if (next.is_symbol('(') && functions.find(fname) != functions.end()) {
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

    // No es función tratar como variable normal
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

  auto target = functions.find(fname);
  if (target == functions.end()) error("undefined function ", fname);

  Defined_function& def = target->second;

  if (args.size() != def.params.size())
    error("wrong number of arguments in call of function ", fname);

  map<string, Value> old_values;
  for (size_t i = 0; i < def.params.size(); ++i) {
    const string& cname = def.params[i];
    auto ctarget = names.find(cname);
    if (ctarget != names.end()) {
      old_values[cname] = ctarget->second;
    }
    // Actualizar parámetro
    names[cname] = Value(cname, args[i], false);
  }
  ts.unget(Token(Token::id::print));

  for (int i = int(def.expression.size()) - 1; i >= 0; --i) {
    ts.unget(def.expression[i]);
  }
  gv result = expression();

  // Consumir ';'
  Token t = ts.get();
  if (t.kind != Token::id::print) {
    ts.unget(t);
  }

  // Restaurar entorno de variables
  for (size_t i = 0; i < def.params.size(); ++i) {
    const string& cname = def.params[i];
    auto target_old = old_values.find(cname);
    if (target_old != old_values.end()) {
      names[cname] = target_old->second;
    } else {
      names.erase(cname);
    }
  }

  return result;
}

gv define_function(const string& name, const vector<Token>& param_tokens)
{
  #if DEBUG_FUNC
    cout << __func__ << endl;
  #endif

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
      }
      else {
        if (!t.is_symbol(','))
          error("',' expected in parameter list of function ", name);
        expect_name = true;
      }
    }
    if (expect_name) {
      error("trailing ',' in parameter list of function ", name);
    }
  }

  vector<Token> body;
  while (true) {
    Token tok = ts.get();
    if (tok.kind == Token::id::print) break;  // ';'
    body.push_back(tok);
  }

  functions[name] = Defined_function{params, body};

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

void set_precision(int digits)
{
  if (digits < 0 || digits > 20)
    error("Precision must be between 0 and 20");
  precision = digits;
  cout.setf(ios::fixed);
  cout.precision(precision);
}

string format_value_for_file(const gv& v)
{
  // Volcar gv a string
  ostringstream oss;
  oss << v;
  string s = oss.str();

  // Recoger tokens ignorando espacios
  istringstream iss(s);
  string token;
  string out;

  while (iss >> token) {
    out += token;
  }

  return out;
}

gv parse_value_expr(const string& expr_str)
{
  streambuf* old_buf = cin.rdbuf();

  istringstream iss(expr_str + ";");
  cin.rdbuf(iss.rdbuf());

  gv result;
  try {
    result = expression();

    Token t = ts.get();
    if (t.kind != Token::id::print) {
      error("Invalid expression in env file");
    }
  }
  catch (...) {
    cin.rdbuf(old_buf);
    throw;
  }

  cin.rdbuf(old_buf);
  return result;
}

string format_function_for_file(const string& fname, const Defined_function& fdef)
{
  ostringstream oss;

  // f(x,y) =
  oss << fname << "(";
  for (size_t i = 0; i < fdef.params.size(); ++i) {
    oss << fdef.params[i];
    if (i + 1 < fdef.params.size()) oss << ",";
  }
  oss << ") = ";

  // Reconstruir la expresión
  for (const auto& tok : fdef.expression) {
    if (tok.kind == Token::id::number) {
      oss << tok.value;
    } 
    else if (tok.kind == Token::id::name_token || tok.kind == Token::id::function_token) {
      oss << tok.name;
    } 
    else if (tok.kind == Token::id::char_token) {
      oss << tok.symbol;
    }
  }

  oss << " ";

  return oss.str();
}

void show_env()
{
  if (names.empty() && functions.empty()) {
    error("\nshow env: (none)\n");
  }

  if (!names.empty()){
    cout << "\nVariables, and constants :" << endl << endl;
    for (const auto& [key, val] : names) {
      cout << "  " << key << " = " << val.value;
      if (val.is_const) cout << " (const)";
      cout << endl << endl;
    }
  }
  else{
    cout << "\nNo variables or constants defined." << endl << endl;
  }

  if (!functions.empty()){
    cout << "\nDefined functions:" << endl << endl;
    for (const auto& [fname, fdef] : functions) {
      cout << "  " << fname << "(";
      for (size_t i = 0; i < fdef.params.size(); ++i) {
        cout << fdef.params[i];
        if (i < fdef.params.size() - 1) cout << ", ";
      }
      cout << ") = ";
      for (const auto& tok : fdef.expression) {
        if (tok.kind == Token::id::number) {
          cout << tok.value << " ";
        } else if (tok.kind == Token::id::name_token || tok.kind == Token::id::function_token) {
          cout << tok.name << " ";
        } else if (tok.kind == Token::id::char_token) {
          cout << tok.symbol << " ";
        }
      }
      cout << endl << endl;
    }
  }
  else{
    cout << "\nNo functions defined." << endl << endl;
  }
}

void save_env(string filename)
{
  if (names.empty() && functions.empty()) {
    error("\nsave env: No variables or user defined functions to save.\n");
  }

  cout 
    << "\n Enter precision for saving:"
    << "\n"
    << "\n1. Default (6 digits)"
    << "\n2. Medium (12 digits)"
    << "\n3. High (19 digits)";
  cout << "\n\nSelect option (1-3): ";

  int option;
  int save_precision;
  bool loop = true;

  while (loop){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cin >> option;

    switch (option){
      case 1:
        save_precision = 6;
        loop = false;
        break;
      case 2:
        save_precision = 12;
        loop = false;
        break;
      case 3:
        save_precision = 19;
        loop = false;
        break;
      default:
        cout << "\nInvalid option. Please select 1, 2, or 3: ";
        break;
    }
  }

  ofstream out(filename);
  if (!out) {
    error("\nsave env: Could not open file for writing\n");
  }

  out.setf(ios::fixed);
  out.precision(save_precision);

  out << "Precision = " << save_precision << endl;

  for (const auto& [key, val] : names) {
    string formatted_value = format_value_for_file(val.value);
    out << key << " = " << formatted_value << " is_const = " << val.is_const << endl;
  }

  if (!functions.empty()){
    for (const auto& [fname, fexp]: functions) {
      string formatted_function = format_function_for_file(fname, fexp);
      out << formatted_function << "is_const = 0" << endl;
    }
  }

  out.close();
  cout << "\nEnvironment saved to " << filename << " with precision of " << save_precision << " digits.\n\n";
}

void load_env(string filename)
{
  ifstream in(filename);
  if (!in) {
    error("\nload env: Could not open file for reading\n");
  }

  string line;

  if (getline(in, line)) {
    istringstream header(line);
    string label, eq;
    int file_precision;
    header >> label >> eq >> file_precision;

    cout << "\nThe file specifies a precision of " << file_precision << " digits.";
    cout << "\nDo you want to apply this precision to future outputs?";
    cout << "\n\n 1. Yes";
    cout << "\n 2. No";
    cout << "\n\nSelect option (1-2): ";

    int option;
    bool loop = true;
    while (loop) {
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');

      cin >> option;

      switch(option){
        case 1:
          set_precision(file_precision);
          cout << "\nPrecision set to " << precision << " digits.\n";
          loop = false;
          break;
        case 2:
          cout << "\nKeeping current precision of " << precision << " digits.\n";
          loop = false;
          break;
        default:
          cout << "\nInvalid option. Please select 1 or 2: ";
          break;
      }
    }
  }

  while (getline(in, line)){
    istringstream stream(line);
    string name;
    string eq;
    string value_str;
    string is_const_str;
    int is_const;

    stream >> name >> eq >> value_str >> is_const_str >> eq >> is_const;

    // Comprobar si es función
    bool is_function = (name.find('(') != string::npos) && (name.find(')') != string::npos);

    if (!is_function){
      gv value = parse_value_expr(value_str);

      if (!is_declared(name)) {
        define_name(name, value, is_const);
        cout << "\nLoaded variable: " << name << " (const: " << (is_const ? "yes" : "no") << ")\n";
      } 
      else {
        cout << "\nConflict detected for variable: " << name << ".\n";
        cout << "\nChoose an action:\n";
        cout << "  1. Keep existing value\n";
        cout << "  2. Overwrite with file value\n";
        cout << "\nSelect option (1-2): ";

        int option;
        bool loop = true;
        while (loop) {
          cin >> option;
          switch (option) {
            case 1:
              cout << "Keeping existing value for '" << name << "'.\n";
              loop = false;
              break;
            case 2:
              names[name] = Value(name, value, is_const);
              cout << "Overwritten '" << name << "' with value from file.\n";
              loop = false;
              break;
            default:
              cout << "Invalid option. Please select 1 or 2: ";
              break;
          }
        }
      }
    }
    else{
      if (!is_function_declared(name)){
        string func = name + " = " + value_str + ";";
        streambuf* old_buf = cin.rdbuf();
        istringstream func_stream(func);
        cin.rdbuf(func_stream.rdbuf());

        try {
          (void)statement();
        }
        catch (...) {
          cin.rdbuf(old_buf);
          throw;
        }

        cin.rdbuf(old_buf);
        cout << "\nLoaded function: " << name << " = " << value_str << "\n";
      }
      else{
        cout << "\nConflict detected for function: " << name << ".\n";
        cout << "Choose an action:\n";
        cout << "  1. Keep existing function\n";
        cout << "  2. Overwrite with file definition\n";
        cout << "\nSelect option (1-2): ";

        int option;
        bool loop = true;

        while (loop){
          cin >> option;
          switch (option) {
            case 1:
              cout << "Keeping existing definition for function '" << name << "'.\n";
              loop = false;
              break;
            case 2:
              {
                streambuf* old_buf = cin.rdbuf();
                istringstream func_stream(name + " = " + value_str + ";");
                cin.rdbuf(func_stream.rdbuf());
                try {
                  (void)statement();
                }
                catch (...) {
                  cin.rdbuf(old_buf);
                  throw;
                }
                cin.rdbuf(old_buf);
                cout << "Overwritten function '" << name << "' with definition from file.\n";
                loop = false;
                break;
              }
            default:
              cout << "Invalid option. Please select 1 or 2: ";
              break;
          }
        }
      }
    }
  }

  in.close();
  cout << "\nEnvironment loaded from " << filename << ".\n\n";
}

string read_filename()
{
  char ch;
  string filename = "";

  cin >> ws;
  while (cin.get(ch) && ch != ';'){
    filename += ch;
  }
  cin.unget();

  if (filename.empty()) error("Filename expected");

  if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") error("\nFilename must end with .txt\n");

  return filename;
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
    case Token::id::show_env_token:
      {
        Token next = ts.get();
        if (next.name != "env") error("Expected 'env' after 'show'");
        show_env();
        return 0.0;
      }
    case Token::id::save_env_token:
      {
        Token next = ts.get();
        if (next.name != "env") error("Expected 'env' after 'save'");
        string filename = read_filename();
        save_env(filename);
        return 0.0;
      }
    case Token::id::load_env_token:
      {      
        Token next = ts.get();
        if (next.name != "env") error("Expected 'env' after 'load'");
        string filename = read_filename();
        load_env(filename);
        return 0.0;
      }
    case Token::id::name_token:
      {
        Token name_token = t;
        Token next = ts.get();

        if (next.is_symbol('(')) {
          vector<Token> params;
          int level = 1;

          while (level > 0) {
            Token x = ts.get();
            params.push_back(x);
            if (x.is_symbol('(')) ++level;
            else if (x.is_symbol(')')) --level;
          }

          Token after = ts.get();

          if (after.is_symbol('=')) {
            // Quitar ')' para dejar solo los parámetros
            if (!params.empty() && params.back().is_symbol(')')) {
              params.pop_back();
            }

            return define_function(name_token.name, params);
          } 
          else {
            // No es definición, devolver todo al stream
            ts.unget(after);
            for (int i = int(params.size()) - 1; i >= 0; --i)
              ts.unget(params[i]);
            ts.unget(Token('('));
            ts.unget(name_token);
            return expression();
          }
        }

        if (next.is_symbol('=')) {
          ts.unget(next);
          ts.unget(name_token);
          return assign();
        } 
        else {
          ts.unget(next);
          ts.unget(name_token);
          return expression();
        }
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
  cout
    << "\n =============================================================="
    << "\n  This is a simple calculator for arithmetic expressions"
    << "\n  supporting variables, constants, matrices and functions."
    << "\n =============================================================="
    << "\n"
    << "\n - Basic Usage:"
    << "\n   - Use ';' to end each statement"
    << "\n   - Type 'quit' to exit the program"
    << "\n   - Example:  a = 5 + 3;"
    << "\n"
    << "\n - Mathematical Functions Supported:"
    << "\n   - Trigonometric:   sin(x), cos(x), tan(x)"
    << "\n   - Inverse trig:    asin(x), acos(x), atan(x)"
    << "\n   - Exponential:     exp(x), pow(x, y)"
    << "\n   - Logarithmic:     ln(x), log10(x), log2(x)"
    << "\n   - Inverse:"
    << "\n       inv(x)  ->  1/x  (for nonzero scalars)"
    << "\n       inv(A)  ->  matrix inverse of A (square, non-singular matrices)"
    << "\n"
    << "\n - Variables and Constants:"
    << "\n   - Assign a variable:      x = 42;"
    << "\n   - Define a constant:      const pi = 3.141592;"
    << "\n   - Use them in expressions: 2*pi - x;"
    << "\n"
    << "\n - Matrices and Vectors:"
    << "\n   This is a matrix calculator. It accepts matrix literals using"
    << "\n   curly braces '{' '}'. Spaces and line breaks are ignored."
    << "\n"
    << "\n   Examples of matrix notation:"
    << "\n     {{1,2},{-2,-1}};"
    << "\n"
    << "\n     {"
    << "\n       { 122.0, 244.0, 366.0 },"
    << "\n       { 244.0, 488.0, 732.0 },"
    << "\n       { 366.0, 732.0, 1098.0 }"
    << "\n     };"
    << "\n"
    << "\n     { 0.540302, 0.004426, 0.004426, -0.999961, -0.013277 };"
    << "\n"
    << "\n   You can assign them to variables and use them in expressions:"
    << "\n     v = {1,1,2,3,5,8,13,21,34};"
    << "\n     pow(v,2);     (apply exp element-wise)"
    << "\n"
    << "\n     a = {"
    << "\n       { 122.0, 244.0, 366.0 },"
    << "\n       { 244.0, 488.0, 732.0 },"
    << "\n       { 366.0, 732.0, 1098.0 }"
    << "\n     };"
    << "\n     a * v;      (matrix–vector / matrix–matrix operations, when valid)"
    << "\n"
    << "\n   Example of matrix inverse:"
    << "\n     A = {{1,2},{3,4}};"
    << "\n     B = inv(A);      (B is the inverse of A)"
    << "\n     A * B;           (returns the identity matrix)"
    << "\n"
    << "\n - Environment Commands:"
    << "\n   - show env;                 --> display current variables/constants"
    << "\n   - save env filename.txt;    --> save environment to file"
    << "\n   - load env filename.txt;    --> load environment from file"
    << "\n"
    << "\n - Precision Settings:"
    << "\n   - precision;                --> show current display precision"
    << "\n   - set precision N;          --> set output precision (0-20 digits)"
    << "\n"
    << "\n Remember: all expressions/statements must end with ';'"
    << "\n Type 'help;' at any time to show this message again."
    << "\n\n";
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
