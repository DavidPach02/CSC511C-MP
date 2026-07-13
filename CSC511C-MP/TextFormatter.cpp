#include "TextFormatter.h"

std::string TextFormatter::GetPrintedBorder(char borderChar, size_t width) {
    std::string borderline = "";
    for(int i = 0 ; i < width; ++i) {
        borderline += borderChar;
	}

    return borderline;
}

std::string TextFormatter::GetFormattedCell(std::string text, size_t width, Alignment alignment, bool leftTruncate) {
    // Truncate text with ellipsis if it exceeds the specified width
    if (text.length() > width) {
        if (width > 3) {
            if (leftTruncate) {
                return "..." + text.substr(text.length() - (width - 3));
            }
            else {
                return text.substr(0, width - 3) + "...";
            }
        }
        else {
            return text.substr(0, width);
        }
    }

    // Pad the string to match width if it's less than the width
    std::string paddedText = text;
    if (alignment == Alignment::LEFT) {
        paddedText.append(width - text.length(), ' ');
    }
    else if (alignment == Alignment::RIGHT) {
        paddedText.insert(0, width - text.length(), ' ');
    }
    else {
        // Ex: 10 width, 7 text length -> 3 spaces to add -> 1 space on the left, 2 spaces on the right
        size_t remaining = width - text.length();
        paddedText.insert(0, std::floor(remaining / 2.0), ' ');
        paddedText.append(std::ceil(remaining / 2.0), ' ');
    }
    return paddedText;
}