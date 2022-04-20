#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <wx/string.h>

std::string WxToString(wxString wx_string);
wxString StringToWxString(std::string tempString);

#endif