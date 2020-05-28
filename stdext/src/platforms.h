/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Platform-specific definitions and macros
 */
#pragma once

#if defined(WIN32) || defined(WIN64)
    #define __STDEXT_WINDOWS
#elif defined(__linux__)
    #define __STDEXT_LINUX
#else
    #error Unsupported platform
#endif

#if defined(__STDEXT_WINDOWS)
    #define __STDEXT_WCHAR_SIZE 2
#elif defined(__STDEXT_LINUX)
    #define __STDEXT_WCHAR_SIZE 4
#else
    #error wchar_t size unknown
#endif

namespace stdext
{
    /*
     * Refers to https://en.wikipedia.org/wiki/Endianness
     */
    struct endianess
    {
        enum class byte_order
        {
            big_endian, // places the most significant byte first 
            little_endian,
        };
        inline static byte_order invert_byte_order(const byte_order order)
        {
            return order == byte_order::big_endian ? byte_order::little_endian : byte_order::big_endian;
        }
        static byte_order platform_value()
        {
            static byte_order_detector detector; // constructor runs once, when someone first needs it 
            return detector.m_value;
        }
        inline static byte_order inverse_platform_value() { return invert_byte_order(platform_value()); }

    private:
        class byte_order_detector
        {
        public:
            byte_order_detector()
            {
                char32_t value = 0x12345678u;
                unsigned char* p = (unsigned char*)&value;
                if (p[0] == 0x12)
                    m_value = byte_order::big_endian;
                else
                    m_value = byte_order::little_endian;
            }
            byte_order m_value;
        };
    };

}