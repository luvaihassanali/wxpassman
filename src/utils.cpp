#include <utils.h>

std::string WxToString(wxString wx_string)
{
	return std::string(wx_string.mb_str(wxConvUTF8));
}

wxString StringToWxString(std::string tempString)
{
	wxString myWxString(tempString.c_str(), wxConvUTF8);
	return myWxString;
}
