/*****************************************************************************
                    The Dark Mod GPL Source Code

 This file is part of the The Dark Mod Source Code, originally based
 on the Doom 3 GPL Source Code as published in 2011.

 The Dark Mod Source Code is free software: you can redistribute it
 and/or modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of the License,
 or (at your option) any later version. For details, see LICENSE.TXT.

 Project: The Dark Mod (http://www.thedarkmod.com/)

 $Revision$ (Revision of last commit)
 $Date$ (Date of last commit)
 $Author$ (Author of last commit)

******************************************************************************/

#include "precompiled_engine.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile( "$Id$" );

#include "qe3.h"
#include "Radiant.h"
#include "ZWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZWnd
IMPLEMENT_DYNCREATE( CZWnd, CWnd );

CZWnd::CZWnd() {
	m_pZClip = NULL;
}

CZWnd::~CZWnd() {
}

BEGIN_MESSAGE_MAP( CZWnd, CWnd )
	//{{AFX_MSG_MAP(CZWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_PAINT()
	ON_WM_GETMINMAXINFO()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_NCCALCSIZE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZWnd message handlers

int CZWnd::OnCreate( LPCREATESTRUCT lpCreateStruct ) {
	if( CWnd::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
	}
	m_dcZ = ::GetDC( GetSafeHwnd() );
	QEW_SetupPixelFormat( m_dcZ, false );
	m_pZClip = new CZClip();
	return 0;
}

void CZWnd::OnDestroy() {
	if( m_pZClip ) {
		delete m_pZClip;
		m_pZClip = NULL;
	}
	CWnd::OnDestroy();
}

void CZWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	g_pParentWnd->HandleKey( nChar, nRepCnt, nFlags );
}

void CZWnd::OnLButtonDown( UINT nFlags, CPoint point ) {
	SetFocus();
	SetCapture();
	CRect rctZ;
	GetClientRect( rctZ );
	Z_MouseDown( point.x, rctZ.Height() - 1 - point.y , nFlags );
}

void CZWnd::OnMButtonDown( UINT nFlags, CPoint point ) {
	SetFocus();
	SetCapture();
	CRect rctZ;
	GetClientRect( rctZ );
	Z_MouseDown( point.x, rctZ.Height() - 1 - point.y , nFlags );
}

void CZWnd::OnRButtonDown( UINT nFlags, CPoint point ) {
	SetFocus();
	SetCapture();
	CRect rctZ;
	GetClientRect( rctZ );
	Z_MouseDown( point.x, rctZ.Height() - 1 - point.y , nFlags );
}

void CZWnd::OnPaint() {
	CPaintDC dc( this ); // device context for painting
	//if (!wglMakeCurrent(m_dcZ, m_hglrcZ))
	//if (!wglMakeCurrent(dc.m_hDC, m_hglrcZ))
	if( !wglMakeCurrent( dc.m_hDC, win32.hGLRC ) ) {
		common->Printf( "ERROR: wglMakeCurrent failed..\n " );
		common->Printf( "Please restart " EDITOR_WINDOWTEXT " if the Z view is not working\n" );
	} else {
		QE_CheckOpenGLForErrors();
		Z_Draw();
		//wglSwapBuffers(m_dcZ);
		wglSwapBuffers( dc.m_hDC );
		TRACE( "Z Paint\n" );
	}
}

void CZWnd::OnGetMinMaxInfo( MINMAXINFO FAR *lpMMI ) {
	lpMMI->ptMinTrackSize.x = ZWIN_WIDTH;
}

void CZWnd::OnMouseMove( UINT nFlags, CPoint point ) {
	CRect rctZ;
	GetClientRect( rctZ );
	float fz = z.origin[2] + ( ( rctZ.Height() - 1 - point.y ) - ( z.height / 2 ) ) / z.scale;
	fz = floor( fz / g_qeglobals.d_gridsize + 0.5 ) * g_qeglobals.d_gridsize;
	CString strStatus;
	strStatus.Format( "Z:: %.1f", fz );
	g_pParentWnd->SetStatusText( 1, strStatus );
	Z_MouseMoved( point.x, rctZ.Height() - 1 - point.y, nFlags );
}

void CZWnd::OnSize( UINT nType, int cx, int cy ) {
	CWnd::OnSize( nType, cx, cy );
	CRect rctZ;
	GetClientRect( rctZ );
	z.width = rctZ.right;
	z.height = rctZ.bottom;
	if( z.width < 10 ) {
		z.width = 10;
	}
	if( z.height < 10 ) {
		z.height = 10;
	}
	Invalidate();
}

void CZWnd::OnNcCalcSize( BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR *lpncsp ) {
	CWnd::OnNcCalcSize( bCalcValidRects, lpncsp );
}

void CZWnd::OnKillFocus( CWnd *pNewWnd ) {
	CWnd::OnKillFocus( pNewWnd );
	SendMessage( WM_NCACTIVATE, FALSE , 0 );
}

void CZWnd::OnSetFocus( CWnd *pOldWnd ) {
	CWnd::OnSetFocus( pOldWnd );
	SendMessage( WM_NCACTIVATE, TRUE , 0 );
}

void CZWnd::OnClose() {
	CWnd::OnClose();
}

void CZWnd::OnLButtonUp( UINT nFlags, CPoint point ) {
	CRect rctZ;
	GetClientRect( rctZ );
	Z_MouseUp( point.x, rctZ.bottom - 1 - point.y, nFlags );
	if( !( nFlags & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) ) ) {
		ReleaseCapture();
	}
}

void CZWnd::OnMButtonUp( UINT nFlags, CPoint point ) {
	CRect rctZ;
	GetClientRect( rctZ );
	Z_MouseUp( point.x, rctZ.bottom - 1 - point.y, nFlags );
	if( !( nFlags & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) ) ) {
		ReleaseCapture();
	}
}

void CZWnd::OnRButtonUp( UINT nFlags, CPoint point ) {
	CRect rctZ;
	GetClientRect( rctZ );
	Z_MouseUp( point.x, rctZ.bottom - 1 - point.y, nFlags );
	if( !( nFlags & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) ) ) {
		ReleaseCapture();
	}
}

BOOL CZWnd::PreCreateWindow( CREATESTRUCT &cs ) {
	WNDCLASS wc;
	HINSTANCE hInstance = AfxGetInstanceHandle();
	if( ::GetClassInfo( hInstance, Z_WINDOW_CLASS, &wc ) == FALSE ) {
		// Register a new class
		memset( &wc, 0, sizeof( wc ) );
		wc.style         = CS_NOCLOSE;// | CS_OWNDC;
		wc.lpszClassName = Z_WINDOW_CLASS;
		wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
		wc.lpfnWndProc = ::DefWindowProc;
		if( AfxRegisterClass( &wc ) == FALSE ) {
			Error( "CZWnd RegisterClass: failed" );
		}
	}
	cs.lpszClass = Z_WINDOW_CLASS;
	cs.lpszName = "Z";
	if( cs.style != QE3_CHILDSTYLE ) {
		cs.style = QE3_SPLITTER_STYLE;
	}
	return CWnd::PreCreateWindow( cs );
}

void CZWnd::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags ) {
	g_pParentWnd->HandleKey( nChar, nRepCnt, nFlags, false );
}