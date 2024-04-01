
// SemanticAnalysisDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SemanticAnalysis.h"
#include "SemanticAnalysisDlg.h"
#include "afxdialogex.h"
#include "Resource.h"
#include<fstream>
#include<string>
#include"MYSA.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// CSemanticAnalysisDlg 对话框

prehandle prh; //预处理器，将预处理后的文本保存到./test_code.txt中供语义分析使用
GA lr;


CSemanticAnalysisDlg::CSemanticAnalysisDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SEMANTICANALYSIS_DIALOG, pParent)
	, mycode(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSemanticAnalysisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, mylist);
	DDX_Text(pDX, IDC_EDIT1, mycode);
	DDX_Control(pDX, IDC_EDIT1, myedit);
}

BEGIN_MESSAGE_MAP(CSemanticAnalysisDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_LOAD, &CSemanticAnalysisDlg::OnLoad)
	ON_COMMAND(ID_SAVE, &CSemanticAnalysisDlg::OnSave)
	ON_BN_CLICKED(IDC_PREPRO, &CSemanticAnalysisDlg::OnBnClickedPrepro)
	ON_BN_CLICKED(IDC_QUATERNARY, &CSemanticAnalysisDlg::OnBnClickedQuaternary)
	ON_BN_CLICKED(IDC_SYMB, &CSemanticAnalysisDlg::OnBnClickedSymb)
END_MESSAGE_MAP()


// CSemanticAnalysisDlg 消息处理程序

BOOL CSemanticAnalysisDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_Menu.LoadMenu(IDR_MENU1);
	SetMenu(&m_Menu);
	my_dir = "NULL";
	mylist.ModifyStyle(0, LVS_REPORT);
	mylist.SetExtendedStyle(LVS_EX_GRIDLINES);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSemanticAnalysisDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSemanticAnalysisDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSemanticAnalysisDlg::CleanList()//清除当前列表内容
{
	mylist.DeleteAllItems();
	while (mylist.DeleteColumn(0));
}

void CSemanticAnalysisDlg::OnLoad()
{
	CFileDialog opendlg(TRUE, _T("*"), _T("*.cpp"), OFN_OVERWRITEPROMPT, _T("所有文件(*.*;)|*.*|C++文件(*.cpp)|*.cpp|C文件(*.c)|*.c|头文件(*.h)|*.h|文本文件(*.txt)|*.txt||"), NULL);
	if (opendlg.DoModal() == IDOK)
	{
		my_dir = opendlg.GetPathName();
	}
	ifstream fin(my_dir);
	ofstream fout("a.txt");
	char c;
	int flag = 0;
	while (fin >> c)
	{
		if (c == '<')
			flag = 0;
		if (flag)
			fout << c;
		if (c == '>')
			flag = 1;
	}

	if (my_dir == "NULL")
		return;

	ifstream readFile(my_dir);
	string s1, s2;
	while (getline(readFile, s1))
	{
		s2 += s1;
		s2 += "\r\n";
	}
	int len = s2.length();
	s2[len - 1] = 0;
	mycode.Format("%s", s2.c_str());

	readFile.close();
	GetDlgItem(IDC_EDIT1)->SetWindowText(mycode);
	my_dir = "NULL";
}


void CSemanticAnalysisDlg::OnSave()
{
	UpdateData(TRUE);

	CFileDialog opendlg(FALSE, _T("*"), NULL, OFN_OVERWRITEPROMPT, _T("所有文件(*.*;)|*.*|C++文件(*.cpp)|*.cpp|C文件(*.c)|*.c|头文件(*.h)|*.h|文本文件(*.txt)|*.txt||"), NULL);
	if (opendlg.DoModal() == IDOK)
	{
		my_dir = opendlg.GetPathName();
	}
	ifstream fin(my_dir);
	ofstream fout("a.txt");
	char c;
	int flag = 0;
	while (fin >> c)
	{
		if (c == '<')
			flag = 0;
		if (flag)
			fout << c;
		if (c == '>')
			flag = 1;
	}

	if (my_dir == "NULL")
		return;

	CFile file(my_dir, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone);
	file.Write(mycode, strlen(mycode));
	file.Close();
	my_dir = "NULL";
}


void CSemanticAnalysisDlg::OnBnClickedPrepro()//预处理
{
	UpdateData(TRUE);
	prh.prime_txt = mycode;
	prh.PreProcessing();
	if (prh.error)
		return;
	prh.prime_txt = prh.preprocessed_txt;
	prh.PreProcessing();
	if (prh.error)
		return;
	mycode = prh.preprocessed_txt;
	UpdateData(FALSE);
	CFile file(DEFAULT_DIR, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone);
	file.Write(mycode, strlen(mycode));
	file.Close();
}


//生成四元式
void CSemanticAnalysisDlg::OnBnClickedQuaternary()
{
	UpdateData(TRUE);
	prh.prime_txt = mycode;
	prh.PreProcessing();
	if (prh.error)
		return;
	prh.prime_txt = prh.preprocessed_txt;
	prh.PreProcessing();
	if (prh.error)
		return;
	mycode = prh.preprocessed_txt;
	UpdateData(FALSE);
	CFile file(DEFAULT_DIR, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone);
	file.Write(mycode, strlen(mycode));
	file.Close();

	LA ls;
	if (ls.Lerror)
		return;
	CleanList();
	mylist.InsertColumn(0, "序号", LVCFMT_CENTER, 200);
	mylist.InsertColumn(1, "四元式", LVCFMT_CENTER, 500);
	lr.Parser(ls.Get_Token_Stack());
	lr.symantic_analysis.quaternary_list_generate();
	int count = 0;
	for (int i = 0; i < lr.symantic_analysis.quaternary_list.size(); i++)
	{
		CString tmp = lr.symantic_analysis.quaternary_list[i].c_str();
		CString tmp2;
		tmp2.Format("%d", count);
		mylist.InsertItem(i, tmp2);
		mylist.SetItemText( i, 1, tmp);
		count++;
	}	
}


void CSemanticAnalysisDlg::OnBnClickedSymb()
{
	CleanList();
	mylist.InsertColumn(0, "作用域", LVCFMT_CENTER, 100);
	mylist.InsertColumn(1, "符号类型", LVCFMT_CENTER, 300);
	mylist.InsertColumn(2, "符号名字", LVCFMT_CENTER, 300);

	int count = 0;
	for (int i = 0; i < lr.symantic_analysis.tables.size(); i++)
	{
		Ssymb_table table_t = lr.symantic_analysis.tables[i];
		for (int j = 0; j < table_t.table.size(); j++)
		{
			CString tmp = table_t.table[j].ID_value_type.c_str();
			CString tmp2 = table_t.table_name.c_str();
			CString tmp3 = table_t.table[j].ID_value.c_str();
			mylist.InsertItem(count, tmp2);
			mylist.SetItemText(count, 1, tmp);
			mylist.SetItemText(count, 2, tmp3);
			count++;
		}
	}
}
