//Add stdafx.h or pch.h here if you have a precompiled header
#include "ActiveXContainer.h"

/*
Seva Alekseyev, 2026
Home at https://github.com/sevaa/ActiveXContainer
*/

CActiveXContainer::CActiveXContainer():
	m_hParentWnd(0), m_rc(0, 0, 0, 0)
{}

CActiveXContainer::CActiveXContainer(REFCLSID clsid, HWND hParentWnd, const RECT& rc)
{
	Create(clsid, hParentWnd, rc);
}

void CActiveXContainer::Create(REFCLSID clsid, HWND hParentWnd, const RECT &rc)
{
	m_hParentWnd = hParentWnd;
	m_rc = rc;

	_com_util::CheckError(m_Object.CreateInstance(clsid));
	try
	{
		_com_util::CheckError(m_Object->SetClientSite(this));
		IPersistStreamInitPtr ipsi; //May or may not support that
		if (SUCCEEDED(m_Object.QueryInterface(__uuidof(IPersistStreamInit), &ipsi)))
		{
			_com_util::CheckError(ipsi->InitNew());
			ipsi.Release();
		}
		_com_util::CheckError(m_Object->DoVerb(OLEIVERB_INPLACEACTIVATE, 0, this, 0, m_hParentWnd, &rc));
	}
	catch (_com_error e)
	{
		m_Object.Release();
		throw e;
	}
}

DWORD CActiveXContainer::ConnectEventSink(REFIID riid, IUnknown* pSink)
{
	IConnectionPointContainerPtr cpc(m_Object);
	IConnectionPointPtr cp;
	_com_util::CheckError((cpc->FindConnectionPoint(riid, &cp)));
	DWORD Cookie;
	_com_util::CheckError(cp->Advise(pSink, &Cookie));
	return Cookie;
}

void CActiveXContainer::DisconnectEventSink(REFIID riid, DWORD Cookie) noexcept
{
	IConnectionPointContainerPtr cpc;
	IConnectionPointPtr cp;
	if (m_Object &&
		SUCCEEDED(m_Object.QueryInterface(__uuidof(IConnectionPointContainer), &cpc)) &&
		SUCCEEDED(cpc->FindConnectionPoint(riid, &cp)))
	{
		cp->Unadvise(Cookie);
	}
}

void CActiveXContainer::Destroy() noexcept
{
	if (m_Object)
	{
		{
			IOleInPlaceObjectPtr oipo;
			if (SUCCEEDED(m_Object.QueryInterface(__uuidof(IOleInPlaceObjectPtr), &oipo)))
				oipo->InPlaceDeactivate();
		}
		m_Object.Release();
	}
}

CActiveXContainer::~CActiveXContainer()
{
	Destroy();
}


/********************************************************/
// IUnnown
/********************************************************/

STDMETHODIMP CActiveXContainer::QueryInterface(REFIID riid, void** ppv)
{
	void* p = 0;
	//First line has to correspond to the first base.
	if (InlineIsEqualGUID(riid, __uuidof(IOleClientSite)) || InlineIsEqualGUID(riid, __uuidof(IUnknown)))
		p = this;
	else if (InlineIsEqualGUID(riid, __uuidof(IOleInPlaceSite)) || InlineIsEqualGUID(riid, __uuidof(IOleWindow)))
		p = (IOleInPlaceSite*)this;
	else if (InlineIsEqualGUID(riid, __uuidof(IOleControlSite)))
		p = (IOleControlSite*)this;
	else if (InlineIsEqualGUID(riid, __uuidof(IOleInPlaceFrame)) || InlineIsEqualGUID(riid, __uuidof(IOleInPlaceUIWindow)))
		p = (IOleInPlaceFrame*)this;
	else if (InlineIsEqualGUID(riid, __uuidof(IDispatch)))
		p = (IDispatch*)this;
	//else if (InlineIsEqualGUID(riid, __uuidof(IServiceProvider)))
		//p = (IServiceProvider*)this;

	if (p)
	{
		*ppv = p;
		return S_OK;
	}
	else
		return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CActiveXContainer::AddRef() { return 1; }

STDMETHODIMP_(ULONG) CActiveXContainer::Release() { return 1; }

/********************************************************/
// IOleClientSite
/********************************************************/

STDMETHODIMP CActiveXContainer::GetContainer(_Out_ IOleContainer**)
{
	return E_NOINTERFACE;
}

STDMETHODIMP CActiveXContainer::GetMoniker(_In_ DWORD, _In_ DWORD, _Out_ IMoniker**)
{
	return E_NOTIMPL;
}

STDMETHODIMP CActiveXContainer::OnShowWindow(_In_ BOOL)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::RequestNewObjectLayout()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::SaveObject()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::ShowObject()
{
	return S_OK;
}

/********************************************************/
// IOleInPlaceSite
/********************************************************/

STDMETHODIMP CActiveXContainer::CanInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::DeactivateAndUndo()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::DiscardUndoState()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::GetWindowContext(
	_Out_ IOleInPlaceFrame** ppFrame,
	_Out_ IOleInPlaceUIWindow** ppDoc,
	_Out_ LPRECT lprcPosRect,
	_Out_ LPRECT lprcClipRect,
	_Inout_  LPOLEINPLACEFRAMEINFO lpFrameInfo
)
{
	*ppFrame = this;
	*ppDoc = this;
	*lprcPosRect = 
	   *lprcClipRect = m_rc;
	return S_OK;
}

STDMETHODIMP CActiveXContainer::OnInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::OnInPlaceDeactivate()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::OnPosRectChange(_In_ LPCRECT)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::OnUIActivate()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::OnUIDeactivate(BOOL)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::Scroll(SIZE)
{
	return S_OK;
}

/********************************************************/
// IOleWindow
/********************************************************/

STDMETHODIMP CActiveXContainer::ContextSensitiveHelp(_In_ BOOL)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::GetWindow(_Out_ HWND* phwnd)
{
	*phwnd = m_hParentWnd;
	return S_OK;
}

//IOleControlSite
STDMETHODIMP CActiveXContainer::GetExtendedControl(_Out_ IDispatch** ppDisp)
{
	return E_NOTIMPL;
}

STDMETHODIMP CActiveXContainer::LockInPlaceActive(_In_ BOOL fLock)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::OnControlInfoChanged()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::OnFocus(_In_ BOOL)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::ShowPropertyFrame()
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::TransformCoords(
	_Inout_ POINTL* pPtlHimetric,
	_Inout_ POINTF* pPtfContainer,
	_In_     DWORD  dwFlags
)
{
	return E_NOTIMPL;
}

STDMETHODIMP CActiveXContainer::TranslateAccelerator(_In_ MSG* pMsg, _In_ DWORD grfModifiers)
{
	return S_OK;
}

/********************************************************/
//IOleInPlaceFrame
/********************************************************/

STDMETHODIMP CActiveXContainer::EnableModeless(_In_ BOOL)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::InsertMenus(_In_ HMENU,	_Inout_ LPOLEMENUGROUPWIDTHS)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::RemoveMenus(_In_ HMENU)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::SetMenu(_In_ HMENU, _In_ HOLEMENU, _In_ HWND)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::SetStatusText(_In_ LPCOLESTR)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::TranslateAccelerator(_In_ LPMSG lpmsg, _In_ WORD  wID)
{
	return S_OK;
}

/********************************************************/
//IOleInPlaceUIWindow
/********************************************************/

STDMETHODIMP CActiveXContainer::GetBorder(_Out_ LPRECT)
{
	return INPLACE_E_NOTOOLSPACE;
}

STDMETHODIMP CActiveXContainer::RequestBorderSpace(_In_ LPCBORDERWIDTHS)
{
	return INPLACE_E_NOTOOLSPACE;
}

STDMETHODIMP CActiveXContainer::SetActiveObject(_In_ IOleInPlaceActiveObject*, _In_ LPCOLESTR)
{
	return S_OK;
}

STDMETHODIMP CActiveXContainer::SetBorderSpace(_In_ LPCBORDERWIDTHS pborderwidths)
{
	return pborderwidths ? OLE_E_INVALIDRECT : S_OK;
}

/********************************************************/
//IDispatch
/********************************************************/

STDMETHODIMP CActiveXContainer::GetTypeInfoCount(UINT* pctinfo) { return E_NOTIMPL; }
STDMETHODIMP CActiveXContainer::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) { return E_NOTIMPL; }
STDMETHODIMP CActiveXContainer::GetIDsOfNames(REFIID, LPOLESTR*, UINT, LCID, DISPID*) { return E_NOTIMPL; }

STDMETHODIMP CActiveXContainer::Invoke(
	_In_ DISPID dispid,
	_In_ REFIID riid,
	_In_ LCID lcid,
	_In_ WORD f,
	_In_ DISPPARAMS* pDispParams,
	_Out_opt_  VARIANT* pvr,
	_Out_opt_  EXCEPINFO* pExcepInfo,
	_Out_opt_  UINT* puArgErr)
{
	switch (dispid)
	{
	case DISPID_AMBIENT_LOCALEID:
		if (pvr && f == DISPATCH_PROPERTYGET)
		{
			pvr->vt = VT_I4;
			pvr->lVal = GetThreadLocale();
		}
		break;
	case DISPID_AMBIENT_USERMODE: //Design mode vs. runtime mode
		if (pvr && f == DISPATCH_PROPERTYGET)
		{
			pvr->vt = VT_BOOL;
			pvr->boolVal = VARIANT_TRUE;
		}
		break;
	case DISPID_AMBIENT_DISPLAYASDEFAULT: //Only for controls with button semantics
		if (pvr && f == DISPATCH_PROPERTYGET)
		{
			pvr->vt = VT_BOOL;
			pvr->boolVal = VARIANT_FALSE;
		}
		break;
	default:
		return DISP_E_MEMBERNOTFOUND;
	}
	return S_OK;
}
