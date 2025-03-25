
// CJH_Circle3PtAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CJH_Circle3PtApp.h"
#include "CJH_Circle3PtAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CCJH_Circle3PtAppDlg dialog
CCJH_Circle3PtAppDlg::CCJH_Circle3PtAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CJH_CIRCLE3PTAPP_DIALOG, pParent),	
	m_canvasWidth(640),
	m_canvasHeight(480),
	m_clickCounter(0),
	m_isDragging(false),
	m_dragIndex(-1),
	m_pRandomThread(nullptr),
	m_threadActive(false),
	m_imageBpp(8),
	m_ptRadius(10),
	m_circleThickness(3)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	for (int i = 0; i < 3; i++) {
		m_pointDefined[i] = false;
		m_ptArray[i] = CPoint(0, 0);
	}
	srand((unsigned int)time(NULL));
	InitializeCriticalSection(&m_csPoints);
}

CCJH_Circle3PtAppDlg::~CCJH_Circle3PtAppDlg()
{
	DeleteCriticalSection(&m_csPoints);
	if (m_threadActive && m_pRandomThread) {
		m_threadActive = false;
		WaitForSingleObject(m_pRandomThread->m_hThread, INFINITE);
		m_pRandomThread = nullptr;
	}
}

void CCJH_Circle3PtAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PT_RADIUS, m_ptRadius);
	DDX_Text(pDX, IDC_EDIT_CIRCLE_THICKNESS, m_circleThickness);
	DDX_Control(pDX, IDC_SPIN_PT_RADIUS, m_spinPtRadius);
	DDX_Control(pDX, IDC_SPIN_CIRCLE_THICKNESS, m_spinCircleThickness);
}

BEGIN_MESSAGE_MAP(CCJH_Circle3PtAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()

	ON_BN_CLICKED(IDC_BUTTON_INIT, &CCJH_Circle3PtAppDlg::OnBnClickedReset)
	ON_BN_CLICKED(IDC_BUTTON_RANDOM, &CCJH_Circle3PtAppDlg::OnBnClickedRandomMove)
	ON_MESSAGE(WM_UPDATE_DRAW, &CCJH_Circle3PtAppDlg::OnUpdateCanvas)

	ON_EN_CHANGE(IDC_EDIT_PT_RADIUS, &CCJH_Circle3PtAppDlg::OnEnChangePtRadius)
	ON_EN_CHANGE(IDC_EDIT_CIRCLE_THICKNESS, &CCJH_Circle3PtAppDlg::OnEnChangeCircleThickness)	
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_PT_RADIUS, &CCJH_Circle3PtAppDlg::OnDeltaposSpinPtRadius)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CIRCLE_THICKNESS, &CCJH_Circle3PtAppDlg::OnDeltaposSpinCircleThickness)

END_MESSAGE_MAP()

// CCJH_Circle3PtAppDlg message handlers
BOOL CCJH_Circle3PtAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		BOOL bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
		
	/*Allocation of Canvas Interface*/
	CWnd* pCanvasWnd = GetDlgItem(IDC_STATIC_CANVAS);
	if (pCanvasWnd) {
		pCanvasWnd->GetWindowRect(&m_canvasRect);
		ScreenToClient(&m_canvasRect);		
	}

	CWnd* pCanvas = GetDlgItem(IDC_STATIC_CANVAS);
	if (pCanvas) {
		pCanvas->SetWindowPos(NULL, 0, 0, 640, 480, SWP_NOMOVE | SWP_NOZORDER);
	}
		
	m_canvas.Create(m_canvasWidth, -m_canvasHeight, m_imageBpp);
	if (m_imageBpp == 8) {
		RGBQUAD palette[256];
		for (int i = 0; i < 256; i++) {
			palette[i].rgbRed = (BYTE)i;
			palette[i].rgbGreen = (BYTE)i;
			palette[i].rgbBlue = (BYTE)i;
			palette[i].rgbReserved = 0;
		}
		m_canvas.SetColorTable(0, 256, palette);
	}

	// Canvas Initialization
	int pitch = m_canvas.GetPitch();
	unsigned char* pBuffer = (unsigned char*)m_canvas.GetBits();
	memset(pBuffer, 0xff, pitch * m_canvasHeight);

	m_spinPtRadius.SetRange(1, 300);
	m_spinPtRadius.SetPos(m_ptRadius);
	m_spinCircleThickness.SetRange(1, 30);
	m_spinCircleThickness.SetPos(m_circleThickness);
	
	UpdateCoordinateLabels();
	UpdateData(FALSE);
	return TRUE;  
}

void CCJH_Circle3PtAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CCJH_Circle3PtAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);

		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		DisplayCanvas();
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCJH_Circle3PtAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static inline CPoint ToCanvasPt(const CRect& rcCanvas, CPoint ptScreen)
{
	CPoint ret = ptScreen;
	ret.Offset(-rcCanvas.left, -rcCanvas.top);
	return ret;
}

bool CCJH_Circle3PtAppDlg::ComputeCircleFromPoints(const CPoint& p1, const CPoint& p2, const CPoint& p3, CPoint& center, double& radius)
{
	double x1 = (double)p1.x, y1 = (double)p1.y;
	double x2 = (double)p2.x, y2 = (double)p2.y;
	double x3 = (double)p3.x, y3 = (double)p3.y;

	double denom = 2.0 * (x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2));
	if (fabs(denom) < 1e-6) {
		return false;
	}

	double centerX = ((x1*x1 + y1*y1)*(y2 - y3) +
		(x2*x2 + y2*y2)*(y3 - y1) +
		(x3*x3 + y3*y3)*(y1 - y2)) / denom;
	double centerY = ((x1*x1 + y1*y1)*(x3 - x2) +
		(x2*x2 + y2*y2)*(x1 - x3) +
		(x3*x3 + y3*y3)*(x2 - x1)) / denom;

	center.x = (int)(centerX + 0.5);
	center.y = (int)(centerY + 0.5);

	radius = sqrt((x1 - centerX)*(x1 - centerX) + (y1 - centerY)*(y1 - centerY));
	return true;
}

void CCJH_Circle3PtAppDlg::RenderCircle(CImage& img, const CPoint& center, double radius, int thickness, COLORREF color)
{
	int imgW = img.GetWidth();
	int imgH = img.GetHeight();
	int pitch = img.GetPitch();
	unsigned char* pData = (unsigned char*)img.GetBits();

	// thickness가 있는 테두리(고리) 그리기
	double rOuter = radius + thickness / 2.0;
	double rInner = (radius > thickness / 2.0) ? radius - thickness / 2.0 : 0.0;
	double rOuterSq = rOuter * rOuter;
	double rInnerSq = rInner * rInner;

	int startX = max(0, center.x - (int)ceil(rOuter));
	int endX = min(imgW - 1, center.x + (int)ceil(rOuter));
	int startY = max(0, center.y - (int)ceil(rOuter));
	int endY = min(imgH - 1, center.y + (int)ceil(rOuter));

	unsigned char shade = (unsigned char)GetRValue(color);

	for (int y = startY; y <= endY; y++) {
		for (int x = startX; x <= endX; x++) {
			double dx = (double)(x - center.x);
			double dy = (double)(y - center.y);
			double distSq = dx*dx + dy*dy;
			if (distSq >= rInnerSq && distSq <= rOuterSq) {
				pData[y * pitch + x] = shade;
			}
		}
	}
}

void CCJH_Circle3PtAppDlg::RenderPoints(CImage& img, int radius, unsigned char shade)
{
	int width = img.GetWidth();
	int height = img.GetHeight();
	int pitch = img.GetPitch();
	unsigned char* pBuffer = (unsigned char*)img.GetBits();

	for (int i = 0; i < m_clickCounter; i++) {
		CPoint pt = m_ptArray[i];

		int left = max(0, pt.x - radius);
		int right = min(width - 1, pt.x + radius);
		int top = max(0, pt.y - radius);
		int bottom = min(height - 1, pt.y + radius);

		for (int y = top; y <= bottom; y++) {
			for (int x = left; x <= right; x++) {
				int dx = x - pt.x;
				int dy = y - pt.y;
				if (dx*dx + dy*dy <= radius*radius) {
					pBuffer[y * pitch + x] = shade;
				}
			}
		}
	}
}

void CCJH_Circle3PtAppDlg::RefreshCanvas()
{
	int pitch = m_canvas.GetPitch();
	unsigned char* buffer = (unsigned char*)m_canvas.GetBits();
	memset(buffer, 0xff, pitch * m_canvasHeight);
	RenderPoints(m_canvas, m_ptRadius, 0);

	if (m_clickCounter == 3) {
		CPoint center;
		double rad;
		if (ComputeCircleFromPoints(m_ptArray[0], m_ptArray[1], m_ptArray[2], center, rad)) {
			RenderCircle(m_canvas, center, rad, m_circleThickness, RGB(0, 0, 0));
		}
	}
}

void CCJH_Circle3PtAppDlg::DisplayCanvas()
{
	RefreshCanvas();

	CClientDC dc(this);
	m_canvas.Draw(dc.GetSafeHdc(), m_canvasRect.left, m_canvasRect.top);
}


void CCJH_Circle3PtAppDlg::UpdateCoordinateLabels()
{
	CString txt;
	for (int i = 0; i < 3; i++) {
		if (m_pointDefined[i] || i < m_clickCounter) {
			EnterCriticalSection(&m_csPoints);
			txt.Format(_T("Point %d: (%d, %d)"), i + 1, m_ptArray[i].x, m_ptArray[i].y);
			LeaveCriticalSection(&m_csPoints);
		}
		else {
			txt.Format(_T("Point %d: ( , )"), i + 1);
		}
		GetDlgItem(IDC_STATIC_POINT1 + i)->SetWindowText(txt);
	}
}

void CCJH_Circle3PtAppDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 캔버스 영역밖이면 무시
	if (!m_canvasRect.PtInRect(point)) {
		CDialogEx::OnLButtonDown(nFlags, point);
		return;
	}

	CPoint localPt = ToCanvasPt(m_canvasRect, point);

	EnterCriticalSection(&m_csPoints);
	if (localPt.x < m_ptRadius || localPt.x >(m_canvasWidth - m_ptRadius) ||
		localPt.y < m_ptRadius || localPt.y >(m_canvasHeight - m_ptRadius))
	{
		LeaveCriticalSection(&m_csPoints);
		return;
	}

	if (m_clickCounter < 3) {
		m_ptArray[m_clickCounter] = localPt;
		m_pointDefined[m_clickCounter] = true;
		m_clickCounter++;
		UpdateCoordinateLabels();
	}
	else {
		for (int i = 0; i<3; i++) {
			int dx = localPt.x - m_ptArray[i].x;
			int dy = localPt.y - m_ptArray[i].y;
			if (dx*dx + dy*dy <= m_ptRadius*m_ptRadius) {
				m_isDragging = true;
				m_dragIndex = i;
				m_lastDragPos = localPt;
				break;
			}
		}
	}
	LeaveCriticalSection(&m_csPoints);
	//InvalidateRect(m_canvasRect, FALSE);
	DisplayCanvas();
}

void CCJH_Circle3PtAppDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_isDragging) {
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	if (!m_canvasRect.PtInRect(point)) {
		CDialogEx::OnMouseMove(nFlags, point);
		return;
	}

	CPoint localPt = ToCanvasPt(m_canvasRect, point);

	if (m_dragIndex >= 0 && m_dragIndex<3) {
		int dx = localPt.x - m_lastDragPos.x;
		int dy = localPt.y - m_lastDragPos.y;

		EnterCriticalSection(&m_csPoints);
		int newX = m_ptArray[m_dragIndex].x + dx;
		int newY = m_ptArray[m_dragIndex].y + dy;

		if (newX < m_ptRadius) newX = m_ptRadius;
		else if (newX >(m_canvasWidth - m_ptRadius)) newX = (m_canvasWidth - m_ptRadius);
		if (newY < m_ptRadius) newY = m_ptRadius;
		else if (newY >(m_canvasHeight - m_ptRadius)) newY = (m_canvasHeight - m_ptRadius);

		m_ptArray[m_dragIndex].x = newX;
		m_ptArray[m_dragIndex].y = newY;

		LeaveCriticalSection(&m_csPoints);

		m_lastDragPos.x = newX;
		m_lastDragPos.y = newY;

		UpdateCoordinateLabels();
		DisplayCanvas();
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CCJH_Circle3PtAppDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_isDragging = false;
	m_dragIndex = -1;

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CCJH_Circle3PtAppDlg::OnBnClickedReset()
{
	if (m_threadActive && m_pRandomThread) {
		m_threadActive = false;
		WaitForSingleObject(m_pRandomThread->m_hThread, INFINITE);
		m_pRandomThread = nullptr;
	}

	EnterCriticalSection(&m_csPoints);
	m_clickCounter = 0;
	for (int i = 0; i < 3; i++) {
		m_pointDefined[i] = false;
		m_ptArray[i] = CPoint(0, 0);
	}
	LeaveCriticalSection(&m_csPoints);

	UpdateCoordinateLabels();
	DisplayCanvas();
}

void CCJH_Circle3PtAppDlg::OnBnClickedRandomMove()
{
	if (m_clickCounter < 3) return;
	if (m_threadActive)     return;

	m_threadActive = true;
	m_pRandomThread = AfxBeginThread(RandomMovementThread, this);
}

UINT CCJH_Circle3PtAppDlg::RandomMovementThread(LPVOID pParam)
{
	CCJH_Circle3PtAppDlg* pDlg = reinterpret_cast<CCJH_Circle3PtAppDlg*>(pParam);
	if (!pDlg) return 0;

	const int iterations = 10;      // 총 10번 반복
	const int sleepTime = 500;     // 초당 2회 = 500ms 간격

	RECT clientRect;
	::GetClientRect(pDlg->m_hWnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	for (int i = 0; i < iterations && pDlg->m_threadActive; i++) {
		EnterCriticalSection(&pDlg->m_csPoints);
		for (int j = 0; j < 3; j++) {
			pDlg->m_ptArray[j].x = rand() % (width - 2 * pDlg->m_ptRadius) + pDlg->m_ptRadius;
			pDlg->m_ptArray[j].y = rand() % (height - 2 * pDlg->m_ptRadius) + pDlg->m_ptRadius;
		}
		LeaveCriticalSection(&pDlg->m_csPoints);
		pDlg->PostMessage(WM_UPDATE_DRAW, 0, 0);
		Sleep(sleepTime);
	}

	pDlg->m_threadActive = false;
	return 0;
}

LRESULT CCJH_Circle3PtAppDlg::OnUpdateCanvas(WPARAM wParam, LPARAM lParam)
{
	UpdateCoordinateLabels();

	DisplayCanvas();
	return 0;
}

void CCJH_Circle3PtAppDlg::OnEnChangePtRadius()
{
	UpdateData(TRUE);

	if (m_ptRadius < 1)      m_ptRadius = 1;
	else if (m_ptRadius > 300) m_ptRadius = 300;
	m_spinPtRadius.SetPos(m_ptRadius);

	UpdateData(FALSE);
	UpdateCoordinateLabels();
	DisplayCanvas();
}

void CCJH_Circle3PtAppDlg::OnEnChangeCircleThickness()
{
	UpdateData(TRUE);

	if (m_circleThickness < 1)       m_circleThickness = 1;
	else if (m_circleThickness > 30) m_circleThickness = 30;
	m_spinCircleThickness.SetPos(m_circleThickness);

	UpdateData(FALSE);
	UpdateCoordinateLabels();
	DisplayCanvas();
}

void CCJH_Circle3PtAppDlg::OnDeltaposSpinPtRadius(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	int oldPos = pNMUpDown->iPos;
	int delta = pNMUpDown->iDelta;
	int newPos = oldPos + delta;

	if (newPos < 1)       newPos = 1;
	else if (newPos > 300) newPos = 300;

	m_ptRadius = newPos;
	m_spinPtRadius.SetPos(m_ptRadius);

	CString strVal;
	strVal.Format(_T("%d"), m_ptRadius);
	GetDlgItem(IDC_EDIT_PT_RADIUS)->SetWindowText(strVal);

	*pResult = 1;
	DisplayCanvas();
}

void CCJH_Circle3PtAppDlg::OnDeltaposSpinCircleThickness(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	int oldPos = pNMUpDown->iPos;
	int delta = pNMUpDown->iDelta;
	int newPos = oldPos + delta;

	if (newPos < 1)       newPos = 1;
	else if (newPos > 30) newPos = 30;

	m_circleThickness = newPos;

	m_spinCircleThickness.SetPos(m_circleThickness);

	CString strVal;
	strVal.Format(_T("%d"), m_circleThickness);
	GetDlgItem(IDC_EDIT_CIRCLE_THICKNESS)->SetWindowText(strVal);

	*pResult = 1;
	DisplayCanvas();
}
