#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "..\..\src\ActiveXContainer.h"
#include "resource.h"
#include <vector>

using std::vector;

INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CActiveXContainer Container;
	static RECT rc = {7, 28, 10, 40};
	static vector<CLSID> Classes;
	switch(uMsg)
	{
	case WM_INITDIALOG:
	{
		RECT crc;
		GetClientRect(hDlg, &crc);
		MapDialogRect(hDlg, &rc);
		rc.right = crc.right - rc.left;
		rc.bottom = crc.bottom - rc.top / 4;

		HKEY hk;
		if(RegOpenKeyEx(HKEY_CLASSES_ROOT, L"CLSID", 0, KEY_READ, &hk) == ERROR_SUCCESS)
		{
			wchar_t kn[200];
			DWORD i = 0, dwl;
			while(RegEnumKeyEx(hk, i++, kn, &(dwl = _countof(kn)), 0, 0, 0, 0) == ERROR_SUCCESS)
			{
				size_t knl = wcslen(kn);
				wcscpy(kn + knl, L"\\Control");
				HKEY hkc;
				if(RegOpenKeyEx(hk, kn, 0, KEY_READ, &hkc) == ERROR_SUCCESS)
				{
					RegCloseKey(hkc);
					kn[knl] = 0;
					if(RegOpenKeyEx(hk, kn, 0, KEY_READ, &hkc) == ERROR_SUCCESS)
					{
						DWORD dwt;
						wchar_t ws[200];
						if(RegQueryValueEx(hkc, L"", 0, &dwt, (LPBYTE)ws, &(dwl = sizeof(ws))) == ERROR_SUCCESS && dwt == REG_SZ && dwl <= sizeof(ws) - 2)
						{
							ws[dwl / 2] = 0;
							LRESULT cbi = SendDlgItemMessage(hDlg, IDC_CLASS, CB_ADDSTRING, 0, (LPARAM)ws);
							SendDlgItemMessage(hDlg, IDC_CLASS, CB_SETITEMDATA, cbi, Classes.size());
							CLSID clsid;
							CLSIDFromString(kn, &clsid);
							Classes.push_back(clsid);
						}
						RegCloseKey(hkc);
					}
				}
			}
			RegCloseKey(hk);
			if(Classes.size())
				SendDlgItemMessage(hDlg, IDC_CLASS, CB_SETCURSEL, 0, 0);
		}

		return 1;
	}
	case WM_COMMAND:
		if(LOWORD(wParam) == IDC_CREATE)
		{
			Container.Destroy();
			LRESULT i = SendDlgItemMessage(hDlg, IDC_CLASS, CB_GETITEMDATA, SendDlgItemMessage(hDlg, IDC_CLASS, CB_GETCURSEL, 0, 0), 0);
			try
			{
				Container.Create(Classes[i], hDlg, rc);
			}
			catch(_com_error err)
			{
				MessageBox(hDlg, L"Error", 0, 0);
			}
		}
		break;
	case WM_DESTROY:
		Container.Destroy();
		break;
	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;
	}
	return 0;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int)
{
	CoInitialize(0);

	DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDLG), NULL, MainDlgProc);

	CoUninitialize();
	return 0;
}
