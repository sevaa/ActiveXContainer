# ActiveXContainer
This is a C++ class that implements the minimal functionality for hosting ActiveX (OLE) controls in a Win32 API dialog box or a window.

The functionality provided is the bare minimum; feel free to enhance upon it.

# Requirements
It is meant to be used in Microsoft Visual Studio - it relies on the Native COM support that it provides (`_com_ptr_t` and related API).

It doesn't require MFC or ATL; the whole point is that it provides the ActiveX containment functionality without the overhead of those.

# Installation

Get ActiveXContainer.h and ActiveXContainer.cpp from Github, add them to your Win32 C++ project.

# Usage
## Basic scenario - singleton dialog with a dialog procedure and one ActiveX control

The basic flow is:
* Include ActiveXContainer.h
* Import the control's type library
* Declare a static CActiveXContainer object in the dialog proc
* Call Create() in WM_INITDIALOG
* Query the control's own interface from the container object and call its methods as needed
* Call Destroy() in WM_DESTROY

Specifically, it might go something like this (error handling omitted):

```
#include "ActiveXContainer.h"
#import "libid:eab22ac0-30c1-11cf-a7eb-0000c05bae0b" //That's WebBrowser, namespace SHDocVw

INT_PTR CALLBACK MyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CActiveXContainer TheBrowser;
    RECT rc;
    switch (uMsg)
    {
    case WM_INITDIALOG:
        // We'll hardcode size in dialog units, then convert to pixels
        rc.left = rc.top = 7;
        rc.right = rc.bottom = 200;
        MapDialogRect(hDlg, &rc);

        // And create/activate the browser
        TheBrowser.Create(__uuidof(SHDocVw::WebBrowser), hDlg, rc);

        // Query the browser's own interface and use it
        SHDocVw::IWebBrowser2Ptr(TheBrowser.GetObject())->Navigate(L"https://cisco.com/");
        return 1;
    case WM_DESTROY:
        TheBrowser.Destroy();
        break;
    }
    return 0;
}
```
## Other scenarios
The lifetime of this class is not supposed to be managed by COM means. You can use it statically, or inside a C++ smart pointer with creation in constructor. The latter might be more natural if you have an object wrapper for your dialogs.

# About events
The class contains a couple of helper methods for connecting event sinks, but it doesn't keep track of them and doesn't disconnect the sinks on destruction.

If the event interface of your control is a dispinterface (quite likely), it's on you to do the legwork of translating Invoke() calls into user friendly method invokations.

C++'s `#import` might have done that bit, but they don't. ATL and MFC, as far as I remember, have that logic, but it's quite heavy; if all you need is a couple of events, doing it by hand might be just easier.

# Caveats and restrictions
The class does not interact with the Win32 dialog editor of Visual Studio. The control rect has to be calculated or hardcoded in the code. The class does not support initializing controls from streams, storages, or property bags as created at design time (it might be a good idea though).

It does not support windowless activation.


