#include "ExplorerDialog.h"
//#include "ExplorerResource.h"
#include "Scintilla.h"
#include "resource.h"
#include <WinUser.h>

#include <windowsx.h>
#include <shellapi.h>
#include <shlwapi.h>
#pragma warning(push)
#pragma warning(disable: 4091)
#include <shlobj.h>
#include "PluginDefinition.h"

ExplorerDialog::ExplorerDialog(void) : DockingDlgInterface(IDD_EXPLORER_DLG)
{

}

void ExplorerDialog::init(HINSTANCE hInst, NppData nppData, tExProp* prop)
{
	_nppData = nppData;
	DockingDlgInterface::init(hInst, nppData._nppHandle);

	_pExProp = prop;
}

void ExplorerDialog::doDialog(bool willBeShown)
{
	if (!isCreated()) // if HWND are not created
	{
		create(&_data);
		// define the default docking behaviour
		_data.uMask = DWS_DF_CONT_LEFT | DWS_ADDINFO | DWS_ICONTAB;
		_data.pszName = _T("CLASS TREE");
		_data.pszAddInfo = _pExProp->szCurrentPath;
		//_data.hIconTab = (HICON)::LoadImage(_hInst, MAKEINTRESOURCE(IDI_EXPLORE), IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
		_data.pszModuleName = getPluginFileName();
		_data.dlgID = DOCKABLE_EXPLORER_INDEX;
		::SendMessage(_hParent, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&_data);
	}
	display(willBeShown);
}

void ExplorerDialog::populateClassTree(std::vector<int> linePositions, std::vector<string> classNames, std::vector<int> dataNum) // HWND _hSelf, UINT treeCtrlID, vector<int>& linePositions, vector<string>& classNames)
{
	HWND hTreeCtrl = GetDlgItem(_hSelf, IDC_TREE1);

	// clear
	HTREEITEM thRoot = TreeView_GetRoot(hTreeCtrl);
	if (thRoot != NULL) {
		TreeView_DeleteItem(hTreeCtrl, thRoot);
	}

	// add root
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL; 
	tvInsert.hInsertAfter = TVI_ROOT;
	tvInsert.item.mask = TVIF_TEXT;
	tvInsert.item.pszText = _T("SEQUENCE:NAME(DATA NUMBER)");
	HTREEITEM hRoot = (HTREEITEM)SendMessage(hTreeCtrl, TVM_INSERTITEM, 0, (LPARAM)& tvInsert);
	// add child
	for (int i = 0; i < linePositions.size(); ++i)
	{
		tvInsert.hParent = hRoot;
		tvInsert.hInsertAfter = TVI_LAST;
		tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		//tvInsert.item.mask = TVIF_TEXT;

		std::wstring classNameWithNumber = std::to_wstring(i + 1) + L": " + std::wstring(classNames[i].begin(), classNames[i].end()) + L"(" + std::to_wstring(dataNum[i]) + L")";//std::to_wstring(linePositions[i]+1) + L")";

		tvInsert.item.pszText = const_cast<LPTSTR>(classNameWithNumber.c_str());

		//tvInsert.item.lParam = linePositions[i];

		HTREEITEM hChild = (HTREEITEM)SendMessage(hTreeCtrl, TVM_INSERTITEM, 0, (LPARAM)& tvInsert);
		TreeView_Expand(hTreeCtrl, hRoot, TVE_EXPAND);
		
		// below was no used, used if you need to use lParamID to do somethings
		TVITEM tvItem;
		tvItem.mask = TVIF_PARAM;
		tvItem.hItem = hChild;
		tvItem.lParam = linePositions[i];
		TreeView_SetItem(hTreeCtrl, &tvItem);
	}
}

INT_PTR CALLBACK ExplorerDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_NOTIFY:
		{
			//NMHDR * nmhdr = (NMHDR*)lParam;
			HWND hTreeCtrl = ::GetDlgItem(_hSelf, IDC_TREE1);
			LPNMHDR		nmhdr = (LPNMHDR)lParam;
			if (nmhdr->hwndFrom == hTreeCtrl)
			{
				//::MessageBox(NULL, TEXT("nmhdr->hwndFrom == hTreeCtrl"), TEXT("Format Process Result"), MB_OK); //
				switch (nmhdr->code)
				{
					case NM_CLICK:
					{
						std::vector<int> linePositions;
						std::vector<string> classNames;
						findClassRowsAndNames(linePositions, classNames);

						NMHDR* pNMHDR = (NMHDR*)lParam;
						POINT pt;
						GetCursorPos(&pt);
						ScreenToClient(hTreeCtrl, &pt);
						TVHITTESTINFO ht;
						ht.pt = pt;
						HTREEITEM hClickedItem = TreeView_HitTest(hTreeCtrl, &ht);

						TVITEM tvItem;
						tvItem.mask = TVIF_PARAM;
						tvItem.hItem = hClickedItem;

						if (TreeView_GetItem(hTreeCtrl, &tvItem))
						{
							LPARAM clickedLParam = tvItem.lParam;
							dumpToTargetLine(clickedLParam);
						}
						break;
					}
				}
			}
		}
		case WM_SIZE:
		{
			RECT rcClient;
			GetClientRect(_hSelf, &rcClient);
			int cxClient = rcClient.right - rcClient.left;
			int cyClient = rcClient.bottom - rcClient.top;

			HWND hTreeCtrl = GetDlgItem(_hSelf, IDC_TREE1);
			SetWindowPos(hTreeCtrl, NULL, 0, 0, cxClient, cyClient, SWP_NOZORDER);
		}
		break;
		default:
			return DockingDlgInterface::run_dlgProc(Message, wParam, lParam);
	}
	return FALSE;
}