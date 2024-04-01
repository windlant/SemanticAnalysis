#pragma once
#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string.h>
#include <vector>
#include <set>
#include <map>
#include <afxwin.h>         
#include <afxext.h>          
#include< Afxtempl.h>
#define DEFAULT_DIR "./tmp_code.txt" //临时处理文件路径
#define CODEEND 0 //定义结束符号
using namespace std;

//1953186 龚乙骁 语义分析器

/*
类C文法
%token ::= return | if | else | while | void | int | <ID> | <INT> | ; | , | ( | ) | { | } | + | - | * | / | = | > | < | >= | <= | != | ==
S ::= Program
Program ::= FunRtType FunDec Block
VarType ::= int
FunRtType ::= void | int
FunDec ::= <ID> PreFunSign ( ParameterList )
PreFunSign ::= @
ParameterList ::= ParamDec , ParameterList | ParamDec | @
ParamDec ::= VarType <ID>
Block ::= { DefList StatementList }
DefList ::= Def DefList | @
Def ::= VarType <ID> ;
StatementList ::= Statement StatementList | @
Statement ::= AssignStatement ; | ReturnStatement ; | IfStatement | WhileStatement
AssignStatement ::= <ID> = Exp
Exp ::= ArithmeticExp | Exp Relop ArithmeticExp
ArithmeticExp ::= Item | Item + Item | Item - Item
Item ::= Factor | Factor * Factor | Factor / Factor
Factor ::= <INT> | ( Exp ) | <ID>
ReturnStatement ::= return Exp | return
Relop ::= > | < | >= | <= | == | !=
IfStatement ::= if IfSign1 ( Exp ) IfSign2 Block ElseBlock
IfSign1 ::= @
IfSign2 ::= @
ElseBlock ::= @ | ElseSign else Block
ElseSign ::= @
WhileStatement ::= while WhileSign1 ( Exp ) WhileSign2 Block
WhileSign1 ::= @
WhileSign2 ::= @
*/

//文法部分
//文法中的符号
class Gsymb {
public:
	enum Gsymb_type { Epsilon = 0, Terminal, NonTerminal, End };
	Gsymb_type type;
	set<int> first_set;
	set<int> follow_set;
	string token;
	Gsymb(Gsymb_type type_, const string token_);
};
//产生式
class Gproduction {
public:
	int left_symbol;
	vector<int> right_symbol;

	Gproduction(const int left_symbol_, const vector<int>& right_symbol_);
};
//文法类
class Grammar
{
public:
	const string EpsilonToken = "@";
	const string SplitToken = " | ";
	const string ProToken = "::=";
	const string EndToken = "#";
	const string StartToken = "Program";
	const string ExtendStartToken = "S";
	const string AllTerminalToken = "%token";

	vector<string> grammar_list;
	vector<Gsymb> symbols;
	set<int> terminals;
	set<int> non_terminals;
	vector<Gproduction> productions;
	int start_production;

	Grammar();

	void read_grammar();
	int find_symbol_index_by_token(const string token);
	void getFirstOfNonterminal();
	set<int> getFirstOfString(const vector<int>& str);
};


//预处理部分
//预处理宏定义替换结构体
struct prestruct
{
	CString x;
	CString y;
};
//预处理器
class prehandle
{
public:
	CString prime_txt;
	CString preprocessed_txt;
	long long search_p;
	char ch;
	CString strToken;
	CArray<prestruct, prestruct> PrestructList;
	CString ErorrMessage;
	int word_num;
	int const_num;
	int character_num;
	int currow;
	int max_idlength;
	int max_constlength;
	int error;
	CString word;

	prehandle();
	~prehandle();
	void GetChar(BOOL flag);
	BOOL GetBC(BOOL flag);
	BOOL IsNewRow(BOOL flag);
	void Concat();
	void Retract();
	void PreProcessing();
	void SetText(CString text);
	void ClearStr();
	void CopeError(int id);
	void Reset();
	void Pre_Concat();
};



//词法分析部分
//词法分析结果中的符号
struct Token
{
	string token;//类型
	string value;
	int row;
	int col;
};
//词法分析 器
class LA
{
private:
	vector<Token> Token_Stack;

public:
	int Lerror;
	string default_dir = DEFAULT_DIR;

	LA();
	~LA() {}
	vector<Token> Get_Token_Stack();
};


//语义分析部分
//用于语义分析的符号，可以使规约产生，也可以是词法分析的结果
struct SToken
{
	string type;//语义分析符号种类
	string value;//具体的名字
	int row;//所在行
	int col;//所在列
	int table_index;//所在的符号表索引
	int index;//在符号表中的索引
};

//标志符结构体
struct S_ID
{
	enum IDType { Function, Variable, TempVar, ConstVar, ReturnVar };

	IDType ID_type;
	string ID_value_type;//标志符数值类型
	string ID_value;//标志符的值（变量为名字，常量为数值）
	int function_parameter_num;//函数的参数个数
	int function_table_index;//函数的符号表对应索引

};

//符号表
struct Ssymb_table
{
	//符号表分为全局符号表，每个函数一个符号表，语句块一个符号表
	enum TableType { GlobalTable, FunctionTable, BlockTable };
	TableType table_type;
	vector<S_ID> table;//符号表内容
	string table_name;//符号表名字

	Ssymb_table(const TableType table_type_, const string table_name_);
	int Find_symbol(const string id_name);//在表中寻找某个字符串
	int Add_symbol(const S_ID id);//将某个符号加入表中
};

//四元式
struct Quaternary
{
	int index;//四元式索引号
	string operator_type;//操作类型
	string arg1;//操作数1
	string arg2;//操作数2
	string result;//结果
};

//语义分析器
class SA
{
public:
	int Serror;//错误标志位，发生错误置位
	vector<Quaternary> quaternary;//四元式栈
	//int backpatching_level;
	//改写文法加入if和while的标志，不用建立链即可回填
	vector<int> backpatching_list;//需要回填的四元式索引号栈
	int if_deep;//if语句层数
	int else_num;//else语句块个数
	int while_deep;//while语句层数
	int next_quaternary_index;//当前下一个四元式的索引号
	int tmp_var_count;//临时变量个数
	vector<SToken> Symbol_Stack;//语义栈
	vector<Ssymb_table> tables;//符号表
	vector<int> current_table_stack;//符号表索引栈，用于区分变量作用域
	vector<string> quaternary_list;//四元式字符串形式列表

	SA();
	void Add_to_Symbol_Stack(const SToken symbol);//加入到语义栈
	void Analysis(const string production_left, const vector<string> production_right);//语义动作选择函数，通过规约产生式来判断
	void quaternary_list_generate();//产生四元式字符串列表

	//语义动作
private:
	//S :: = Program
	void Analysis_Program(const string production_left, const vector<string> production_right);
	//VarType ::= int
	void Analysis_VarType(const string production_left, const vector<string> production_right);
	//FunRtType ::= void | int 
	void Analysis_FunRtType(const string production_left, const vector<string> production_right);
	//FunDec :: = <ID> CreateFunTable_m(VarList)
	void Analysis_FunDec(const string production_left, const vector<string> production_right);
	//PreFunSign ::= @
	void Analysis_PreFunSign(const string production_left, const vector<string> production_right);
	//ParamDec ::= VarType <ID>
	void Analysis_ParamDec(const string production_left, const vector<string> production_right);
	//Block ::= { DefList StatementList }
	void Analysis_Block(const string production_left, const vector<string> production_right);
	//Def ::= VarType <ID> ;
	void Analysis_Def(const string production_left, const vector<string> production_right);
	//AssignStatement ::= <ID> = Exp
	void Analysis_AssignStatement(const string production_left, const vector<string> production_right);
	//Exp ::= ArithmeticExp | Exp Relop ArithmeticExp
	void Analysis_Exp(const string production_left, const vector<string> production_right);
	//ArithmeticExp ::= Item | Item + Item | Item - Item
	void Analysis_ArithmeticExp(const string production_left, const vector<string> production_right);
	//Item ::= Factor | Factor * Factor | Factor / Factor
	void Analysis_Item(const string production_left, const vector<string> production_right);
	//Factor ::= <INT> | ( Exp ) | <ID> 
	void Analysis_Factor(const string production_left, const vector<string> production_right);
	//ReturnStatement ::= return Exp | return
	void Analysis_ReturnStatement(const string production_left, const vector<string> production_right);
	//Relop ::= > | < | >= | <= | == | !=
	void Analysis_Relop(const string production_left, const vector<string> production_right);
	//IfStatement ::= if IfSign1 ( Exp ) IfSign2 Block ElseBlock
	void Analysis_IfStatement(const string production_left, const vector<string> production_right);
	//IfSign1 ::= @
	void Analysis_IfSign1(const string production_left, const vector<string> production_right);
	//IfSign2 ::= @
	void Analysis_IfSign2(const string production_left, const vector<string> production_right);
	//ElseBlock ::= @ | ElseSign else Block
	void Analysis_ElseBlock(const string production_left, const vector<string> production_right);
	//ElseSign ::= @
	void Analysis_ElseSign(const string production_left, const vector<string> production_right);
	//WhileStatement ::= while WhileSign1 ( Exp ) WhileSign2 Block
	void Analysis_WhileStatement(const string production_left, const vector<string> production_right);
	//WhileSign1 ::= @
	void Analysis_WhileSign1(const string production_left, const vector<string> production_right);
	//WhileSign2 ::= @
	void Analysis_WhileSign2(const string production_left, const vector<string> production_right);
};



//LR1语法分析器部分
class GA_item {
public:
	int left_symbol;
	vector<int> right_symbol;
	int index;
	int dot_position;
	int lookahead_symbol;
	GA_item(const int left_symbol_, const vector<int>& right_symbol_, const int index_, const int dot_position_, const int lookahead_symbol_);
	bool operator==(const GA_item& item);
};
//求闭包
class GA_closure {
public:
	vector<GA_item> lr1_closure;
	bool operator==(const GA_closure& closure);
};
//动作类型
enum Action {
	ShiftIn,
	Reduce,
	Accept,
	Error
};
//动作结构体
struct ActionInfo {
	Action action;
	int info;
};
//语法分析器
class GA :public Grammar {
public:
	vector<GA_closure> lr1_cluster;
	map<pair<int, int>, int> goto_info;
	map<pair<int, int>, ActionInfo>goto_table;
	map<pair<int, int>, ActionInfo>action_table;
	SA symantic_analysis;

	GA();
	void ClusterGenerate();
	GA_closure& ClosureGenerate(GA_closure& initial_closure);
	GA_closure GotoGenerate(const GA_closure& from_closure, int present_symbol);
	void TableGenerate();
	void Parser(vector<Token> Token_Stack);
};






