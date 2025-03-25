
// CJH_Circle3PtAppDlg.h : header file
//

#pragma once

// CCJH_Circle3PtAppDlg dialog
class CCJH_Circle3PtAppDlg : public CDialogEx
{
// Construction
public:
	CCJH_Circle3PtAppDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CCJH_Circle3PtAppDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CJH_CIRCLE3PTAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions	
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	
	afx_msg void OnBnClickedReset();
	afx_msg void OnBnClickedRandomMove();
	
	afx_msg LRESULT OnUpdateCanvas(WPARAM wParam, LPARAM lParam);
	
	afx_msg void OnEnChangePtRadius();
	afx_msg void OnEnChangeCircleThickness();
	afx_msg void OnDeltaposSpinPtRadius(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinCircleThickness(NMHDR* pNMHDR, LRESULT* pResult);
	
	DECLARE_MESSAGE_MAP()

protected:
		
	/*Allocation of Variable*/
	CImage m_canvas;
	CRect   m_canvasRect;
	int    m_canvasWidth;
	int    m_canvasHeight;
	int    m_imageBpp;
		
	int   m_clickCounter;
	CPoint m_ptArray[3];
	bool   m_pointDefined[3];

	int m_ptRadius;
	int m_circleThickness;
	CSpinButtonCtrl m_spinPtRadius;           
	CSpinButtonCtrl m_spinCircleThickness;    

	bool  m_isDragging;
	int   m_dragIndex;
	CPoint m_lastDragPos;

	CWinThread* m_pRandomThread;
	bool        m_threadActive;

	CRITICAL_SECTION m_csPoints;

	/*Allocation of Function*/
	bool ComputeCircleFromPoints(const CPoint& p1, const CPoint& p2, const CPoint& p3, CPoint& center, double& radius);
	void RenderCircle(CImage& img, const CPoint& center, double radius, int thickness, COLORREF color);
	void RenderPoints(CImage& img, int radius, unsigned char shade);
	void RefreshCanvas();
	void DisplayCanvas();
	void UpdateCoordinateLabels();	
	static UINT RandomMovementThread(LPVOID pParam);
};
