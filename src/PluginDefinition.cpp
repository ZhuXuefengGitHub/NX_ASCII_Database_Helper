//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <CommCtrl.h>
#include "ExplorerDialog.h"
#include "PluginInterface.h"
#include <string>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
HANDLE g_hModule;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, TEXT("Format Database"), dataFormatFix, NULL, false);
    setCommand(1, TEXT("Show Class"), showFileTree, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
// ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
// ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)"Hello, Notepad++!");

static void countTitle(const long long& line, const HWND& curSci, vector<int>& titleCnt, vector<int>& lineMax, long long& titleLen)
{
	titleLen = ::SendMessage(curSci, SCI_LINELENGTH, line - 1, 0);
	char* titleBuff = new char[titleLen];
	::memset(titleBuff, 0, titleLen);
	::SendMessage(curSci, SCI_GETLINE, line - 1, (LPARAM)titleBuff);

	int noneBlankLen = 0;
	long long titlePos = ::SendMessage(curSci, SCI_POSITIONFROMLINE, line - 1, 0);
	string newTitleLine = "";
	for (auto i = 0; i < titleLen; ++i) {
		if (titleBuff[i] != ' ') {
			newTitleLine += titleBuff[i];
		}
		else {
			if (i > 0 && titleBuff[i - 1] != ' ') {
				newTitleLine += titleBuff[i];
			}
		}
	}

	::SendMessage(curSci, SCI_DELETERANGE, titlePos, titleLen);
	::SendMessage(curSci, SCI_INSERTTEXT, titlePos, (LPARAM)newTitleLine.c_str());

	titleLen = ::SendMessage(curSci, SCI_LINELENGTH, line - 1, 0);
	//titleLen = newTitleLine.size();
	for (auto i = 0; i < titleLen-1; ++i) {
		if (newTitleLine[i] == ' ' || i == titleLen - 2) {
			if (i == titleLen - 2) ++noneBlankLen;
			titleCnt.emplace_back(noneBlankLen);
			lineMax.emplace_back(noneBlankLen);
			noneBlankLen = 0;
		}
		else {
			++noneBlankLen;
		}
	}
	delete[] titleBuff;
}

static void countData(const long long& lineBegin, const long long& lineEnd, const HWND& curSci, vector<vector<int>>& dataCnt, vector<int>& lineMax)
{
	const long long MAXLINELEN = 500;
	char* dataBuff = new char[MAXLINELEN];
	for (auto i = lineBegin; i <= lineEnd; ++i) {
		::memset(dataBuff, 0, MAXLINELEN);
		int noneBlankLen = 0, cnt = 0;
		vector<int> lineCnt;
		::SendMessage(curSci, SCI_GETLINE, i - 1, (LPARAM)dataBuff);
		long long lineLen = ::SendMessage(curSci, SCI_LINELENGTH, i - 1, 0);
		for (auto j = 0; j < lineLen-2; ++j) {
			if (dataBuff[j] == '|' || j == lineLen - 2) {
				//if (j == lineLen - 2) ++noneBlankLen;
				lineCnt.emplace_back(noneBlankLen);
				lineMax[cnt] = lineMax[cnt] > noneBlankLen ? lineMax[cnt] : noneBlankLen;
				noneBlankLen = 0;
				++cnt;
			}
			else {
				++noneBlankLen;
			}
		}
		dataCnt.emplace_back(lineCnt);
	}
}

static void fixTitle(const long long& line, HWND& curSci, const vector<int>& titleCnt, const vector<int>& lineMax, const long long& titleLen)
{
	char* fillBuff = new char[titleLen];
	::memset(fillBuff, ' ', titleLen);
	long long cur = ::SendMessage(curSci, SCI_POSITIONFROMLINE, line - 1, 0);
	for (auto i = 0; i < titleCnt.size(); ++i) {
		int blankNum = lineMax[i] - titleCnt[i];
		cur += titleCnt[i];
		::SendMessage(curSci, SCI_SETCURRENTPOS, cur, 0);
		::SendMessage(curSci, SCI_ADDTEXT, blankNum, (LPARAM)fillBuff);
		cur += blankNum;
		++cur;
	}
	delete[] fillBuff;
}

static void fixData(const long long& lineBegin, const long long& lineEnd, HWND& curSci, const vector<vector<int>>& dataCnt, const vector<int>& lineMax)
{
	const long long MAXLINELEN = 500;
	char* fillBuff = new char[MAXLINELEN];
	::memset(fillBuff, ' ', MAXLINELEN);
	long long cur = 0;
	for (auto i = lineBegin; i <= lineEnd; ++i) {
		cur = ::SendMessage(curSci, SCI_POSITIONFROMLINE, i - 1, 0) + 1;
		for (auto j = 0; j < dataCnt[i - lineBegin].size(); ++j) {
			int blankNum = lineMax[j] - dataCnt[i - lineBegin][j];
			cur += dataCnt[i - lineBegin][j];
			::SendMessage(curSci, SCI_SETCURRENTPOS, cur - 1, 0);
			::SendMessage(curSci, SCI_ADDTEXT, blankNum, (LPARAM)fillBuff);
			cur += blankNum;
			++cur;
		}
	}
	delete[] fillBuff;
}

int processFix()
{
	// Line Format:
	// #CLASS CLASSNAME
	// FORMAT LIBRF ...
	// #---------------- / OR [NONE]
	// DATA | ... / OR [NONE]
	// #END DATA

	// 取类的行号->查标题行各关键词长度+记录长度->遍历数据段+更新各字段最大长度->根据最大长度替换空格长度
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)& which);
	if (which == -1) {
		return GET_SCI_ERROR;
	}
	HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	// whether read-only
	if (::SendMessage(curScintilla, SCI_GETREADONLY, 0, 0) != 0)
	{
		return isReadOnly;
	}

	vector<int> lineMax;
	vector<int> titleCnt;
	vector<vector<int>> dataCnt;
	long long classPos = 0, lineClass = 0, endPos = 0, lineEnd = 0;
	long long titleLen = 0;

	::SendMessage(curScintilla, SCI_SETSEARCHFLAGS, (WPARAM)SCFIND_MATCHCASE, 0);
	::SendMessage(curScintilla, SCI_TARGETWHOLEDOCUMENT, 0, 0);

	// 设置正则表达式搜索标志
	::SendMessage(curScintilla, SCI_SETSEARCHFLAGS, SCFIND_REGEXP, 0);
	const char* searchText = "# *(CLASS)";
	classPos = ::SendMessage(curScintilla, SCI_SEARCHINTARGET, 6, (LPARAM)searchText) + 1;

	//classPos = ::SendMessage(curScintilla, SCI_SEARCHINTARGET, 6, (LPARAM)"# *CLASS") + 1;

	const long long MAXLINELEN = 400;
	char* splitBuff = new char[MAXLINELEN];
	::memset(splitBuff, 0, MAXLINELEN);

	while (classPos != 0) {
		lineClass = ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, classPos, 0) + 1;

		::SendMessage(curScintilla, SCI_TARGETWHOLEDOCUMENT, 0, 0);
		::SendMessage(curScintilla, SCI_SETTARGETSTART, classPos, 0);
		endPos = ::SendMessage(curScintilla, SCI_SEARCHINTARGET, 9, (LPARAM)"#END_DATA") + 1;
		lineEnd = ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, endPos, 0) + 1;

		long long dataLineStart = lineClass + 3;
		long long dataLineEnd = lineEnd - 1;
		long long titleLineStart = lineClass + 1;

		::SendMessage(curScintilla, SCI_GETLINE, titleLineStart, (LPARAM)splitBuff);
		string sp = splitBuff;
		sp = sp.substr(0, 3);
		if (sp != "#--") {
			dataLineStart = lineClass + 2;
		}

		// 处理FORMAT后面有多行#-------问题
		int curLine = titleLineStart;
		while (true)
		{
			curLine += 1;
			::SendMessage(curScintilla, SCI_GETLINE, curLine, (LPARAM)splitBuff);
			string sp = splitBuff;
			sp = sp.substr(0, 3);
			if (sp == "#--") {
				++dataLineStart;
			}
			if (sp != "#--") {
				break;
			}
		}

		countTitle(titleLineStart, curScintilla, titleCnt, lineMax, titleLen);
		countData(dataLineStart, dataLineEnd, curScintilla, dataCnt, lineMax);
		fixTitle(titleLineStart, curScintilla, titleCnt, lineMax, titleLen);
		fixData(dataLineStart, dataLineEnd, curScintilla, dataCnt, lineMax);
		//if (lineEnd - lineClass > 3) {

		//}

		//string tmp = std::to_string(dataLineStart);
		//::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
		//::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)tmp.c_str());

		::SendMessage(curScintilla, SCI_TARGETWHOLEDOCUMENT, 0, 0);
		::SendMessage(curScintilla, SCI_SETTARGETSTART, endPos, 0);

		// 设置正则表达式搜索标志
		::SendMessage(curScintilla, SCI_SETSEARCHFLAGS, SCFIND_REGEXP, 0);
		const char* searchText = "# *(CLASS)";
		classPos = ::SendMessage(curScintilla, SCI_SEARCHINTARGET, 6, (LPARAM)searchText) + 1;

		//classPos = ::SendMessage(curScintilla, SCI_SEARCHINTARGET, 6, (LPARAM)"# *CLASS") + 1;

		lineMax.clear();
		titleCnt.clear();
		dataCnt.clear();
	}

	return PROCESS_OK;
}

void dataFormatFix()
{
	int result = processFix();

	switch (result) {
	case(PROCESS_OK):
		::MessageBox(NULL, TEXT("Process complete"), TEXT("Format Process Result"), MB_OK);
		break;
	case(PROCESS_ERROR):
		::MessageBox(NULL, TEXT("Process Failed"), TEXT("Format Process Result"), MB_OK);
		break;
	case(GET_SCI_ERROR):
		::MessageBox(NULL, TEXT("Get current Sci Failed"), TEXT("Format Process Result"), MB_OK);
		break;
	case(isReadOnly):
		::MessageBox(NULL, TEXT("This file is read-only"), TEXT("Process Failed"), MB_OK);
		break;
	}
}


/* plugin by */

#include "ExplorerDialog.h"


/* global explorer params */
tExProp				exProp;

//HANDLE						hModule;

ExplorerDialog		explorerDlg;

void showFileTree(void) 
{	
	/* initial dialogs */
	explorerDlg.init((HINSTANCE)g_hModule, nppData, &exProp);
    HMENU menu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, DOCKABLE_EXPLORER_INDEX, 0); // get menu handle
    UINT state = ::GetMenuState(menu, funcItem[DOCKABLE_EXPLORER_INDEX]._cmdID, MF_BYCOMMAND);
    if (state & MF_CHECKED) {
        explorerDlg.doDialog(false);
    }
    else {
        //UpdateDocs();
        explorerDlg.doDialog();

		std::vector<int> linePositions;
		std::vector<string> classNames;
		findClassRowsAndNames(linePositions, classNames);

		std::vector<int> dataNum;
		getDataNumber(dataNum);

		explorerDlg.populateClassTree(linePositions, classNames, dataNum);
    }
}


void getDataNumber(std::vector<int>& dataNum)
{
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)& which);
	HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	vector<int> linePositions;
	vector<string> classNames;
	findClassRowsAndNames(linePositions, classNames);

	for (int classRow = 0; classRow < linePositions.size(); ++classRow)
	{
		int classStart = linePositions[classRow];
		int dataCount = 0;
		bool endDataFound = false;

		while (!endDataFound)
		{
			int lineLength = SendMessage(curScintilla, SCI_LINELENGTH, classStart, 0);
			int len = lineLength + 1;
			char* lineText = new char[len];
			::memset(lineText, 0, len);
			SendMessage(curScintilla, SCI_GETLINE, classStart, (LPARAM)lineText);

			const char* endData = "#END_DATA";
			if (strncmp(lineText, endData, strlen(endData)) == 0)
			{
				endDataFound = true;
				delete[] lineText;
				break;
			}

			const char* data = "DATA";
			if (strncmp(lineText, data, strlen(data)) == 0)
			{
				dataCount++;
			}

			delete[] lineText;
			classStart++; // Move to the next line
		}

		dataNum.push_back(dataCount);
	}
}


void findClassRowsAndNames(std::vector<int>& linePositions, std::vector<string>& classNames)
{
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)& which);
	HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
	// get the number of rows
	int totalLines = SendMessage(curScintilla, SCI_GETLINECOUNT, 0, 0);

	for (int line = 0; line < totalLines; ++line)
	{
		int lineLength = SendMessage(curScintilla, SCI_LINELENGTH, line, 0);
		int len = lineLength + 1;
		char* lineText = new char[len];
		::memset(lineText, 0, len);
		SendMessage(curScintilla, SCI_GETLINE, line, (LPARAM)lineText);

		// whether start form "CLASS " ?
		const char* prefix = "#CLASS ";
		const char* prefix1 = "# CLASS ";
		if (strncmp(lineText, prefix, strlen(prefix)) == 0)
		{
			linePositions.push_back(line);
			std::string className = lineText + strlen(prefix); // get CLASS name
			className.erase(className.find_last_not_of(" \n\r\t") + 1); // remove the space and link breaks
			classNames.push_back(className);
		}

		if (strncmp(lineText, prefix1, strlen(prefix1)) == 0)
		{
			linePositions.push_back(line);
			std::string className = lineText + strlen(prefix1); // get CLASS name
			className.erase(className.find_last_not_of(" \n\r\t") + 1); // remove the space and link breaks
			classNames.push_back(className);
		}

		delete[] lineText;
	}
}

void dumpToTargetLine(LPARAM line)
{
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)& which);
	HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
	int totalLines = SendMessage(curScintilla, SCI_GETLINECOUNT, 0, 0);

	::SendMessage(curScintilla, SCI_GOTOLINE, line, 0);
	
	int linesVisible = ::SendMessage(curScintilla, SCI_LINESONSCREEN, 0, 0);
	int firstVisibleLine = ::SendMessage(curScintilla, SCI_GETFIRSTVISIBLELINE, 0, 0);

	int linesToScroll = linesVisible / 2;
	
	if (line < firstVisibleLine + linesToScroll) {
		::SendMessage(curScintilla, SCI_LINESCROLL, 0, -linesToScroll);
	}
	else if (line > firstVisibleLine + linesToScroll)
	{
		::SendMessage(curScintilla, SCI_LINESCROLL, 0, linesToScroll);
	}
	/*
	if (line < firstVisibleLine + linesToScroll && line - linesToScroll >= 0) {
		::SendMessage(curScintilla, SCI_LINESCROLL, 0, -linesToScroll);
	}
	else if (line > firstVisibleLine + linesToScroll && line + linesToScroll <= totalLines)
	{
		::SendMessage(curScintilla, SCI_LINESCROLL, 0, linesToScroll);
	}
	*/
	::SendMessage(curScintilla, SCI_ENSUREVISIBLE, line, 0);
}
