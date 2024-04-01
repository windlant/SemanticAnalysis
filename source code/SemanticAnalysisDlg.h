
// SemanticAnalysisDlg.h: 头文件
//

#pragma once

// CSemanticAnalysisDlg 对话框
class CSemanticAnalysisDlg : public CDialogEx
{
// 构造
public:
	CSemanticAnalysisDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SEMANTICANALYSIS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	void CleanList();//清除当前列表
	CEdit myedit;
	CString mycode;
	CListCtrl mylist;
	CMenu m_Menu;
	CString my_dir;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLoad();
	afx_msg void OnSave();
	afx_msg void OnBnClickedPrepro();
	afx_msg void OnBnClickedQuaternary();
	afx_msg void OnBnClickedSymb();
};
