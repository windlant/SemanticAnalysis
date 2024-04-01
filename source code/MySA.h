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
#define DEFAULT_DIR "./tmp_code.txt" //��ʱ�����ļ�·��
#define CODEEND 0 //�����������
using namespace std;

//1953186 ������ ���������

/*
��C�ķ�
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

//�ķ�����
//�ķ��еķ���
class Gsymb {
public:
	enum Gsymb_type { Epsilon = 0, Terminal, NonTerminal, End };
	Gsymb_type type;
	set<int> first_set;
	set<int> follow_set;
	string token;
	Gsymb(Gsymb_type type_, const string token_);
};
//����ʽ
class Gproduction {
public:
	int left_symbol;
	vector<int> right_symbol;

	Gproduction(const int left_symbol_, const vector<int>& right_symbol_);
};
//�ķ���
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


//Ԥ������
//Ԥ����궨���滻�ṹ��
struct prestruct
{
	CString x;
	CString y;
};
//Ԥ������
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



//�ʷ���������
//�ʷ���������еķ���
struct Token
{
	string token;//����
	string value;
	int row;
	int col;
};
//�ʷ����� ��
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


//�����������
//������������ķ��ţ�����ʹ��Լ������Ҳ�����Ǵʷ������Ľ��
struct SToken
{
	string type;//���������������
	string value;//���������
	int row;//������
	int col;//������
	int table_index;//���ڵķ��ű�����
	int index;//�ڷ��ű��е�����
};

//��־���ṹ��
struct S_ID
{
	enum IDType { Function, Variable, TempVar, ConstVar, ReturnVar };

	IDType ID_type;
	string ID_value_type;//��־����ֵ����
	string ID_value;//��־����ֵ������Ϊ���֣�����Ϊ��ֵ��
	int function_parameter_num;//�����Ĳ�������
	int function_table_index;//�����ķ��ű��Ӧ����

};

//���ű�
struct Ssymb_table
{
	//���ű��Ϊȫ�ַ��ű�ÿ������һ�����ű�����һ�����ű�
	enum TableType { GlobalTable, FunctionTable, BlockTable };
	TableType table_type;
	vector<S_ID> table;//���ű�����
	string table_name;//���ű�����

	Ssymb_table(const TableType table_type_, const string table_name_);
	int Find_symbol(const string id_name);//�ڱ���Ѱ��ĳ���ַ���
	int Add_symbol(const S_ID id);//��ĳ�����ż������
};

//��Ԫʽ
struct Quaternary
{
	int index;//��Ԫʽ������
	string operator_type;//��������
	string arg1;//������1
	string arg2;//������2
	string result;//���
};

//���������
class SA
{
public:
	int Serror;//�����־λ������������λ
	vector<Quaternary> quaternary;//��Ԫʽջ
	//int backpatching_level;
	//��д�ķ�����if��while�ı�־�����ý��������ɻ���
	vector<int> backpatching_list;//��Ҫ�������Ԫʽ������ջ
	int if_deep;//if������
	int else_num;//else�������
	int while_deep;//while������
	int next_quaternary_index;//��ǰ��һ����Ԫʽ��������
	int tmp_var_count;//��ʱ��������
	vector<SToken> Symbol_Stack;//����ջ
	vector<Ssymb_table> tables;//���ű�
	vector<int> current_table_stack;//���ű�����ջ���������ֱ���������
	vector<string> quaternary_list;//��Ԫʽ�ַ�����ʽ�б�

	SA();
	void Add_to_Symbol_Stack(const SToken symbol);//���뵽����ջ
	void Analysis(const string production_left, const vector<string> production_right);//���嶯��ѡ������ͨ����Լ����ʽ���ж�
	void quaternary_list_generate();//������Ԫʽ�ַ����б�

	//���嶯��
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



//LR1�﷨����������
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
//��հ�
class GA_closure {
public:
	vector<GA_item> lr1_closure;
	bool operator==(const GA_closure& closure);
};
//��������
enum Action {
	ShiftIn,
	Reduce,
	Accept,
	Error
};
//�����ṹ��
struct ActionInfo {
	Action action;
	int info;
};
//�﷨������
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






