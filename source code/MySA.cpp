#include "pch.h"
#include "MySA.h"

//语义分析部分
//符号表构造函数
Ssymb_table::Ssymb_table(const TableType table_type_, const string table_name_)
{
	table_type = table_type_;
	table_name = table_name_;
}

//在符号表中寻找一个符号，并返回索引
int Ssymb_table::Find_symbol(const string id_name)
{
	for (int i = 0; i < table.size(); i++) {
		if (table[i].ID_value == id_name)
			return i;
	}
	return -1;
}

//将一个符号添加到符号表
int Ssymb_table::Add_symbol(const S_ID id)
{
	if (Find_symbol(id.ID_value) == -1) {
		table.push_back(id);
		return table.size() - 1;
	}
	return -1;
}

//语义分析器构造函数
SA::SA()
{
	Serror = 0;
	//初始添加全局符号表
	tables.push_back(Ssymb_table(Ssymb_table::GlobalTable, "global table"));
	//当前符号表设置为全局符号表
	current_table_stack.push_back(0);
	//四元式索引从0开始
	next_quaternary_index = 0;
	//初始临时变量个数为0
	tmp_var_count = 0;
	if_deep = 0;//if语句层数
	else_num = 0;//else语句块个数
	while_deep = 0;//while语句层数
}

//将语义符号添加到语义栈中，语义符号可以是规约的产生式左部，也可以是词法分析的结果
void SA::Add_to_Symbol_Stack(const SToken symbol)
{
	Symbol_Stack.push_back(symbol);
}

//语法分析中加入语义分析
void GA::Parser(vector<Token> Token_Stack)
{
	symantic_analysis = SA();
	Token_Stack.push_back({ EndToken,EndToken,-1,-1 });
	vector<int> symbol_stack;
	vector<int> status_stack;
	int step = 1;

	// 初始化
	symantic_analysis.Add_to_Symbol_Stack({ StartToken,"",-1,-1,-1,-1 });

	symbol_stack.push_back(find_symbol_index_by_token(EndToken));
	status_stack.push_back(0);


	//分析符号的合法性
	for (int i = 0; i < Token_Stack.size(); i++) {
		int current_state = status_stack.back();
		int current_token_index = find_symbol_index_by_token(Token_Stack[i].token);

		if (current_token_index == -1) {
			CString em;
			em.Format("词法分析错误，有无法识别符号！");
			AfxMessageBox(em);
			return;
		}

		//根据表格查找相应的动作
		auto current_action_iter = action_table.find({ current_state ,current_token_index });

		if (current_action_iter == action_table.end()) {
			CString em;
			em.Format("语法分析错误！错误位置%d行", Token_Stack[i].row);
			AfxMessageBox(em);
			return;
		}

		ActionInfo current_actioninfo = current_action_iter->second;

		//分析动作
		switch (current_actioninfo.action) {
		case Action::ShiftIn: { //移入
			symbol_stack.push_back(current_token_index);
			status_stack.push_back(current_actioninfo.info);

			//同时生成语义符号并移入语义栈
			symantic_analysis.Add_to_Symbol_Stack({ Token_Stack[i].token , Token_Stack[i].value , Token_Stack[i].row , Token_Stack[i].col ,-1,-1 });
			break;
		}

		//规约
		case Action::Reduce: {

			int production_index = current_actioninfo.info;
			Gproduction production = productions[production_index];

			if (symbols[production.right_symbol.front()].type != Gsymb::Epsilon) {
				int count = production.right_symbol.size();
				while (count--) {
					symbol_stack.pop_back();
					status_stack.pop_back();
				}
			}
			auto current_goto_iter = goto_table.find({ status_stack.back(),production.left_symbol });

			if (current_goto_iter == goto_table.end()) {
				CString em;
				em.Format("语法分析错误！错误位置%d行", Token_Stack[i].row);
				AfxMessageBox(em);
				return;
			}
			symbol_stack.push_back(production.left_symbol);
			status_stack.push_back(current_goto_iter->second.info);

			i--;
			vector<string> production_right;
			for (auto s : production.right_symbol)
				production_right.push_back(symbols[s].token);

			//根据规约的产生式来进行对应的语义动作
			symantic_analysis.Analysis(symbols[production.left_symbol].token, production_right);
			if (symantic_analysis.Serror)
				return;
		}
						   break;

		case Action::Accept:
			return;
			break;

		case Action::Error:
			return;
			break;
		}
	}
}

//语义动作选择函数，根据规约产生式来选择
//各个语义动作详说明见设计说明书
void SA::Analysis(const string production_left, const vector<string> production_right)
{
	if (production_left == "Program")
		Analysis_Program(production_left, production_right);
	else if (production_left == "VarType")
		Analysis_VarType(production_left, production_right);
	else if (production_left == "FunRtType")
		Analysis_FunRtType(production_left, production_right);
	else if (production_left == "FunDec")
		Analysis_FunDec(production_left, production_right);
	else if (production_left == "PreFunSign")
		Analysis_PreFunSign(production_left, production_right);
	else if (production_left == "ParamDec")
		Analysis_ParamDec(production_left, production_right);
	else if (production_left == "Block")
		Analysis_Block(production_left, production_right);
	else if (production_left == "Def")
		Analysis_Def(production_left, production_right);
	else if (production_left == "AssignStatement")
		Analysis_AssignStatement(production_left, production_right);
	else if (production_left == "Exp")
		Analysis_Exp(production_left, production_right);
	else if (production_left == "ArithmeticExp")
		Analysis_ArithmeticExp(production_left, production_right);
	else if (production_left == "Item")
		Analysis_Item(production_left, production_right);
	else if (production_left == "Factor")
		Analysis_Factor(production_left, production_right);
	else if (production_left == "ReturnStatement")
		Analysis_ReturnStatement(production_left, production_right);
	else if (production_left == "Relop")
		Analysis_Relop(production_left, production_right);
	else if (production_left == "IfStatement")
		Analysis_IfStatement(production_left, production_right);
	else if (production_left == "IfSign1")
		Analysis_IfSign1(production_left, production_right);
	else if (production_left == "IfSign2")
		Analysis_IfSign2(production_left, production_right);
	else if (production_left == "ElseBlock")
		Analysis_ElseBlock(production_left, production_right);
	else if (production_left == "ElseSign")
		Analysis_ElseSign(production_left, production_right);
	else if (production_left == "WhileStatement")
		Analysis_WhileStatement(production_left, production_right);
	else if (production_left == "WhileSign1")
		Analysis_WhileSign1(production_left, production_right);
	else if (production_left == "WhileSign2")
		Analysis_WhileSign2(production_left, production_right);
	else {
		//如果不是特殊的产生式就直接对语义栈进行规约
		if (production_right[0] != "@") {
			int count = production_right.size();
			while (count--)
				Symbol_Stack.pop_back();
		}
		Symbol_Stack.push_back({ production_left,"",-1,-1,-1,-1 });
	}

}

//生成四元式字符串列表，用于图形化
void SA::quaternary_list_generate()
{
	for (int i = 0; i < quaternary.size(); i++)
	{
		Quaternary q = quaternary[i];
		string tmp = "(" + q.operator_type + "," + q.arg1 + "," + q.arg2 + "," + q.result + ")";
		quaternary_list.push_back(tmp);
	}
}

//Program ::= FunRtType FunDec Block 
void SA::Analysis_Program(const string production_left, const vector<string> production_right)
{
	SToken ID = Symbol_Stack[Symbol_Stack.size() - 2];
	current_table_stack.pop_back();
	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,ID.value,ID.row,ID.col,ID.table_index,ID.index });
}

//VarType ::= int
void SA::Analysis_VarType(const string production_left, const vector<string> production_right)
{
	SToken varname = Symbol_Stack.back();
	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,varname.value,varname.row,varname.col,-1,-1 });
}

//FunRtType ::= void | int 
void SA::Analysis_FunRtType(const string production_left, const vector<string> production_right)
{
	SToken varname = Symbol_Stack.back();
	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,varname.value,varname.row,varname.col,-1,-1 });
}

//FunDec ::= <ID> PreFunSign ( ParameterList )
void SA::Analysis_FunDec(const string production_left, const vector<string> production_right)
{
	SToken varname = Symbol_Stack[Symbol_Stack.size() - 4];
	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,varname.value,varname.row,varname.col,varname.table_index,varname.index });
}

//PreFunSign ::= @
void SA::Analysis_PreFunSign(const string production_left, const vector<string> production_right)
{
	SToken ID = Symbol_Stack.back();
	SToken varname = Symbol_Stack[Symbol_Stack.size() - 2];

	tables.push_back(Ssymb_table(Ssymb_table::FunctionTable, ID.value));

	tables[0].Add_symbol({ S_ID::Function,varname.value,ID.value,0,int(tables.size() - 1) });

	current_table_stack.push_back(tables.size() - 1);
	S_ID return_value;
	return_value.ID_type = S_ID::ReturnVar;
	return_value.ID_value =  "return_value";
	return_value.ID_value_type = varname.value;

	quaternary.push_back({ next_quaternary_index++ , ID.value,"-","-" ,"-" });
	tables[current_table_stack.back()].Add_symbol(return_value);

	Symbol_Stack.push_back({ production_left,ID.value,ID.row,ID.col,0,int(tables[0].table.size() - 1) });
}

//ParamDec ::= VarType <ID>
void SA::Analysis_ParamDec(const string production_left, const vector<string> production_right)
{
	SToken ID = Symbol_Stack.back();
	SToken varname = Symbol_Stack[Symbol_Stack.size() - 2];

	Ssymb_table& function_table = tables[current_table_stack.back()];

	if (function_table.Find_symbol(ID.value) != -1) {
		CString em;
		em.Format("语义分析错误，参数重定义！错误位置%d行", ID.row);
		AfxMessageBox(em);
		Serror = 1;
		return;
	}
	int new_position = function_table.Add_symbol({ S_ID::Variable,varname.value,ID.value,-1,-1 });
	int table_position = tables[0].Find_symbol(function_table.table_name);
	tables[0].table[table_position].function_parameter_num++;

	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,ID.value,ID.row,ID.col,current_table_stack.back(),new_position });
}

//Block ::= { DefList StatementList }
void SA::Analysis_Block(const string production_left, const vector<string> production_right)
{
	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,to_string(next_quaternary_index),-1,-1,-1,-1 });
}

//Def ::= VarType <ID> ;
void SA::Analysis_Def(const string production_left, const vector<string> production_right)
{
	SToken ID = Symbol_Stack[Symbol_Stack.size() - 2];
	SToken varname = Symbol_Stack[Symbol_Stack.size() - 3];
	Ssymb_table& current_table = tables[current_table_stack.back()];

	if (current_table.Find_symbol(ID.value) != -1)
	{
		CString em;
		em.Format("语义分析错误，内部声明重定义！错误位置%d行", ID.row);
		AfxMessageBox(em);
		Serror = 1;
		return;
	}

	current_table.Add_symbol({ S_ID::Variable,varname.value,ID.value ,-1,-1 });

	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left, ID.value ,ID.row,ID.col,current_table_stack.back(),int(tables[current_table_stack.back()].table.size() - 1) });
}

//AssignStatement ::= <ID> = Exp
void SA::Analysis_AssignStatement(const string production_left, const vector<string> production_right)
{
	SToken ID = Symbol_Stack[Symbol_Stack.size() - 3];
	SToken exp = Symbol_Stack.back();

	bool existed = false;
	int table_index = -1, index = -1;

	for (int scope_layer = current_table_stack.size() - 1; scope_layer >= 0; scope_layer--) {
		auto current_table = tables[current_table_stack[scope_layer]];
		if ((index = current_table.Find_symbol(ID.value)) != -1) {
			existed = true;
			table_index = current_table_stack[scope_layer];
			break;
		}
	}
	if (existed == false) {
		CString em;
		em.Format("语义分析错误，变量未定义！错误位置%d行", ID.row);
		AfxMessageBox(em);
		Serror = 1;
		return;
	}

	quaternary.push_back({ next_quaternary_index++, "=", exp.value, "-", ID.value });

	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left, ID.value ,ID.row,ID.col,table_index,index });
}

//Exp ::= ArithmeticExp | Exp Relop ArithmeticExp
void SA::Analysis_Exp(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SToken exp = Symbol_Stack.back();

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, exp.value ,exp.row,exp.col,exp.table_index,exp.index });
	}
	else {
		SToken sub_exp1 = Symbol_Stack[Symbol_Stack.size() - 3];
		SToken op = Symbol_Stack[Symbol_Stack.size() - 2];
		SToken sub_exp2 = Symbol_Stack[Symbol_Stack.size() - 1];
		int next_label_num = next_quaternary_index++;
		string new_tmp_var = "T" + to_string(tmp_var_count++);
		quaternary.push_back({ next_label_num, "j" + op.value, sub_exp1.value, sub_exp2.value, to_string(next_label_num + 3) });
		quaternary.push_back({ next_quaternary_index++, "=", "0", "-", new_tmp_var });
		quaternary.push_back({ next_quaternary_index++, "j", "-", "-", to_string(next_label_num + 4) });
		quaternary.push_back({ next_quaternary_index++, "=", "1", "-", new_tmp_var });

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, new_tmp_var ,-1,-1,-1,-1 });
	}
}

//ArithmeticExp :: = Item | Item + Item | Item – Item
void SA::Analysis_ArithmeticExp(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SToken exp = Symbol_Stack.back();

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, exp.value ,exp.row,exp.col,exp.table_index,exp.index });
	}
	else {
		SToken sub_exp1 = Symbol_Stack[Symbol_Stack.size() - 3];
		SToken op = Symbol_Stack[Symbol_Stack.size() - 2];
		SToken sub_exp2 = Symbol_Stack[Symbol_Stack.size() - 1];
		string new_tmp_var = "T" + to_string(tmp_var_count++);
		quaternary.push_back({ next_quaternary_index++, op.value, sub_exp1.value, sub_exp2.value, new_tmp_var });

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, new_tmp_var ,-1,-1,-1,-1 });
	}
}

//Item :: = Factor | Factor * Factor | Factor / Factor
void SA::Analysis_Item(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SToken exp = Symbol_Stack.back();

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, exp.value ,exp.row,exp.col,exp.table_index,exp.index });
	}
	else {
		SToken sub_exp1 = Symbol_Stack[Symbol_Stack.size() - 3];
		SToken op = Symbol_Stack[Symbol_Stack.size() - 2];
		SToken sub_exp2 = Symbol_Stack[Symbol_Stack.size() - 1];
		std::string new_tmp_var = "T" + to_string(tmp_var_count++);
		quaternary.push_back({ next_quaternary_index++, op.value, sub_exp1.value, sub_exp2.value, new_tmp_var });

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, new_tmp_var ,-1,-1,-1,-1 });
	}
}

//Factor ::= <INT> | ( Exp ) | <ID> 
void SA::Analysis_Factor(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SToken exp = Symbol_Stack.back();

		if (production_right[0] == "<ID>") {
			bool existed = false;
			for (int scope_layer = current_table_stack.size() - 1; scope_layer >= 0; scope_layer--) {
				auto current_table = tables[current_table_stack[scope_layer]];
				if (current_table.Find_symbol(exp.value) != -1) {
					existed = true;
					break;
				}
			}
			if (existed == false) {
				CString em;
				em.Format("语义分析错误，变量未定义！错误位置%d行", exp.row);
				AfxMessageBox(em);
				Serror = 1;
				return;
			}
		}

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, exp.value ,exp.row,exp.col,exp.table_index,exp.index });
	}
	else {
		SToken exp = Symbol_Stack[Symbol_Stack.size() - 2];

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, exp.value ,exp.row,exp.col,exp.table_index,exp.index });
	}
}

//ReturnStatement ::= return Exp | return
void SA::Analysis_ReturnStatement(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 2) {
		SToken return_exp = Symbol_Stack.back();
		Ssymb_table function_table = tables[current_table_stack.back()];

		quaternary.push_back({ next_quaternary_index++,"=",return_exp.value,"-",function_table.table[0].ID_value });

		quaternary.push_back({ next_quaternary_index++ ,"return",function_table.table[0].ID_value,"-",function_table.table_name });

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, return_exp.value,-1,-1,-1,-1 });
	}
	else {
		Ssymb_table function_table = tables[current_table_stack.back()];

		if (tables[0].table[tables[0].Find_symbol(function_table.table_name)].ID_value_type != "void") {
			CString em;
			em.Format("语义分析错误，函数缺少返回值！错误位置%d行", Symbol_Stack.back().row);
			AfxMessageBox(em);
			Serror = 1;
			return;
		}

		quaternary.push_back({ next_quaternary_index++ ,"return","-","-",function_table.table_name });

		int count = production_right.size();
		while (count--)
			Symbol_Stack.pop_back();
		Symbol_Stack.push_back({ production_left, "",-1,-1,-1,-1 });
	}
}

//Relop ::= > | < | >= | <= | == | !=
void SA::Analysis_Relop(const string production_left, const vector<string> production_right)
{
	SToken op = Symbol_Stack.back();

	int count = production_right.size();
	while (count--) {
		Symbol_Stack.pop_back();
	}
	Symbol_Stack.push_back({ production_left, op.value ,-1,-1,-1,-1 });
}

/*
if语句和while语句回填原理
在一些sign规约的时候，记录下当时需要的四元式位置，存放在回填栈中，
并且记录真假出口以及跳转出口位置，之后对整个语句进行归于的时候通过回填栈进行回填
*/

//IfStatement ::= if IfSign1 ( Exp ) IfSign2 Block ElseBlock
void SA::Analysis_IfStatement(const string production_left, const vector<string> production_right)
{
	SToken ifsign2 = Symbol_Stack[Symbol_Stack.size() - 3];
	SToken elseblock = Symbol_Stack[Symbol_Stack.size() - 1];

	if (elseblock.value[0] < 48 || elseblock.value[0] > 57) {
		quaternary[backpatching_list.back()].result = ifsign2.value;
		backpatching_list.pop_back();

		quaternary[backpatching_list.back()].result = to_string(next_quaternary_index);
		backpatching_list.pop_back();
	}
	else {
		quaternary[backpatching_list.back()].result = to_string(next_quaternary_index);
		backpatching_list.pop_back();

		quaternary[backpatching_list.back()].result = ifsign2.value;
		backpatching_list.pop_back();

		quaternary[backpatching_list.back()].result = elseblock.value;
		backpatching_list.pop_back();
	}

	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,"",-1,-1,-1,-1 });
	current_table_stack.pop_back();
}

//IfSign1 ::= @
void SA::Analysis_IfSign1(const string production_left, const vector<string> production_right)
{
	if_deep++;
	Symbol_Stack.push_back({ production_left,to_string(next_quaternary_index),-1,-1,-1,-1 });
}

//IfSign2 ::= @
void SA::Analysis_IfSign2(const string production_left, const vector<string> production_right)
{
	SToken if_exp = Symbol_Stack[Symbol_Stack.size() - 2];

	tables.push_back(Ssymb_table(Ssymb_table::BlockTable, "if"+to_string(if_deep)));
	current_table_stack.push_back(tables.size() - 1);

	quaternary.push_back({ next_quaternary_index++,"j=",if_exp.value,"0","" });
	backpatching_list.push_back(quaternary.size() - 1);

	quaternary.push_back({ next_quaternary_index++,"j=","-","-","" });
	backpatching_list.push_back(quaternary.size() - 1);
	Symbol_Stack.push_back({ production_left,to_string(next_quaternary_index),-1,-1,-1,-1 });
}

//ElseBlock ::= @ | ElseSign else Block
void SA::Analysis_ElseBlock(const string production_left, const vector<string> production_right)
{
	SToken else_sign;
	if (production_right[0] == "@")
	{
		quaternary.push_back({ next_quaternary_index++,"j","-","-","" });
		backpatching_list.push_back(quaternary.size() - 1);
		Symbol_Stack.push_back({ production_left,to_string(next_quaternary_index),-1,-1,-1,-1 });
		return;
	}
	else
	{
		else_sign = Symbol_Stack[Symbol_Stack.size() - 3];
	}

	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();

	Symbol_Stack.push_back({ production_left,else_sign.value,-1,-1,-1,-1 });
	else_num--;
	current_table_stack.pop_back();
}

//ElseSign ::= @
void SA::Analysis_ElseSign(const string production_left, const vector<string> production_right)
{
	else_num++;
	tables.push_back(Ssymb_table(Ssymb_table::BlockTable, "else" + to_string(else_num)));
	current_table_stack.push_back(tables.size() - 1);

	quaternary.push_back({ next_quaternary_index++,"j","-","-","" });
	backpatching_list.push_back(quaternary.size() - 1);
	Symbol_Stack.push_back({ production_left,to_string(next_quaternary_index),-1,-1,-1,-1 });
}

//WhileStatement ::= while WhileSign1 ( Exp ) WhileSign2 Block
void SA::Analysis_WhileStatement(const string production_left, const vector<string> production_right)
{
	SToken while_sign1 = Symbol_Stack[Symbol_Stack.size() - 6];
	SToken while_sign2 = Symbol_Stack[Symbol_Stack.size() - 2];

	quaternary.push_back({ next_quaternary_index++,"j","-","-" ,while_sign1.value });

	quaternary[backpatching_list.back()].result = while_sign2.value;
	backpatching_list.pop_back();

	quaternary[backpatching_list.back()].result = to_string(next_quaternary_index);
	backpatching_list.pop_back();

	int count = production_right.size();
	while (count--)
		Symbol_Stack.pop_back();
	Symbol_Stack.push_back({ production_left,"",-1,-1,-1,-1 });
	current_table_stack.pop_back();
}

//WhileSign1 ::= @
void SA::Analysis_WhileSign1(const string production_left, const vector<string> production_right)
{
	while_deep++;
	Symbol_Stack.push_back({ production_left,to_string(next_quaternary_index),-1,-1,-1,-1 });
}

//WhileSign2 ::= @
void SA::Analysis_WhileSign2(const string production_left, const vector<string> production_right)
{
	tables.push_back(Ssymb_table(Ssymb_table::BlockTable, "while" + to_string(while_deep)));
	current_table_stack.push_back(tables.size() - 1);
	SToken while_exp = Symbol_Stack[Symbol_Stack.size() - 2];
	quaternary.push_back({ next_quaternary_index++,"j=",while_exp.value,"0","" });
	backpatching_list.push_back(quaternary.size() - 1);
	quaternary.push_back({ next_quaternary_index++ ,"j","-","-" ,"" });
	backpatching_list.push_back(quaternary.size() - 1);
	Symbol_Stack.push_back({ production_left,to_string(next_quaternary_index),-1,-1,-1,-1 });
}




//文法部分
string& trim(string& str)
{
	if (str.empty())
		return str;
	str.erase(0, str.find_first_not_of("\f\v\r\t\n "));
	str.erase(str.find_last_not_of("\f\v\r\t\n ") + 1);
	return str;
}

vector<string> split(const string str, const string separator)
{
	vector<string> strs;
	int start = 0;
	int end = str.find(separator, start);

	if (end == string::npos) {
		strs.push_back(str);
		return strs;
	}
	while (end != string::npos) {

		string sub = str.substr(start, end - start);

		trim(sub);

		if (!sub.empty())
			strs.push_back(sub);

		start = end + separator.length();
		end = str.find(separator, start);
	}
	string sub = str.substr(start);
	trim(sub);
	if (!sub.empty())
		strs.push_back(sub);

	return strs;
}

bool mergeSetExceptEmpty(set<int>& des, set<int>& src, int epsilon_index)
{
	set<int> tmp;
	set_difference(src.begin(), src.end(), des.begin(), des.end(), inserter(tmp, tmp.begin()));
	bool desExisted = des.find(epsilon_index) != des.end();
	int beforeInsert = des.size();
	des.insert(tmp.begin(), tmp.end());
	if (!desExisted)
	{
		des.erase(epsilon_index);
	}
	return beforeInsert < des.size();
}

Gsymb::Gsymb(Gsymb::Gsymb_type type_, const string token_)
{
	this->type = type_;
	this->token = token_;
}

Gproduction::Gproduction(const int left_symbol_, const vector<int>& right_symbol_)
{
	this->left_symbol = left_symbol_;
	this->right_symbol = right_symbol_;
}

Grammar::Grammar()
{
	grammar_list.push_back("%token ::= return | if | else | while | void | int | <ID> | <INT> | ; | , | ( | ) | { | } | + | - | * | / | = | > | < | >= | <= | != | ==");
	grammar_list.push_back("S ::= Program");
	grammar_list.push_back("Program ::= FunRtType FunDec Block ");
	grammar_list.push_back("VarType ::= int");
	grammar_list.push_back("FunRtType ::= void | int ");
	grammar_list.push_back("FunDec ::= <ID> PreFunSign ( ParameterList )");
	grammar_list.push_back("PreFunSign ::= @");
	grammar_list.push_back("ParameterList ::= ParamDec , ParameterList | ParamDec | @");
	grammar_list.push_back("ParamDec ::= VarType <ID>");
	grammar_list.push_back("Block ::= { DefList StatementList }");
	grammar_list.push_back("DefList ::= Def DefList | @");
	grammar_list.push_back("Def ::= VarType <ID> ;");
	grammar_list.push_back("StatementList ::= Statement StatementList | @");
	grammar_list.push_back("Statement ::= AssignStatement ; | ReturnStatement ; | IfStatement | WhileStatement");
	grammar_list.push_back("AssignStatement ::= <ID> = Exp");
	grammar_list.push_back("Exp ::= ArithmeticExp | Exp Relop ArithmeticExp");
	grammar_list.push_back("ArithmeticExp ::= Item | Item + Item | Item - Item");
	grammar_list.push_back("Item ::= Factor | Factor * Factor | Factor / Factor");
	grammar_list.push_back("Factor ::= <INT> | ( Exp ) | <ID> ");
	grammar_list.push_back("ReturnStatement ::= return Exp | return");
	grammar_list.push_back("Relop ::= > | < | >= | <= | == | !=");
	grammar_list.push_back("IfStatement ::= if IfSign1 ( Exp ) IfSign2 Block ElseBlock");
	grammar_list.push_back("IfSign1 ::= @");
	grammar_list.push_back("IfSign2 ::= @");
	grammar_list.push_back("ElseBlock ::= @ | ElseSign else Block");
	grammar_list.push_back("ElseSign ::= @");
	grammar_list.push_back("WhileStatement ::= while WhileSign1 ( Exp ) WhileSign2 Block");
	grammar_list.push_back("WhileSign1 ::= @");
	grammar_list.push_back("WhileSign2 ::= @");
	read_grammar();
	for (auto ter = terminals.begin(); ter != terminals.end(); ter++)
		symbols[*ter].first_set.insert(*ter);

	getFirstOfNonterminal();
}


void Grammar::read_grammar()
{
	symbols.push_back(Gsymb(Gsymb::End, EndToken));
	terminals.insert(int(symbols.size()) - 1);
	symbols.push_back(Gsymb(Gsymb::Epsilon, EpsilonToken));
	int grammar_row_num = 0;

	string line;
	for (int i = 0; i < grammar_list.size(); i++)
	{
		line = grammar_list[i];
		grammar_row_num++;
		if (line.empty())
			continue;

		trim(line);
		if (line.find("$") != line.npos)
			line.erase(line.find_first_of("$"));
		if (line.empty())
			continue;

		vector<string> production_left_and_right = split(line, ProToken);
		string production_left = production_left_and_right[0];
		string production_right = production_left_and_right[1];

		int left_symbol = -1;

		if (production_left != AllTerminalToken) {
			left_symbol = find_symbol_index_by_token(production_left);

			if (left_symbol == -1) {
				symbols.push_back(Gsymb(Gsymb::NonTerminal, production_left));
				left_symbol = symbols.size() - 1;
				non_terminals.insert(left_symbol);
			}
		}
		vector<string> production_right_parts = split(production_right, SplitToken);

		for (auto production_right_it = production_right_parts.begin(); production_right_it != production_right_parts.end(); production_right_it++)
		{
			if (left_symbol == -1) {
				symbols.push_back(Gsymb(Gsymb::Terminal, *production_right_it));
				terminals.insert(symbols.size() - 1);
			}
			else {
				vector<int> right_symbol;
				vector<string> right_symbol_str = split(*production_right_it, " ");
				for (auto right_symbol_str_it = right_symbol_str.begin(); right_symbol_str_it != right_symbol_str.end(); right_symbol_str_it++) {
					int right_symbol_present = find_symbol_index_by_token(*right_symbol_str_it);
					if (right_symbol_present == -1) {
						symbols.push_back(Gsymb(Gsymb::NonTerminal, *right_symbol_str_it));
						right_symbol_present = symbols.size() - 1;
						non_terminals.insert(right_symbol_present);
					}
					right_symbol.push_back(right_symbol_present);
				}
				productions.push_back(Gproduction(left_symbol, right_symbol));

				if (symbols[left_symbol].token == ExtendStartToken)
					start_production = productions.size() - 1;
			}
		}
	}
}

int Grammar::find_symbol_index_by_token(const string token)
{
	auto symbol_it = find_if(symbols.begin(), symbols.end(), [token](Gsymb it)->bool {
		return it.token == token;
		});
	if (symbol_it == symbols.end()) {
		return -1;
	}
	else {
		return int(symbol_it - symbols.begin());
	}
}

void Grammar::getFirstOfNonterminal()
{
	bool changed;
	while (true) {
		changed = false;

		for (auto nonTerminal = non_terminals.begin(); nonTerminal != non_terminals.end(); nonTerminal++)
		{
			for (auto production = productions.begin(); production != productions.end(); production++)
			{

				if (production->left_symbol != *nonTerminal)
					continue;
				auto it = production->right_symbol.begin();
				if (terminals.find(*it) != terminals.end() || symbols[*it].type == Gsymb::Epsilon) {

					changed = symbols[*nonTerminal].first_set.insert(*it).second || changed;
					continue;
				}

				bool fprehandleg = true;
				for (; it != production->right_symbol.end(); ++it) {

					if (terminals.find(*it) != terminals.end()) {
						changed = mergeSetExceptEmpty(symbols[*nonTerminal].first_set, symbols[*it].first_set, find_symbol_index_by_token(EpsilonToken)) || changed;
						fprehandleg = false;
						break;
					}
					changed = mergeSetExceptEmpty(symbols[*nonTerminal].first_set, symbols[*it].first_set, find_symbol_index_by_token(EpsilonToken)) || changed;
					fprehandleg = fprehandleg && symbols[*it].first_set.count(find_symbol_index_by_token(EpsilonToken));

					if (!fprehandleg)
						break;
				}

				if (fprehandleg && it == production->right_symbol.end())
					changed = symbols[*nonTerminal].first_set.insert(find_symbol_index_by_token(EpsilonToken)).second || changed;
			}
		}

		if (!changed)
			break;
	}
}

set<int> Grammar::getFirstOfString(const vector<int>& str)
{

	set<int> FirstSet;

	if (str.empty())
		return FirstSet;

	bool epsilonIn = true;

	for (auto it = str.begin(); it != str.end(); it++) {

		if (symbols[*it].type == Gsymb::Terminal)
		{
			mergeSetExceptEmpty(FirstSet, symbols[*it].first_set, find_symbol_index_by_token(EpsilonToken));
			epsilonIn = false;
			break;
		}

		if (symbols[*it].type == Gsymb::Epsilon)
		{
			FirstSet.insert(*it);
			epsilonIn = false;
			break;
		}

		mergeSetExceptEmpty(FirstSet, symbols[*it].first_set, find_symbol_index_by_token(EpsilonToken));

		epsilonIn = epsilonIn && symbols[*it].first_set.count(find_symbol_index_by_token(EpsilonToken));
		if (!epsilonIn)
			break;
	}

	if (epsilonIn)
		FirstSet.insert(find_symbol_index_by_token(EpsilonToken));
	return FirstSet;

}

//预处理部分
prehandle::prehandle()
{
	search_p = 0;
	word_num = 0;
	const_num = 0;
	character_num = 0;
	ch = 1;
	currow = 1;
	strToken = "";
	max_idlength = 0;
	max_constlength = 0;
	error = 0;
}

prehandle::~prehandle()
{
	PrestructList.RemoveAll();
}

void prehandle::SetText(CString text)
{
	prime_txt = text;
}

void prehandle::GetChar(BOOL fprehandleg)
{
	if (fprehandleg)
		ch = preprocessed_txt[search_p++];
	else
		ch = prime_txt[search_p++];
}

BOOL prehandle::GetBC(BOOL fprehandleg)
{
	if (ch == ' ' || ch == '\t')
	{
		GetChar(fprehandleg);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL prehandle::IsNewRow(BOOL fprehandleg)
{
	if (ch == '\n')
	{
		currow++;
		GetChar(fprehandleg);
		return TRUE;
	}
	else if (ch == '\r')
	{
		GetChar(fprehandleg);
		return TRUE;
	}
	else
		return FALSE;
}

void prehandle::Concat()
{
	strToken += ch;
}

void prehandle::Retract()
{
	search_p--;
}

void prehandle::ClearStr()
{
	strToken = "";
}

void prehandle::Reset()
{
	strToken = "";
	ch = 1;
	search_p = 0;
	currow = 1;
	word_num = 0;
	const_num = 0;
	character_num = 0;
	max_idlength = 0;
	max_constlength = 0;
}

void prehandle::PreProcessing()
{
	CString x, y;
	prestruct p;
	error = 0;
	x = "";
	y = "";
	Reset();
	ClearStr();
	GetChar(0);
	preprocessed_txt = "";
	char cend = CODEEND;
	while (GetBC(0) || IsNewRow(0));
	while (ch != 0)
	{
		if (ch == '#')
		{
			if (prime_txt.Mid(search_p, 7) != "include" && prime_txt.Mid(search_p, 7) != "define ")
			{
				if (cend != '#')
				{
					CopeError(0);
					return;
				}
				else
				{
					break;
				}
			}
			GetChar(0);
			if (ch == 'i')
			{
				while (1)
				{
					GetChar(0);
					if (ch == '\r' || ch == '\n' || ch == 0)
						break;
				}
				while (GetBC(0) || IsNewRow(0));
			}
			else if (ch == 'd')
			{
				search_p += 5;
				GetChar(0);
				while (GetBC(0) || IsNewRow(0));
				while (1)
				{
					x += ch;
					GetChar(0);
					if (ch == ' ' || ch == 0)
						break;
				}
				p.x = x;
				x = "";
				while (GetBC(0));
				if (ch == '\r' || ch == '\n' || ch == 0)
				{
					while (GetBC(0) || IsNewRow(0));
					continue;
				}
				while (1)
				{
					y += ch;
					GetChar(0);
					if (ch == ' ' || ch == '\r' || ch == '\n' || ch == 0)
						break;
				}
				p.y = y;
				y = "";
				PrestructList.Add(p);
				while (GetBC(0) || IsNewRow(0));
			}
		}
		else if (ch == ' ')
		{
			Pre_Concat();
			while (GetBC(0));
			if (ch == '\n' || ch == '\r')
				continue;
			else
				preprocessed_txt += ' ';
		}
		else if (ch == '\n' || ch == '\r')
		{
			Pre_Concat();
			preprocessed_txt += "\r\n";
			while (GetBC(0) || IsNewRow(0));
		}
		else if (ch == '/')
		{
			if (prime_txt.GetAt(search_p) == '/')
			{
				while (1)
				{
					GetChar(0);
					if (ch == '\r' || ch == '\n' || ch == 0)
						break;
				}
			}
			else if (prime_txt.GetAt(search_p) == '*')
			{
				while (1)
				{
					GetChar(0);
					if (prime_txt.Mid(search_p, 2) == "*/")
					{
						search_p += 2;
						GetChar(0);
						break;
					}
					if (ch == 0)
						break;
				}
				while (GetBC(0) || IsNewRow(0));
			}
			else
			{
				Concat();
				GetChar(0);
			}
		}
		else
		{
			Concat();
			GetChar(0);
		}
	}
	Pre_Concat();
	if (cend != 0)
	{
		preprocessed_txt += cend;
	}
	Reset();
	PrestructList.RemoveAll();
}

void prehandle::CopeError(int id)
{
	/*
	*0:预处理错误
	*/
	if (id == 0)
	{
		ErorrMessage.Format("预处理出错！出错在：%d行附近", currow);
		AfxMessageBox(ErorrMessage);
		error = 1;
	}
	else if (id == 1)
	{
		ErorrMessage.Format("词法分析出错，有无法识别单词！出错在：%d行附近", currow);
		AfxMessageBox(ErorrMessage);
	}
	else if (id == 3)
	{
		ErorrMessage.Format("语法分析出错！出错在：%d行附近，具体错误请见分析过程", currow);
		AfxMessageBox(ErorrMessage);
	}
}

void prehandle::Pre_Concat()
{
	if (strToken == "")
		return;
	int n = PrestructList.GetSize();
	for (int i = 0; i < n; i++)
	{
		prestruct p = PrestructList.GetAt(i);
		strToken.Replace(p.x, p.y);
	}
	preprocessed_txt += strToken;
	ClearStr();
}


//词法分析部分
const set<string> Keyword = { "int" ,"void" ,"if" ,"else" ,"while" ,"return" };
const set<string> Separator = { "," , ";" , "(" , ")" , "{" , "}" };
const set<string> Operator_1 = { "+",  "-",  "*",  "/",  "=",  ">",  "<" };
const set<string> Operator_2 = { "==",  ">=",  "<=",  "!=" };
const string ID = "<ID>";
const string ConstInt = "<INT>";

set<string> InsertTokens()
{
	set<string> temp;
	temp.insert(Keyword.begin(), Keyword.end());
	temp.insert(Separator.begin(), Separator.end());
	temp.insert(Operator_1.begin(), Operator_1.end());
	temp.insert(Operator_2.begin(), Operator_2.end());
	temp.insert(ID);
	temp.insert(ConstInt);
	return temp;
}
const set<string> AllTokens = move(InsertTokens());

LA::LA()
{
	Lerror = 0;
	ifstream file_in;
	file_in.open(default_dir, ios::in | ios::binary);
	int present_row = 1;
	int present_col = 0;
	string present_str;
	char present_ch;


	while (file_in.peek() != EOF) {
		present_ch = char(file_in.get());
		present_col++;

		if (isspace(present_ch)) {
			if (present_ch == '\n') {
				present_row++;
				present_col = 0;
			}
			else if (present_ch == '\t') {
				present_col += 3;
			}
			continue;
		}

		present_str = present_ch;


		if (isalpha(present_ch)) {

			while (isalnum(present_ch = char(file_in.get()))) {
				present_str += present_ch;
				present_col++;
			}


			file_in.seekg(-1, ios::cur);

			if (Keyword.find(present_str) != Keyword.cend())
				Token_Stack.push_back({ present_str,present_str,present_row,present_col - int(present_str.length()) + 1 });
			else
				Token_Stack.push_back({ ID,present_str,present_row,present_col - int(present_str.length()) + 1 });
		}

		else if (isdigit(present_ch)) {

			while (isdigit(present_ch = char(file_in.get()))) {
				present_str += present_ch;
				present_col++;
			}

			file_in.seekg(-1, ios::cur);

			Token_Stack.push_back({ ConstInt,present_str,present_row,present_col - int(present_str.length()) + 1 });
		}

		else if (Separator.find(present_str) != Separator.cend())
			Token_Stack.push_back({ present_str,present_str,present_row,present_col });

		else if (present_ch == '/' && file_in.peek() == '/') {
			while (char(file_in.get()) != '\n')
				;
			present_row++;
			present_col = 0;
		}

		else if (present_ch == '/' && file_in.peek() == '*') {
			while (!((present_ch = char(file_in.get())) == '*' && char(file_in.peek()) == '/')) {
				present_col++;
				if (present_ch == '\n') {
					present_row++;
					present_col = 0;
				}
				else if (present_ch == '\t')
					present_col += 3;
			}
			file_in.get();
			present_col += 2;
		}

		else if (Operator_2.find(present_str + char(file_in.peek())) != Operator_2.cend()) {
			present_str += char(file_in.get());
			present_col++;
			Token_Stack.push_back({ present_str,present_str,present_row,present_col - 1 });
		}

		else if (Operator_1.find(present_str) != Operator_1.cend())
			Token_Stack.push_back({ present_str,present_str,present_row,present_col });
		else {
			CString em;
			em.Format("词法分析错误，有无法识别符号！错误位置%d行", present_row);
			AfxMessageBox(em);
			Lerror = 1;
			return;
		}
	}
}

vector<Token> LA::Get_Token_Stack()
{
	return Token_Stack;
}


//语法分析部分
GA_item::GA_item(const int left_symbol_, const vector<int>& right_symbol_, const int index_, const int dot_position_, const int lookahead_symbol_)
{
	this->left_symbol = left_symbol_;
	this->right_symbol = right_symbol_;
	this->index = index_;
	this->dot_position = dot_position_;
	this->lookahead_symbol = lookahead_symbol_;
}

bool GA_item::operator==(const GA_item& item)
{
	return (this->left_symbol == item.left_symbol && this->right_symbol == item.right_symbol && this->dot_position == item.dot_position && this->index == item.index && this->lookahead_symbol == item.lookahead_symbol);
}

bool GA_closure::operator==(const GA_closure& closure)
{
	if (this->lr1_closure.size() != closure.lr1_closure.size()) {
		return false;
	}
	int count = 0;
	for (auto& tmp : this->lr1_closure) {
		for (auto& tmp_ : closure.lr1_closure) {
			if (tmp == tmp_) {
				++count;
				break;
			}
		}
	}
	return count == this->lr1_closure.size();
}

GA::GA() {

	ClusterGenerate();
	TableGenerate();
}

void GA::ClusterGenerate()
{

	GA_item initial_item(find_symbol_index_by_token(ExtendStartToken), { find_symbol_index_by_token(StartToken) }, start_production, 0, find_symbol_index_by_token(EndToken));
	GA_closure initial_closure;
	initial_closure.lr1_closure.push_back(initial_item);

	lr1_cluster.push_back(ClosureGenerate(initial_closure));


	for (int i = 0; i < lr1_cluster.size(); i++) {

		for (int s = 0; s < symbols.size(); s++) {

			if (symbols[s].type != Gsymb::NonTerminal && symbols[s].type != Gsymb::Terminal)
				continue;

			GA_closure to_closure = GotoGenerate(lr1_cluster[i], s);

			if (to_closure.lr1_closure.empty())
				continue;

			int exist_index = -1;
			for (int j = 0; j < lr1_cluster.size(); j++) {
				if (lr1_cluster[j] == to_closure) {
					exist_index = j;
					break;
				}
			}
			if (exist_index != -1)
				goto_info[{i, s}] = exist_index;
			else {
				lr1_cluster.push_back(to_closure);

				goto_info[{i, s}] = lr1_cluster.size() - 1;
			}
		}
	}
}

GA_closure& GA::ClosureGenerate(GA_closure& initial_closure)
{

	for (int i = 0; i < initial_closure.lr1_closure.size(); i++) {
		GA_item present_lr1_item = initial_closure.lr1_closure[i];

		if (present_lr1_item.dot_position >= static_cast<int>(present_lr1_item.right_symbol.size()))
			continue;

		int next_symbol_index = present_lr1_item.right_symbol[present_lr1_item.dot_position];
		Gsymb next_symbol = symbols[next_symbol_index];

		if (next_symbol.type == Gsymb::Terminal)
			continue;

		if (next_symbol.type == Gsymb::Epsilon) {
			initial_closure.lr1_closure[i].dot_position++;
			continue;
		}

		vector<int> BetaA(present_lr1_item.right_symbol.begin() + present_lr1_item.dot_position + 1, present_lr1_item.right_symbol.end());
		BetaA.push_back(present_lr1_item.lookahead_symbol);
		set<int> BetaAFirstSet = getFirstOfString(BetaA);

		for (int j = 0; j < productions.size(); j++) {
			Gproduction present_production = productions[j];
			if (present_production.left_symbol != next_symbol_index)
				continue;

			for (auto it = BetaAFirstSet.begin(); it != BetaAFirstSet.end(); it++) {

				if (symbols[present_production.right_symbol.front()].type == Gsymb::Epsilon) {

					vector<GA_item>::iterator tmp;
					for (tmp = initial_closure.lr1_closure.begin(); tmp != initial_closure.lr1_closure.end(); tmp++) {
						if (*tmp == GA_item({ present_production.left_symbol,present_production.right_symbol,j,1, *it }))
							break;
					}
					if (tmp == initial_closure.lr1_closure.end())
						initial_closure.lr1_closure.push_back({ present_production.left_symbol,present_production.right_symbol,j,1, *it });
				}
				else {

					vector<GA_item>::iterator tmp;
					for (tmp = initial_closure.lr1_closure.begin(); tmp != initial_closure.lr1_closure.end(); tmp++) {
						if (*tmp == GA_item({ present_production.left_symbol,present_production.right_symbol,j,0, *it }))
							break;
					}
					if (tmp == initial_closure.lr1_closure.end())
						initial_closure.lr1_closure.push_back({ present_production.left_symbol,present_production.right_symbol,j,0, *it });
				}
			}
		}
	}
	return initial_closure;
}

GA_closure GA::GotoGenerate(const GA_closure& from_closure, int present_symbol)
{
	GA_closure to_closure;

	if (symbols[present_symbol].type != Gsymb::NonTerminal && symbols[present_symbol].type != Gsymb::Terminal)
		return to_closure;
	for (auto lr1_item_it = from_closure.lr1_closure.begin(); lr1_item_it != from_closure.lr1_closure.end(); lr1_item_it++) {

		if (lr1_item_it->dot_position >= lr1_item_it->right_symbol.size())
			continue;

		if (lr1_item_it->right_symbol[lr1_item_it->dot_position] != present_symbol)
			continue;

		to_closure.lr1_closure.push_back({ lr1_item_it->left_symbol,lr1_item_it->right_symbol,lr1_item_it->index,lr1_item_it->dot_position + 1,lr1_item_it->lookahead_symbol });
	}
	return ClosureGenerate(to_closure);
}

void GA::TableGenerate()
{
	for (int closure_index = 0; closure_index < lr1_cluster.size(); closure_index++) {
		for (int lr1_item_index = 0; lr1_item_index < lr1_cluster[closure_index].lr1_closure.size(); lr1_item_index++) {
			GA_item present_lr1_item = lr1_cluster[closure_index].lr1_closure[lr1_item_index];


			if (present_lr1_item.dot_position >= present_lr1_item.right_symbol.size()) {

				if (symbols[present_lr1_item.left_symbol].token != ExtendStartToken)
					action_table[{closure_index, present_lr1_item.lookahead_symbol}] = { Action::Reduce,present_lr1_item.index };
				else
					action_table[{closure_index, present_lr1_item.lookahead_symbol}] = { Action::Accept, -1 };
			}
			else {
				int next_symbol = present_lr1_item.right_symbol[present_lr1_item.dot_position];

				if (symbols[next_symbol].type == Gsymb::NonTerminal) {

					auto it = goto_info.find({ closure_index,next_symbol });

					if (it != goto_info.end())
						goto_table[{closure_index, next_symbol}] = { Action::ShiftIn,it->second };
				}

				else if (symbols[next_symbol].type == Gsymb::Terminal) {

					auto it = goto_info.find({ closure_index,next_symbol });

					if (it != goto_info.end())
						action_table[{closure_index, next_symbol}] = { Action::ShiftIn,it->second };
				}
			}
		}
	}
}
