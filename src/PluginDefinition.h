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

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

//
// All difinitions of plugin interface
//
#include "PluginInterface.h"
#include <vector>
#include <string>
using std::vector;
using std::string;

#include <commctrl.h>
#include "PluginInterface.h"
#include <tchar.h>
#include <vector>
using namespace std;

#define PROCESS_OK 0
#define PROCESS_ERROR 1
#define GET_SCI_ERROR 2
#define isReadOnly 3

#define DOCKABLE_EXPLORER_INDEX		0

typedef enum {
	DFMT_ENG,
	DFMT_GER
} eDateFmt;

typedef enum {
	SFMT_BYTES,
	SFMT_KBYTE,
	SFMT_DYNAMIC,
	SFMT_DYNAMIC_EX
} eSizeFmt;

typedef struct {
	TCHAR			szScriptName[MAX_PATH];
	TCHAR			szArguments[MAX_PATH];
} tNppExecScripts;

typedef struct {
	TCHAR			szAppName[MAX_PATH];
	TCHAR			szScriptPath[MAX_PATH];
	vector<tNppExecScripts>	vNppExecScripts;
} tNppExecProp;

typedef struct {
	/* pointer to global current path */
	TCHAR			szCurrentPath[MAX_PATH];
	UINT			iFontSize;
	INT				iSplitterPos;
	INT				iSplitterPosHorizontal;
	BOOL			bAscending;
	INT				iSortPos;
	INT				iColumnPosName;
	INT				iColumnPosExt;
	INT				iColumnPosSize;
	INT				iColumnPosDate;
	BOOL			bShowHidden;
	BOOL			bViewBraces;
	BOOL			bViewLong;
	BOOL			bAddExtToName;
	BOOL			bAutoUpdate;
	eSizeFmt		fmtSize;
	eDateFmt		fmtDate;
	vector<wstring>	vStrFilterHistory;
	wstring			strLastFilter;
	UINT			uTimeout;
	BOOL			bUseSystemIcons;
	tNppExecProp	nppExecProp;
} tExProp;

//-------------------------------------//
//-- STEP 1. DEFINE YOUR PLUGIN NAME --//
//-------------------------------------//
// Here define your plugin name
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT("NX ASCII Database Helper");

//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 2;


//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit(HANDLE hModule);

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command 
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);


//
// Your plugin command functions
//
int processFix();
void dataFormatFix();
void showFileTree(void);
void findClassRowsAndNames(std::vector<int>& linePositions, std::vector<string>& classNames);
void dumpToTargetLine(LPARAM line);
void getDataNumber(std::vector<int>& dataNum);
#endif //PLUGINDEFINITION_H
