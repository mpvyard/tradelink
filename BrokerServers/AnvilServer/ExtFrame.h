#if !defined(AFX_EXTFRAME_H__0E7D4985_B30B_4067_BFC0_0DB7C6D9C54C__INCLUDED_)
#define AFX_EXTFRAME_H__0E7D4985_B30B_4067_BFC0_0DB7C6D9C54C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExtFrame.h : header file
//

#include "Messages.h"
#include "ObserverApi.h"
#include "Monitor.h"

class Money;
class MoneySize;
//class BaseDlg;

/////////////////////////////////////////////////////////////////////////////
// ExtFrame frame

class ExtFrame : public CFrameWnd, public Observer
{
//	DECLARE_DYNCREATE(ExtFrame)
public:
	ExtFrame();           // protected constructor used by dynamic creation
	virtual ~ExtFrame();
    void ShowWindowAndChildren(int show);
	Monitor* monitor;

// Attributes
public:
//    const std::string& GetVersion() const{return m_version;}
    static ExtFrame* GetInstance(){return instance;}
	/*
    static void FormatDollarsAndCents(CString& dest, int dollars, int cents);
    static void FormatMoney(CString& dest, const Money& money);
    static void FormatMoneySize(CString& dest, const MoneySize& money);
    static void FormatTimeToken(unsigned int i, std::string& str);
    static void FormatTif(unsigned int tif, std::string& text);
	*/
//    static CString ExtFrame::FormatPrice(const Money& price)
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ExtFrame)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ExtFrame)
	afx_msg void OnAccount();
	afx_msg void OnPositions();
	afx_msg void OnOrders();
	afx_msg void OnQuotes();
	afx_msg void OnBookQuotes();
	afx_msg void OnAgrBookQuotes();
	afx_msg void OnLevel1();
	afx_msg void OnSubscription();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnLoadStocks();
	afx_msg void OnSendOrder();
	afx_msg void OnCancelOrder();
	afx_msg void OnIndex();
	afx_msg void OnSendStopOrder();
	afx_msg void OnAnvignalName();
	afx_msg void OnRegister();
	afx_msg void OnPositionsOrdersExecutions();
	afx_msg void OnSingleBook();
	afx_msg void OnMultiBook();
	afx_msg BOOL OnCopyData(CWnd* sWnd,COPYDATASTRUCT* pCD);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    virtual void Process(const Message* message, Observable* from, const Message* additionalInfo);
    void MarketSummaryPopulationDone(bool done);

    static ExtFrame* instance;

    bool m_marketSummaryPopulationDone;

//    std::string m_version;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXTFRAME_H__0E7D4985_B30B_4067_BFC0_0DB7C6D9C54C__INCLUDED_)
