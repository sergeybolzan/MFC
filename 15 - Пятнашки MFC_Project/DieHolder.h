#pragma once

#include "Die.h"

class DieHolder
{
public:
	DieHolder();
	~DieHolder();

	/// doxygen style
	///@brief move the Die and analize win situation.
	/// return true if win combination
	///@param wndID - pressed window identifier
	bool DieMove(int wndID);

	///@brief initialize DieHolder.
	///@param hDlg - handle of main dialog.
	void DieBuild(HWND hDlg);

	///@brief restart the game. Check initial state of Holder for solvation.
	/// no return
	void RestartGame();
	bool WinningCheck();

protected:

	/// 1-st index - x axis
	/// 2-nd index - y axis
	Die m_dies[4][4];
};
