
// CJH_Circle3PtApp.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CCJH_Circle3PtAppApp:
// See CJH_Circle3PtApp.cpp for the implementation of this class
//

class CCJH_Circle3PtAppApp : public CWinApp
{
public:
	CCJH_Circle3PtAppApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CCJH_Circle3PtAppApp theApp;