#ifndef EXPLORERDLG_DEFINE_H
#define EXPLORERDLG_DEFINE_H

#include "DockingFeature\DockingDlgInterface.h"
#include "PluginDefinition.h"

class ExplorerDialog : public DockingDlgInterface
{
public:
	ExplorerDialog(void);


	void init(HINSTANCE hInst, NppData nppData, tExProp* prop);
	void doDialog(bool willBeShown = true);
	void populateClassTree(std::vector<int> linePositions, std::vector<string> classNames, std::vector<int> dataNum);
	LRESULT OnNotify(NMHDR* pNMHDR, LRESULT* pResult);
	INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	/* Handles */
	NppData					_nppData;
	tTbData					_data;

	/* splitter values */
	tExProp* _pExProp;

	/* classes */
	//FileList				_FileList;
};

#endif // EXPLORERDLG_DEFINE_H