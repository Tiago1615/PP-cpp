/*
  simple_calculator_v13.cpp - Advanced expression calculator

  This program implements an extensible expression calculator with support for:
    - Variables and constants
    - Built-in functions (e.g., sin, pow, log)
    - Environment saving/loading
    - Customizable output precision
    - Commands for inspecting environment

  Grammar:

  Statement:
      Help
      Constant Declaration
      Assign
      Expression
      Print
      Quit
      Precision
      Set Precision
      Show Env
      Save Env
      Load Env

  Print:
      ;

  Quit:
      quit

  Help:
      help

  Constant Declaration:
      const Name = Expression

  Assign:
      Name = Expression

  Precision:
      precision

  Set Precision:
      set precision Number

  Show Env:
      show env

  Save Env:
      save env FileName

  Load Env:
      load env FileName

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
      Function
      Number
      Name
      ( Expression )
      - Primary
      + Primary

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
      a string of letters and numbers (e.g., var1, pi, result123)

  FileName:
      a valid filename (e.g., env.txt, my_env-1.dat)
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include <queue>
#include <cmath>
#include <sstream>
#include <map>
#include <fstream>

using namespace std;

#define DEBUG_FUNC false

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
    set_precision_token,
    show_env_token,
    save_env_token,
    load_env_token
  };

  id kind;
  char symbol;
  double value;
  string name;
  using function_t = double(double);
  function_t* function;

  Token() 
  : kind(id::none), symbol(0), value(0), name(), function(nullptr)
  {}

  Token(id tk) 
  : kind(tk), symbol(0), value(0), name(), function(nullptr)
  {}

  Token(char ch) 
  : kind(id::char_token), symbol(ch), value(0), name(), function(nullptr)
  {}

  Token(double val)
  : kind(id::number), symbol(0), value(val), name(), function(nullptr)
  {}

  Token(const string& str) 
  : kind(id::name_token), symbol(0), value(0), name(str), function(nullptr) 
  {}

  Token(const string& str, double (*the_function)(double)) 
  : kind(id::function_token), symbol(0), value(0), name(str), function(the_function) 
  {}

  bool is_symbol(char c) { return ((kind==id::char_token) && (symbol==c)); }
  bool is_number(double v) { return ((kind==id::number) && (value==v)); }
  bool is_name(const string& str) { return ((kind==id::name_token) && (name==str)); }
  bool is_function() { return (kind==id::function_token); }
};

class Token_stream 
{ 
  private:

    queue<Token> buffer; 
  public: 
    Token_stream() { } 
    Token get(); 
    void unget(Token t) { buffer.push(t); } 
    void ignore();
};

string current_filename = "";

string read_word_after_keyword(const string& keyword)
{
  char ch;
  string next;

  while (cin.get(ch) && isspace(ch));

  if (isalpha(ch)) {
    next += ch;
    while (cin.get(ch) && isalpha(ch)) next += ch;
    cin.unget();
  }
  else {
    cin.unget();
  }

  if (next != "env") {
    error("\nExpected 'env' after '" + keyword + "'\n");
  }

  return next;
}

string read_filename(const string& command)
{
  char ch;
  string filename;

  while (cin.get(ch) && isspace(ch));

  if (isalpha(ch)) {
    filename += ch;
    while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_' || ch == '.' || ch == '-')) {
      filename += ch;
    }
    cin.unget();
  } else {
    error("\nExpected filename after '" + command + "'\n");
  }

  if (filename.empty()) {
    error("\nExpected filename after '" + command + "'\n");
  }

  return filename;
}

Token Token_stream::get()
{
  if(!buffer.empty()) 
  { 
    auto t=buffer.front(); 
    buffer.pop(); 
    return t; 
  }

  char ch;
  do { cin.get(ch); } while(isspace(ch));
  switch (ch) 
  {
    case '(':
    case ')':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=': 
    case ',': 
      return Token(ch);

    case ';':
      return Token(Token::id::print);

    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {	
      cin.unget();
      double val;
      cin>>val;
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
        if(s=="precision") return Token(Token::id::precision_token);
        if(s=="set") {
          string next;
          cin >> next;
          if(next == "precision") return Token(Token::id::set_precision_token);
          error("Expected 'precision' after 'set'");
        }
        if (s == "show"){
          read_word_after_keyword("show");
          return Token(Token::id::show_env_token);
        }
        if (s == "save"){
          read_word_after_keyword("save");
          current_filename = read_filename("save");
          return Token(Token::id::save_env_token);
        }
        if (s == "load"){
          read_word_after_keyword("load");
          current_filename = read_filename("load");
          return Token(Token::id::load_env_token);
        }

        if(s=="sin") return Token(s,sin);
        if(s=="cos") return Token(s,cos);
        if(s=="tan") return Token(s,tan);
        if(s=="asin") return Token(s,asin);
        if(s=="acos") return Token(s,acos);
        if(s=="atan") return Token(s,atan);
        if(s=="exp") return Token(s,exp);
        if(s=="pow") return Token(s,nullptr);
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
    auto t=buffer.front(); buffer.pop();
    if(t.kind==Token::id::quit) return;
  }

  char ch;
  while (cin>>ch)
    if (ch==';') return;
}

struct Value 
{
  string name;
  double value;
  bool is_const;

  Value() :name{}, value{0}, is_const{false} {}

  Value(string n, double v, bool is_constant=false) 
    :name(n), value(v), is_const(is_constant) 
  {}
};

map<string,Value> names;
int current_precision = 6;

double get_value(string s)
{
  if(names.count(s)>0) return names[s].value;
  error("get: undefined name ",s);
}

void set_value(string s, double d)
{
  if(names.count(s)>0) 
  {
    if(names[s].is_const) error("set: const name ",s);
    names[s].value=d;
  }
  error("set: undefined name ",s);
}

bool is_constant(string s)
{
  return (
    (names.count(s)>0) && 
    (names[s].is_const)
  ); 
}

bool is_declared(string s) { return (names.count(s)>0); }

void define_name(string s, double d, bool constant=false)
{ names[s]=Value(s,d,constant); }

Token_stream ts;

double expression();

double function_name()
{
  Token t=ts.get();
  if(!t.is_function()) error("function name expected");
  Token tt=ts.get();
  if(!tt.is_symbol('(')) error("'(' expected");
  double d=expression();
  tt=ts.get();
  if(tt.is_symbol(')')) 
  {
    if(t.function) return t.function(d);
    else error(t.name," needs two arguments");
  }
  else if(!tt.is_symbol(',')) error("')' expected");
  {
    double dd=expression();
    tt=ts.get();
    if(tt.is_symbol(')')) 
    {
      if(t.name=="pow") return pow(d,dd); 
      else error(t.name," needs only one argument");
    }
    else error("')' expected");
  }
}

double primary()
{
  Token t = ts.get();
  if(t.is_function()) { ts.unget(t); return function_name(); }
  else if(t.kind==Token::id::char_token)
  {
    if(t.is_symbol('('))
    {
      double d=expression();
      t=ts.get();
      if(!t.is_symbol(')')) error("'(' expected");
      return d;
    }
    else if(t.is_symbol('-')) return -primary();
    else if(t.is_symbol('+')) return primary();
  }
  else if(t.kind==Token::id::number) return t.value;
  else if(t.kind==Token::id::name_token) return get_value(t.name);
  error("primary expected");
}

double term()
{
  double left = primary();
  while(true) 
  {
    Token t = ts.get();
    if(t.is_symbol('*')) left *= primary();
    else if(t.is_symbol('/')) 
    {	
      double d = primary();
      if (d == 0) error("divide by zero");
      left /= d;
    }
    else if(t.is_symbol('%'))
    {	
      double d = primary();
      if (d == 0) error("divide by zero");
      left=fmod(left,d);
    }
    else { ts.unget(t); return left; }
  }
}

double expression()
{
  double left = term();
  while(true) 
  {
    Token t = ts.get();
    if(t.is_symbol('+')) left+=term();
    else if(t.is_symbol('-')) left-=term();
    else { ts.unget(t); return left; }
  }
}

double assign()
{
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error ("name expected in assign");
  string name = t.name;
  if (is_constant(name)) error(name," constant cannot be modified"); 
  t=ts.get();
  if(!t.is_symbol('=')) error("= missing in assign of " ,name);
  double d = expression();
  if(is_declared(name)) 
    set_value(name,d);
  else
    define_name(name,d);
  return d;
}

double constant_assign()
{
  Token t=ts.get();
  if(t.kind!=Token::id::name_token) error("name expected in const assign");
  string name = t.name;
  if(is_declared(name)) error(name," has already been defined"); 
  t=ts.get();
  if(!t.is_symbol('=')) error("= missing in assign of " ,name);
  double d = expression();
  define_name(name,d,true);
  return d;
}

void set_precision()
{
  Token t = ts.get();
  if (t.kind != Token::id::number)
    error("Expected a number after 'set precision'");
  int digits = static_cast<int>(t.value);
  if (digits < 0 || digits > 20)
    error("Precision must be between 0 and 20");
  current_precision = digits;
  cout << "Precision set to " << digits << " digits." << endl;
}

void show_precision()
{
  cout << "Current precision: " << current_precision << " digits." << endl;
}

void set_precision(int digits)
{
  if (digits < 0 || digits > 20)
    error("Precision must be between 0 and 20");
  current_precision = digits;
  cout.setf(ios::fixed);
  cout.precision(current_precision);
}

void show_env()
{
  if (names.empty()) {
    error("\nshow env: (none)\n");
  }

  cout << "\nCurrent environment:" << endl << endl;
  for (const auto& [key, val] : names) {
    cout << "  " << key << " = " << val.value;
    if (val.is_const) cout << " (const)";
    cout << endl << endl;
  }
}

void save_env()
{
  if (names.empty()) {
    error("\nsave env: No variables or constants to save.\n");
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

  ofstream out(current_filename);
  if (!out) {
    error("\nsave env: Could not open file for writing\n");
  }

  out.setf(ios::fixed);
  out.precision(save_precision);

  out << "Precision = " << save_precision << endl;

  for (const auto& [key, val] : names) {
    out << key << " = " << val.value << " is_const = " << val.is_const << endl;
  }

  out.close();
  cout << "\nEnvironment saved to " << current_filename << " with precision of " << save_precision << " digits.\n\n";
}

void load_env()
{
  ifstream in(current_filename);
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
          cout << "\nPrecision set to " << current_precision << " digits.\n";
          loop = false;
          break;
        case 2:
          cout << "\nKeeping current precision of " << current_precision << " digits.\n";
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
    string name, eq, is_const_str;
    double value;
    int is_const;

    stream >> name >> eq >> value >> is_const_str >> eq >> is_const;

    if (name.empty()) continue;

    if (!is_declared(name)){
      define_name(name, value, is_const);
      cout << "\nLoaded variable : " << name << " = " << value;
      if (is_const) cout << " (const)\n";
      else cout << "\n";
    }
    else{
      cout << "\nConflict detected for variable: " << name << "." << endl;
      cout << "\nExisting value: " << get_value(name) << "\n(const: " << (is_constant(name) ? "yes" : "no") << ")\n";
      cout << "\nFile value: " << value << "\n(const: " << (is_const ? "yes" : "no") << ")\n";

      cout << "\nChoose an action:\n";
      cout << "  \n1. Keep existing value\n";
      cout << "  \n2. Overwrite with file value\n";
      cout << "  \n3. Keep both (rename file value)\n";
      cout << "\n\nSelect option (1-3): ";

      int option;
      bool loop = true;

      while (loop){
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cin >> option;

        switch (option){
          case 1:
            cout << "\nKeeping existing value for '" << name << "'.\n";
            loop = false;
            break;
          case 2:
            names[name] = Value(name, value, is_const);
            cout << "\nOverwritten '" << name << "' with value from file.\n";
            loop = false;
            break;
          case 3:
            {
              string new_name;
              int rename_attempts = 0;
              do {
                new_name = name + "_file";
                if (rename_attempts > 0) {
                  new_name += to_string(rename_attempts);
                }
                rename_attempts++;
              } while (is_declared(new_name));

              define_name(new_name, value, is_const);
              cout << "\nRenamed file variable to '" << new_name << "'.\n";
              loop = false;
            }
            break;
          default:
            cout << "\nInvalid option. Please select 1, 2 or 3: ";
            break;
        }
      }
    }
  }

  in.close();
  cout << "\nEnvironment loaded from " << current_filename << ".\n\n";
}

double statement()
{
  Token t=ts.get();
  switch(t.kind)
  {
    case Token::id::const_token:
      return constant_assign();
    case Token::id::name_token:
      {
        Token tt=ts.get();
        if(tt.is_symbol('=')) { ts.unget(t); ts.unget(tt); return assign(); }
        else { ts.unget(t); ts.unget(tt); return expression(); }
      }
    default:
      { ts.unget(t); return expression(); }
  }
}

void clean_up_mess()
{
	ts.ignore();
}

void help()
{
  cout
    << "\n =============================================================="
    << "\n  This is a simple calculator for arithmetic expressions"
    << "\n  supporting variables, constants, and mathematical functions."
    << "\n =============================================================="
    << "\n"
    << "\n - Basic Usage:"
    << "\n   - Use ';' to end each statement"
    << "\n   - Type 'quit' to exit the program"
    << "\n   - Example: a = 5 + 3;"
    << "\n"
    << "\n - Mathematical Functions Supported:"
    << "\n   - Trigonometric: sin(x), cos(x), tan(x)"
    << "\n   - Inverse trig:  asin(x), acos(x), atan(x)"
    << "\n   - Exponential :  exp(x), pow(x, y)"
    << "\n   - Logarithmic :  ln(x), log10(x), log2(x)"
    << "\n"
    << "\n - Variables and Constants:"
    << "\n   - Assign a variable:     x = 42;"
    << "\n   - Define a constant:     const pi = 3.1416;"
    << "\n"
    << "\n - Environment Commands:"
    << "\n   - show env;              --> display current variables/constants"
    << "\n   - save env filename;     --> save environment to file"
    << "\n   - load env filename;     --> load environment from file"
    << "\n"
    << "\n - Precision Settings:"
    << "\n   - precision;             --> show current display precision"
    << "\n   - set precision N;       --> set output precision (0-20 digits)"
    << "\n"
    << "\n Type 'help;' at any time to show this message again."
    << "\n\n";
}

const string prompt = "> ";
const string result = "= ";

void calculate()
{
  while(true) 
  try 
  {
    cout<<prompt;
    Token t=ts.get();
    while (t.kind==Token::id::print) t=ts.get();
    if(t.kind==Token::id::quit) return;
    if(t.kind==Token::id::help_token) { help(); continue; }
    if (t.kind==Token::id::set_precision_token) { set_precision(); continue; }
    if (t.kind==Token::id::precision_token) { show_precision(); continue; }
    if (t.kind==Token::id::show_env_token) { show_env(); continue; }
    if (t.kind==Token::id::save_env_token) { save_env(); continue; }
    if (t.kind==Token::id::load_env_token) { load_env(); continue; }
    ts.unget(t);
    auto the_result=statement();
    cout.setf(ios::fixed);
    cout.precision(current_precision);
    cout<<result<<the_result<<endl;
  }
  catch(runtime_error& e) 
  {
    cerr<<e.what()<< endl;
    clean_up_mess();
  }
}

int main()
try 
{
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
