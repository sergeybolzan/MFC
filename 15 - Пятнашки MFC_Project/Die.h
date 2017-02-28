#pragma once


class Die
{
public:
	Die();
	~Die();

	/// @brief set new value
	/// always return value which was before SetValue call
	///  @note  Value will not be changed if 
	///    aNewValue is not in [0..15]
	///
	int SetValue(const int aNewValue);

	///@brief set the handle of our window
	void SetOurView(const HWND aView, const int myID);


	int GetID(){ return m_myID; };
	int GetValue(){ return m_value; };

protected:
	int m_myID = -1;
	int m_value = -1; // not valid at start
	HWND m_button = nullptr; /// for display the value
};

