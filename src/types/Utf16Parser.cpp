/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#include "precomp.h"

#include "inc/Utf16Parser.hpp"

#include <bitset>

static constexpr unsigned short IndicatorBitCount = 6;
static constexpr unsigned short WcharShiftAmount = sizeof(wchar_t) * 8 - IndicatorBitCount;
static constexpr std::bitset<IndicatorBitCount> LeadingSurrogateMask = { 54 };  // 110 110 indicates a leading surrogate
static constexpr std::bitset<IndicatorBitCount> TrailingSurrogateMask = { 55 }; // 110 111 indicates a trailing surrogate

// Routine Description:
// - formats a utf16 encoded wstring and splits the codepoints into individual collections.
// - will drop badly formatted leading/trailing char sequences.
// - does not validate utf16 input beyond proper leading/trailing char sequences.
// Arguments:
// - wstr - the string to parse
// Return Value:
// - a vector of utf16 codepoints. glyphs that require surrogate pairs will be grouped
// together in a vector and codepoints that use only one wchar will be in a vector by themselves.
std::vector<std::vector<wchar_t>> Utf16Parser::Parse(std::wstring_view wstr)
{
    std::vector<std::vector<wchar_t>> result;
    std::vector<wchar_t> sequence;
    for (const auto wch : wstr)
    {
        if (_isLeadingSurrogate(wch))
        {
            sequence.clear();
            sequence.push_back(wch);
        }
        else if (_isTrailingSurrogate(wch))
        {
            if (!sequence.empty())
            {
                sequence.push_back(wch);
                result.push_back(sequence);
                sequence.clear();
            }
        }
        else
        {
            result.push_back({ wch });
        }
    }
    return result;
}

// Routine Description:
// - checks if wchar is a utf16 leading surrogate
// Arguments:
// - wch - the wchar to check
// Return Value:
// - true if wch is a leading surrogate, false otherwise
bool Utf16Parser::_isLeadingSurrogate(const wchar_t wch) noexcept
{
    const wchar_t bits = wch >> WcharShiftAmount;
    const std::bitset<IndicatorBitCount> possBits = { bits };
    return (possBits ^ LeadingSurrogateMask).none();
}

// Routine Description:
// - checks if wchar is a utf16 trailing surrogate
// Arguments:
// - wch - the wchar to check
// Return Value:
// - true if wch is a trailing surrogate, false otherwise
bool Utf16Parser::_isTrailingSurrogate(const wchar_t wch) noexcept
{
    const wchar_t bits = wch >> WcharShiftAmount;
    const std::bitset<IndicatorBitCount> possBits = { bits };
    return (possBits ^ TrailingSurrogateMask).none();
}