#include "stdafx.h"
#include "DieHolder.h"
#include "resource.h"
#include <vector>
#include <algorithm>

DieHolder::DieHolder()
{
}


DieHolder::~DieHolder()
{
}


void DieHolder::DieBuild(HWND hDlg)
{
	for (auto meter = IDC_BUTTON_0_0; meter <= IDC_BUTTON_3_3; ++meter)
	{
		auto index = meter - IDC_BUTTON_0_0;
		auto x = index % 4;
		auto y = index / 4;
		m_dies[x][y].SetOurView(::GetDlgItem(hDlg, meter), meter);
	}
}

void DieHolder::RestartGame()
{
	srand(time(NULL));
/*	std::vector<int> myvector;
	for (auto i = 0; i < 16; ++i) myvector.push_back(i);
	std::random_shuffle(myvector.begin(), myvector.end());*/
	for (auto x = 0; x < 4; ++x)
	{
		for (auto y = 0; y < 4; ++y)
		{
			m_dies[x][y].SetValue((y * 4 + x + 1) % 16); // set the text for our button
			//m_dies[x][y].SetValue(myvector[0]); 
			//myvector.erase(myvector.begin());
		}
	}
	int i = 0;
	while (i < 300)
	{
		DieMove(rand() % (IDC_BUTTON_3_3 - IDC_BUTTON_0_0 + 1) + IDC_BUTTON_0_0);
		i++;
	}
}

bool DieHolder::WinningCheck()
{
	for (auto x = 0; x < 4; ++x)
	{
		for (auto y = 0; y < 4; ++y)
		{
			if (m_dies[x][y].GetValue() != (y * 4 + x + 1) % 16) return false;
		}
	}
	return true;
}


bool DieHolder::DieMove(int wndID)
{
	/// 1) find line where we press
	/// 2) find column if line was not founded

	POINT nullWhere{ -1, -1 };
	Die* pNullElement = nullptr; // where 0 Die (hided die now)
	POINT pressedWhere{ -1, -1 };
	Die* pPressed = nullptr; // we press this die
	for (auto x = 0; x < 4; ++x)
	{
		for (auto y = 0; y < 4; ++y)
		{
			if (m_dies[x][y].SetValue(-20) == 0)
			{
				pNullElement = &m_dies[x][y];
				nullWhere.x = x;
				nullWhere.y = y;
			}
			if (m_dies[x][y].GetID() == wndID)
			{
				pPressed = &m_dies[x][y];
				pressedWhere.x = x;
				pressedWhere.y = y;
			}
		}
	}

	if (pPressed == nullptr || nullptr == pNullElement)
	{
		return false;
	}
	if (pPressed == pNullElement)
	{
		return false;
	}
	if (pressedWhere.x != nullWhere.x && pressedWhere.y != nullWhere.y)
	{
		return false;
	}

	auto swapF = [this](int x1, int y1, int x2, int y2) // swap die values
	{
		auto v1 = m_dies[x1][y1].SetValue(-20);
		auto v2 = m_dies[x2][y2].SetValue(-20);
		m_dies[x2][y2].SetValue(v1);
		m_dies[x1][y1].SetValue(v2);
	};

	int dx = 0;
	int dy = 0;
	if (pressedWhere.x == nullWhere.x)
	{
		dy = (nullWhere.y < pressedWhere.y) ? 1 : -1;
	}
	else
	{
		dx = (nullWhere.x < pressedWhere.x) ? 1 : -1;
	}

	int restrictor = 5;
	while ((nullWhere.y != pressedWhere.y || nullWhere.x != pressedWhere.x) && restrictor-- > 0)
	{
		swapF(nullWhere.x, nullWhere.y, nullWhere.x + dx, nullWhere.y + dy);
		nullWhere.y += dy;
		nullWhere.x += dx;
	}


	return true;
}

