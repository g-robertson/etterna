﻿#include "global.h"
#include "RageUnicode.h"
#include <vector>

wchar_t const Rage::invalid_char = 0xFFFD;

std::wstring
convert_unicode_casing(std::wstring const& s, unsigned char const mapping[256]);
std::wstring
convert_unicode_casing(std::wstring const& s, unsigned char const mapping[256])
{
	std::vector<wchar_t> letters{ s.begin(), s.end() };
	for (auto& c : letters) {
		if (c < 256) {
			c = mapping[c];
		}
	}

	return std::wstring{ letters.begin(), letters.end() };
}

void
utf8_sanitize(std::string& s)
{
	std::string ret;
	for (unsigned start = 0; start < s.size();) {
		wchar_t ch;
		if (!Rage::utf8_to_wchar_ec(s, start, ch)) {
			ch = Rage::invalid_char;
		}
		Rage::wchar_to_utf8(ch, ret);
	}

	s = ret;
}

int
Rage::utf8_get_char_len(char p)
{
	if (!(p & 0x80))
		return 1; /* 0xxxxxxx - 1 */
	if (!(p & 0x40))
		return 1; /* 10xxxxxx - continuation */
	if (!(p & 0x20))
		return 2; /* 110xxxxx */
	if (!(p & 0x10))
		return 3; /* 1110xxxx */
	if (!(p & 0x08))
		return 4; /* 11110xxx */
	if (!(p & 0x04))
		return 5; /* 111110xx */
	if (!(p & 0x02))
		return 6; /* 1111110x */
	return 1;	 /* 1111111x */
}

bool
Rage::is_utf8_continuation_byte(char c)
{
	return (c & 0xC0) == 0x80;
}

bool
Rage::utf8_to_wchar_ec(std::string const& s, unsigned& start, wchar_t& ch)
{
	if (start >= s.size())
		return false;

	if (Rage::is_utf8_continuation_byte(
		  s[start]) ||			   /* misplaced continuation byte */
		(s[start] & 0xFE) == 0xFE) /* 0xFE, 0xFF */
	{
		start += 1;
		return false;
	}

	int len = Rage::utf8_get_char_len(s[start]);

	const int first_byte_mask[] = { -1, 0x7F, 0x1F, 0x0F, 0x07, 0x03, 0x01 };

	ch = wchar_t(s[start] & first_byte_mask[len]);

	for (int i = 1; i < len; ++i) {
		if (start + i >= s.size()) {
			/* We expected a continuation byte, but didn't get one. Return
			 * error, and point start at the unexpected byte; it's probably a
			 * new sequence. */
			start += i;
			return false;
		}

		char byte = s[start + i];
		if (!Rage::is_utf8_continuation_byte(byte)) {
			/* We expected a continuation byte, but didn't get one. Return
			 * error, and point start at the unexpected byte; it's probably a
			 * new sequence. */
			start += i;
			return false;
		}
		ch = (ch << 6) | (byte & 0x3F);
	}

	bool bValid = true;
	{
		unsigned c1 = (unsigned)s[start] & 0xFF;
		unsigned c2 = (unsigned)s[start + 1] & 0xFF;
		int c = (c1 << 8) + c2;
		if ((c & 0xFE00) == 0xC000 || (c & 0xFFE0) == 0xE080 ||
			(c & 0xFFF0) == 0xF080 || (c & 0xFFF8) == 0xF880 ||
			(c & 0xFFFC) == 0xFC80) {
			bValid = false;
		}
	}

	if (ch == 0xFFFE || ch == 0xFFFF) {
		bValid = false;
	}
	start += len;
	return bValid;
}

bool
Rage::utf8_to_wchar(const char* s, size_t iLength, unsigned& start, wchar_t& ch)
{
	if (start >= iLength)
		return false;

	int len = Rage::utf8_get_char_len(s[start]);

	if (start + len > iLength) {
		// We don't have room for enough continuation bytes. Return error.
		start += len;
		ch = L'?';
		return false;
	}

	switch (len) {
		case 1:
			ch = (s[start + 0] & 0x7F);
			break;
		case 2:
			ch = ((s[start + 0] & 0x1F) << 6) | (s[start + 1] & 0x3F);
			break;
		case 3:
			ch = ((s[start + 0] & 0x0F) << 12) | ((s[start + 1] & 0x3F) << 6) |
				 (s[start + 2] & 0x3F);
			break;
		case 4:
			ch = ((s[start + 0] & 0x07) << 18) | ((s[start + 1] & 0x3F) << 12) |
				 ((s[start + 2] & 0x3F) << 6) | (s[start + 3] & 0x3F);
			break;
		case 5:
			ch = ((s[start + 0] & 0x03) << 24) | ((s[start + 1] & 0x3F) << 18) |
				 ((s[start + 2] & 0x3F) << 12) | ((s[start + 3] & 0x3F) << 6) |
				 (s[start + 4] & 0x3F);
			break;

		case 6:
			ch = ((s[start + 0] & 0x01) << 30) | ((s[start + 1] & 0x3F) << 24) |
				 ((s[start + 2] & 0x3F) << 18) | ((s[start + 3] & 0x3F) << 12) |
				 ((s[start + 4] & 0x3F) << 6) | (s[start + 5] & 0x3F);
			break;
	}

	start += len;
	return true;
}

void
Rage::wchar_to_utf8(wchar_t ch, std::string& out)
{
	if (ch < 0x80) {
		out.append(1, static_cast<char>(ch));
		return;
	}

	int cbytes = 0;
	if (ch < 0x800)
		cbytes = 1;
	else if (ch < 0x10000)
		cbytes = 2;
	else if (ch < 0x200000)
		cbytes = 3;
	else if (ch < 0x4000000)
		cbytes = 4;
	else
		cbytes = 5;
	{
		int shift = cbytes * 6;
		const int init_masks[] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		out.append(1,
				   static_cast<char>(init_masks[cbytes - 1] | (ch >> shift)));
	}

	for (int i = 0; i < cbytes; ++i) {
		int shift = (cbytes - i - 1) * 6;
		out.append(1, static_cast<char>(0x80 | ((ch >> shift) & 0x3F)));
	}
}

wchar_t
Rage::utf8_get_char(std::string const& s)
{
	unsigned start = 0;
	wchar_t ret;
	if (!utf8_to_wchar_ec(s, start, ret)) {
		return Rage::invalid_char;
	}
	return ret;
}

bool
Rage::utf8_is_valid(std::string const& s)
{
	for (unsigned start = 0; start < s.size();) {
		wchar_t ch;
		if (!utf8_to_wchar_ec(s, start, ch)) {
			return false;
		}
	}
	return true;
}

void
Rage::utf8_remove_bom(std::string& sLine)
{
	if (!sLine.compare(0, 3, "\xef\xbb\xbf")) {
		sLine.erase(0, 3);
	}
}

std::wstring
Rage::make_upper(std::wstring const& s)
{
	return convert_unicode_casing(s, Rage::upperCase);
}

wchar_t
Rage::make_upper(wchar_t const& s)
{
	return Rage::make_upper(std::wstring{ s }).front();
}

std::wstring
Rage::make_lower(std::wstring const& s)
{
	return convert_unicode_casing(s, Rage::lowerCase);
}

wchar_t
Rage::make_lower(wchar_t const& s)
{
	return Rage::make_lower(std::wstring{ s }).front();
}

namespace Rage {
unsigned char upperCase[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
	0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
	0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53,
	0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B,
	0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
	0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
	0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
	0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
	0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
	0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xC0, 0xC1, 0xC2, 0xC3,
	0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xF7, 0xD8, 0xD9, 0xDA, 0xDB,
	0xDC, 0xDD, 0xDE, 0xFF
};

unsigned char lowerCase[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
	0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
	0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73,
	0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
	0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
	0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
	0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
	0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB,
	0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
	0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xC0, 0xC1, 0xC2, 0xC3,
	0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xF7, 0xD8, 0xD9, 0xDA, 0xDB,
	0xDC, 0xDD, 0xDE, 0xFF
};
} // namespace Rage