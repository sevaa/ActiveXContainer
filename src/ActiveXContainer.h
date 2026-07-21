/*
Seva Alekseyev, 2026
Home at https://github.com/sevaa/ActiveXContainer
*/
#pragma once

#include <windows.h>
#include <comdef.h>
#include <comdefsp.h>
#include <oleidl.h>
#include <ocidl.h>

class CActiveXContainer :
	IOleClientSite,
	IOleInPlaceSite /* and IOleWindow */,
	IOleControlSite,
	IOleInPlaceFrame /* and IOleInPlaceUIWindow */,
	IDispatch
{
private:
	HWND m_hParentWnd;
	IOleObjectPtr m_Object;
	RECT m_rc;

public:
	CActiveXContainer();

	/// <summary>
	/// Calls Create at construction time, for RAII
	/// </summary>
	/// <param name="clsid">CLSID of the ActiveX control - if #imported, use __uuidof(TheCoclass)</param>
	/// <param name="hParentWnd">HWND of the parent</param>
	/// <param name="rc">Initial position and size, in pixels</param>
	CActiveXContainer(REFCLSID clsid, HWND hParentWnd, const RECT& rc);

	/// <summary>
	/// Creates an ActiveX control instance, initializes, and activates it in
	/// the provided dialog/window, with self as site.
	/// Throws a _com_error if something goes wrong.
	/// </summary>
	/// <param name="clsid">CLSID of the ActiveX control - if #imported, use __uuidof(TheCoclass)</param>
	/// <param name="hParentWnd">HWND of the parent window</param>
	/// <param name="rc">Initial position and size of the control, in pixels</param>
	void Create(REFCLSID clsid, HWND hParentWnd, const RECT& rc);
	void Destroy() noexcept;
	~CActiveXContainer();

	/// <summary>
	/// The interface of the contained ActiveX control. Feel free to query it for the control's
	/// real automation interface.
	/// </summary>
	const IOleObjectPtr& GetObject() const noexcept { return m_Object; }

	/// <returns>The window handle of the the contained control, NULL if there isn't one</returns>
	HWND GetControlHWND() noexcept;

	/// <summary>
	/// Finds a connection point for the given IID, connects the provided sink to it,
	/// returns the cookie. The connection point is not automatically disconnected at
	/// destruction time! Throws a _com_error if the object has no connection points, or
	/// the provided IID is not supported.
	/// 
	/// If the event interface is a dispinterface, implementing Invoke() is on the caller.
	/// </summary>
	/// <param name="riid">IID of the event interface - see the control's typelib</param>
	/// <param name="pSink">An object that implements the interface (or dispinterface) of RIID</param>
	/// <returns>A DWORD cookie to be used for disconnecting the sink</returns>
	DWORD ConnectEventSink(REFIID riid, IUnknown* pSink);

	/// <summary>
	/// Disconnects a connection point for a IID and a cookie. Doesn't throw on errors.
	/// </summary>
	/// <param name="riid">IID of the event interface - see the control's typelib</param>
	/// <param name="Cookie">Cookie from a previous call to ConnectEvents</param>
	void DisconnectEventSink(REFIID riid, DWORD Cookie) noexcept;

private:
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	//IOleClientSite
	STDMETHOD(GetContainer)(_Out_ IOleContainer** ppContainer);
	STDMETHOD(GetMoniker)(_In_ DWORD dwAssign, _In_ DWORD dwWhichMoniker, _Out_ IMoniker** ppmk);
	STDMETHOD(OnShowWindow)(_In_ BOOL fShow);
	STDMETHOD(RequestNewObjectLayout)();
	STDMETHOD(SaveObject)();
	STDMETHOD(ShowObject)();

	// IOleInPlaceSite
	STDMETHOD(CanInPlaceActivate)();
	STDMETHOD(DeactivateAndUndo)();
	STDMETHOD(DiscardUndoState)();
	STDMETHOD(GetWindowContext)(
		_Out_ IOleInPlaceFrame** ppFrame,
		_Out_ IOleInPlaceUIWindow** ppDoc,
		_Out_ LPRECT lprcPosRect,
		_Out_ LPRECT lprcClipRect,
		_Inout_  LPOLEINPLACEFRAMEINFO lpFrameInfo
		);
	STDMETHOD(OnInPlaceActivate)();
	STDMETHOD(OnInPlaceDeactivate)();
	STDMETHOD(OnPosRectChange)(_In_ LPCRECT lprcPosRect);
	STDMETHOD(OnUIActivate)();
	STDMETHOD(OnUIDeactivate)(BOOL);
	STDMETHOD(Scroll)(SIZE);

	//IOleWindow
	STDMETHOD(ContextSensitiveHelp)(_In_ BOOL fEnterMode);
	STDMETHOD(GetWindow)(_Out_ HWND* phwnd);

	//IOleControlSite
	STDMETHOD(GetExtendedControl)(_Out_ IDispatch** ppDisp);
	STDMETHOD(LockInPlaceActive)(_In_ BOOL fLock);
	STDMETHOD(OnControlInfoChanged)();
	STDMETHOD(OnFocus)(_In_ BOOL fGotFocus);
	STDMETHOD(ShowPropertyFrame)();
	STDMETHOD(TransformCoords)(
		_Inout_ POINTL* pPtlHimetric,
		_Inout_ POINTF* pPtfContainer,
		_In_     DWORD  dwFlags
		);
	STDMETHOD(TranslateAccelerator)(_In_ MSG* pMsg, _In_ DWORD grfModifiers);

	//IOleInPlaceFrame
	STDMETHOD(EnableModeless)(_In_ BOOL fEnable);
	STDMETHOD(InsertMenus)(_In_ HMENU hmenuShared, _Inout_ LPOLEMENUGROUPWIDTHS lpMenuWidths);
	STDMETHOD(RemoveMenus)(_In_ HMENU hmenuShared);
	STDMETHOD(SetMenu)(_In_ HMENU hmenuShared, _In_ HOLEMENU holemenu, _In_ HWND     hwndActiveObject);
	STDMETHOD(SetStatusText)(_In_ LPCOLESTR pszStatusText);
	STDMETHOD(TranslateAccelerator)(_In_ LPMSG lpmsg, _In_ WORD  wID);

	//IOleInPlaceUIWindow
	STDMETHOD(GetBorder)(_Out_ LPRECT lprectBorder);
	STDMETHOD(RequestBorderSpace)(_In_ LPCBORDERWIDTHS pborderwidths);
	STDMETHOD(SetActiveObject)(_In_ IOleInPlaceActiveObject* pActiveObject, _In_ LPCOLESTR pszObjName);
	STDMETHOD(SetBorderSpace)(_In_ LPCBORDERWIDTHS pborderwidths);

	//IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
	STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, UINT, LCID, DISPID*);
	STDMETHOD(Invoke)(
		_In_ DISPID dispid,
		_In_ REFIID riid,
		_In_ LCID lcid,
		_In_ WORD f,
		_In_ DISPPARAMS* pDispParams,
		_Out_opt_  VARIANT* pvr,
		_Out_opt_  EXCEPINFO* pExcepInfo,
		_Out_opt_  UINT* puArgErr);
};

