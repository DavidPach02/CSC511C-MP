#pragma once

#include <string>
#include <cmath>

class TextFormatter {
public:
	enum Alignment {
		LEFT,
		RIGHT,
		CENTER
	};

	static std::string GetPrintedBorder(char borderChar, size_t width);
	static std::string GetFormattedCell(std::string text, size_t width, TextFormatter::Alignment alignment = TextFormatter::Alignment::LEFT, bool leftTruncate = false);
};

