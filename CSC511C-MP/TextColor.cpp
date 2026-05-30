#include "TextColor.h"

const std::string TextColor::Colorize(const std::string& text, const std::string& colorCode)
{
	return colorCode + text + "\033[0m";
}
