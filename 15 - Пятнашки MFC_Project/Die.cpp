#include "stdafx.h"
#include "Die.h"
#include <sstream>


Die::Die()
{
}


Die::~Die()
{
}


/// @brief set new value
/// always return value which was before SetValue call
///  @note  Value will not be changed if 
///    aNewValue is not in [0..15]
///
int Die::SetValue(const int aNewValue)
{
	auto returnValue = m_value;
	if (aNewValue >= 0 && aNewValue <= 15)
	{
		std::basic_stringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > ss;
		ss << aNewValue;

		m_value = aNewValue;
		if (m_button != nullptr)
		{
			::SetWindowText(m_button, ss.str().c_str());
			::ShowWindow(m_button, aNewValue != 0 ? SW_SHOW : SW_HIDE);
		}
	}
	return returnValue;
}

///@brief set the handle of our window
void Die::SetOurView(const HWND aView, const int myID)
{
	m_myID = myID;
	m_button = aView;
}


