﻿/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_UNICODE_HPP
#define LIBBITCOIN_UNICODE_HPP

#include <string>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>
#include <bitcoin/bitcoin/utility/unicode.hpp>
#include <bitcoin/bitcoin/utility/console_streambuf.hpp>

// Regarding Unicode design for Windows:
//
// Windows and other environments, such as Java, that supported Unicode prior
// to the advent of utf8 utilize 16 bit characters. These are typically encoded
// as wchar_t in C++. Unicode no longer fits in 16 bits and as such these
// implementations now require variable length character encoding just as utf8.
//
// Libbitcoin embraces the "utf8 everywhere" design: http://utf8everywhere.org
// The objective is to use utf8 as the canonical string encoding, pushing
// wchar_t translation to the edge (stdio, argv, O/S and external API calls).
// The macro BC_USE_LIBBITCOIN_MAIN does most of the heavy lifting to ensure
// that stdio and argv is configured for utf8. The function 'to_utf' is
// provided for API translation.

// Regarding Unicode source files in VC++ builds:
//
// Don't use #pragma execution_character_set("utf-8") in Windows.
// This instruction causes sources to be explicitly interpreted as UTF8.
// However this technique improperly encodes literals ("フ" for example).
// Instead use non-BOM UTF encoded files. To do this use "save as..."
// utf8 without a "signature" (BOM) in the Visual Studio save dialog.
// This is the typical file format for C++ sources in other environments.

// Regarding Unicode in console applications:
//
// BC_USE_LIBBITCOIN_MAIN should be declared prior to bc::main() in a console
// application. This enables Unicode stdio and argument processing in Windows.
// This macro implements main() and forwards to bc::main(), which should be 
// implemented as if it was main() with the expectation that argv is utf8.
//
// Do not use cout|cerr|cin (aborts on assertion):
// std::cout << "text";
// std::cerr << "text";
// std::string text;
// std::cin >> text;
//
// Do not use the L qualifier when source is UTF-8 w/out BOM (mangles output):
// const auto utf16 = L"acción.кошка.日本国";
// std::wcout << utf16;

// Regarding use of boost::filesystem::path:
//
// If generic_wstring or c_str is used the boost::path default code page should
// be inbued with UTF8. Otherwise generic_wstring and c_str will be in error.
// This examples shows path with utf8 confiured properly:
//      using namespace boost::locale;
//      using namespace boost::filesystem;
//      std::locale::global(generator().generate(""));
//      path::imbue(std::locale());
//      const auto directory = path() / "acción" / "кошка" / "日本国";
//      auto utf16 = directory.generic_wstring();

#ifdef _MSC_VER
    #define BC_USE_LIBBITCOIN_MAIN \
        namespace libbitcoin { int main(int argc, char* argv[]); } \
        int wmain(int argc, wchar_t* argv[]) \
        { \
            bc::console_streambuf::initialize(); \
            auto buffer = libbitcoin::to_utf8(argc, argv); \
            auto args = reinterpret_cast<char**>(&buffer[0]); \
            return libbitcoin::main(argc, args); \
        }
#else
    #define BC_USE_LIBBITCOIN_MAIN \
        namespace libbitcoin { int main(int argc, char* argv[]); } \
        int main(int argc, char* argv[]) \
        { \
            return libbitcoin::main(argc, argv); \
        }
#endif

namespace libbitcoin {

/**
 * Caller should reinterpret result as:
 * auto args = reinterpret_cast<char**>(&buffer[0])
 * @param[in]  argc  The number of elements in argv.
 * @param[in]  argv  The wide command line arguments
 * @return           A buffer holding the narrow version of argv.
 */
BC_API data_chunk to_utf8(int argc, wchar_t* argv[]);

/**
 * Convert a wide (presumed UTF16) array to wide (UTF8/char).
 * @param[in]  out        The converted narrow array.
 * @param[in]  out_bytes  The allocated number of bytes in 'out'.
 * @param[in]  in         The wide character array to convert.
 * @param[in]  in_chars   The number of 'in' wide characters to convert.
 * @return                The number of bytes converted.
 */
BC_API size_t to_utf8(char out[], int out_bytes, const wchar_t in[],
    int in_chars);

/**
 * Convert a wide (presumed UTF16) string to narrow (UTF8/char).
 * @param[in]  wide  The utf16 string to convert.
 * @return           The resulting utf8 string.
 */
BC_API std::string to_utf8(const std::wstring& wide);

/**
 * Convert a narrow (presumed UTF8) array to wide (UTF16/wchar_t).
 * This is designed for buffering, where the narrow array may have been
 * truncated in the middle of a multiple byte character. The terminating
 * offset is returned in the 'unread' out parameter.
 * @param[in]  out        The converted wide array.
 * @param[in]  out_chars  The allocated number of wide characters in 'out'.
 * @param[in]  in         The narrow array to convert.
 * @param[in]  in_bytes   The number of 'in' bytes to convert.
 * @param[in]  truncated  The number of 'in' bytes [0..3] that were truncated.
 * @return                The number of characters converted.
 */
BC_API size_t to_utf16(wchar_t out[], int out_chars, const char in[],
    int in_bytes, uint8_t& truncated);

/**
 * Convert a narrow (presumed UTF8) string to wide (UTF16/wchar_t).
 * @param[in]  narrow  The utf8 string to convert.
 * @return             The resulting utf16 string.
 */
BC_API std::wstring to_utf16(const std::string& narrow);

} // namespace libbitcoin

#endif