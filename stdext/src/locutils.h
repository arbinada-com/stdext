/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Locale manipulation utils
 Converter implementations
 */
#pragma once
#include <clocale>
#include <locale>
#include <string>
#include "platforms.h"

using namespace std;

namespace stdext::locutils
{
    class locale_guard
    {
    public:
        locale_guard(int category, const char* new_locale_name);
        ~locale_guard();
    private:
        int m_category;
        std::string m_old_locale_name;
    };


    /*
     * Converters
     */
    struct utf16
    {
        static const char bom_be[2];
        static const char bom_le[2];
        static const uint16_t bom_value = 0xFEFFu;
        static void add_bom(wstring& ws);
        static void add_bom(string& bytes, endianess::byte_order order);
        static bool is_bom(wchar_t c);
        static bool is_bom(char byte0, char byte1, endianess::byte_order order);
        static const uint16_t high_surrogate_min = 0xD800u;
        static const uint16_t high_surrogate_max = 0xDBFFu;
        static const uint16_t low_surrogate_min = 0xDC00u;
        static const uint16_t low_surrogate_max = 0xDFFFu;
        static const char32_t max_char = 0x10FFFF;
        static const wchar_t replacement_character = 0xFFFDu;
        static const int bytes_per_character = 2;
        //
        static bool is_high_surrogate(const wchar_t c);
        static bool is_low_surrogate(const wchar_t c);
        static bool is_noncharacter(const wchar_t c);
        static bool is_surrogate_pair(const char32_t c);
        static bool is_surrogate_pair(const wchar_t high, const wchar_t low);
        static bool to_surrogate_pair(const char32_t c, wchar_t& high, wchar_t& low);
        static bool from_surrogate_pair(wchar_t high, wchar_t low, char32_t& c);
        static std::wstring swap_byte_order(const std::wstring& ws);
        static char16_t swap_byte_order16(const char16_t c);
        static char32_t swap_byte_order32(const char32_t c);
#if __STDEXT_WCHAR_SIZE == 2
        inline static char16_t swap_byte_order(const char16_t c) { return swap_byte_order16(c); }
#else
        inline static char32_t swap_byte_order(const char32_t c) { return swap_byte_order32(c); }
#endif
        static std::string wchar_to_multibyte(const std::wstring& ws,
                                              const endianess::byte_order to_order);
        static std::string wchar_to_multibyte(const std::wstring& ws);
        static std::string wchar_to_multibyte(const wchar_t* ws, const size_t len,
                                              const endianess::byte_order to_order);
        static std::string wchar_to_multibyte(const wchar_t* ws, const size_t len);
        static bool try_detect_byte_order(const char* str, const size_t len, endianess::byte_order& order);
        // String functions
        static std::wstring to_lower(const std::wstring& ws);
        static std::wstring to_upper(const std::wstring& ws);
        static std::string to_utf8string(const std::wstring& ws);
        static bool equal_ci(const std::wstring& ws1, const std::wstring& ws2);
    };

    struct utf8
    {
        static const char bom[3];
        static void add_bom(string& bytes);
        static bool is_bom(char byte0, char byte1, char byte2);
        static const int max_seq_length = 4; // RFC 3629 limit since 2003
        // check points
        static const unsigned char chk_seq1 = 0x80;
        static const unsigned char chk_seq2 = 0xC0;
        static const unsigned char chk_seq3 = 0xE0;
        static const unsigned char chk_seq4 = 0xF0;
        static const unsigned char chk_seq5 = 0xF8;
        static const unsigned char chk_seq6 = 0xFC;
        // code points
        static const char32_t code_point1 = 0x80;
        static const char32_t code_point2 = 0x800;
        static const char32_t code_point3 = 0x10000;
        static const char32_t code_point4 = 0x200000;
        static const char32_t code_point5 = 0x4000000;
        //
        static bool is_noncharacter(const unsigned char c);
        // String functions
        static std::wstring to_utf16string(const std::string& s);
    };

    template <class CharT, CharT default_min, CharT default_max>
    class char_range_base
    {
    public:
        char_range_base() {};
        char_range_base(const CharT min, const CharT max)
            : m_min(min), m_max(max)
        { }
        char_range_base(const char_range_base&) = default;
        char_range_base& operator=(const char_range_base&) = default;
        char_range_base(char_range_base&&) = default;
        char_range_base& operator=(char_range_base&&) = default;
    public:
        bool contains(const CharT c) const noexcept { return c >= m_min && c <= m_max; }
        bool contains_all(const CharT* str, const size_t length)
        {
            for(size_t i = 0; i != length; i++)
            {
                if (!contains(str[i]))
                    return false;
            }
            return true;
        }
        bool contains_all(const std::basic_string<CharT>& str) { return contains_all(str.c_str(), str.length()); }
        CharT min() const noexcept { return m_min; }
        void min(const CharT c) noexcept { m_min = c; }
        CharT max() const noexcept { return m_max; }
        void max(const CharT c) noexcept { m_max = c; }
    protected:
        CharT m_min = default_min;
        CharT m_max = default_max;
    };

    typedef char_range_base<char, -0x7F, 0x7F> char_range;
    typedef char_range_base<unsigned char, 0x01, 0xFF> uchar_range;
    typedef char_range_base<wchar_t, 0x0001, 0xFFFF> wchar_range;


    /*
     * Converters
     */
    enum class codecvt_headers
    {
        consume,
        generate
    };

    class codecvt_mode_base
    {
    public:
        codecvt_mode_base() {}
        explicit codecvt_mode_base(const codecvt_headers headers_mode)
            : m_headers(headers_mode)
        {}
        codecvt_mode_base(const codecvt_mode_base&) = default;
        codecvt_mode_base& operator=(const codecvt_mode_base&) = default;
        codecvt_mode_base(codecvt_mode_base&&) = default;
        codecvt_mode_base& operator=(codecvt_mode_base&&) = default;
        virtual ~codecvt_mode_base() {}
    public:
        codecvt_headers headers_mode() const noexcept { return m_headers; }
        void headers_mode(const codecvt_headers value) noexcept { m_headers = value; }
        bool consume_header() const noexcept { return m_headers == codecvt_headers::consume; }
        bool generate_header() const noexcept { return m_headers == codecvt_headers::generate; }
    protected:
        codecvt_headers m_headers = codecvt_headers::consume;
    };

    enum codecvt_state : unsigned short
    {
        initial = 0,
        passed_once_or_more = 1,
    };

    class mbstate_adapter
    {
    public:
        explicit mbstate_adapter(mbstate_t& state)
            : m_state(state), m_internal(((internal_state*)&m_state))
        {}
    private:
        struct internal_state
        {
            locutils::codecvt_state codecvt;
            endianess::byte_order order;
        };
    public:
        locutils::codecvt_state codecvt_state() const { return m_internal->codecvt; }
        void codecvt_state(const locutils::codecvt_state value) { m_internal->codecvt = value; }
        endianess::byte_order byte_order() const { return m_internal->order; }
        void byte_order(const endianess::byte_order order) { m_internal->order = order; }
    private:
        mbstate_t& m_state;
        internal_state* m_internal;
    };

    /**
     * @brief The codecvt_mode_utf8 class
     * Facet class for converting between wchar_t and UTF-8 byte sequences
     * Implementation details: https://docs.microsoft.com/en-us/cpp/standard-library/codecvt-class
     *
     * UTF-8 sequence is limited by 4 bytes for support of the U+0000..U+10FFFF range
     * See RFC 3629 for more information: https://tools.ietf.org/html/rfc3629
     */
    class codecvt_mode_utf8 : public codecvt_mode_base
    {
    public:
        codecvt_mode_utf8()
            : codecvt_mode_base()
        {}
        explicit codecvt_mode_utf8(const codecvt_headers headers_mode)
            : codecvt_mode_base(headers_mode)
        {}
    };


    class codecvt_utf8_wchar_t : public codecvt<wchar_t, char, mbstate_t>
    {
    public:
        typedef codecvt<wchar_t, char, mbstate_t> codecvt_base_t;
        typedef typename codecvt_base_t::result result_t;
    public:
        explicit codecvt_utf8_wchar_t(size_t refs = 0)
            : codecvt_utf8_wchar_t(codecvt_mode_utf8(), refs)
        {}
        explicit codecvt_utf8_wchar_t(const codecvt_mode_utf8& mode, size_t refs = 0)
            : codecvt_base_t(refs), m_cvt_mode(codecvt_mode_utf8(mode))
        {}
        virtual ~codecvt_utf8_wchar_t() noexcept { }
    public:
        result_t to_utf16(mbstate_t& state, const std::string& utf8s, std::wstring& utf16s, std::size_t& converted_bytes);
        result_t to_utf16(const std::string& utf8s, std::wstring& utf16s);
        result_t to_utf8(mbstate_t& state, const std::wstring& utf16s, std::string& utf8s);
        result_t to_utf8(const std::wstring& utf16s, std::string& utf8s);
    protected:
        // codecvt implementation
        virtual result_t do_in(mbstate_t& state,
            const extern_type* first1, const extern_type* last1, const extern_type*& next1,
            intern_type* first2, intern_type* last2, intern_type*& next2) const override;
        virtual result_t do_out(mbstate_t& state,
            const intern_type* first1, const intern_type* last1, const intern_type*& next1,
            extern_type* first2, extern_type* last2, extern_type*& next2) const override;
        virtual result_t do_unshift(mbstate_t&, extern_type* first2, extern_type*, extern_type*& next2) const noexcept override;
        virtual int do_length(mbstate_t& state, const extern_type* first1, const extern_type* last1, size_t len2) const noexcept override;
        virtual bool do_always_noconv() const noexcept override { return false; }
        virtual int do_max_length() const noexcept override;
        virtual int do_encoding() const noexcept override;
    private:
        codecvt_mode_utf8 m_cvt_mode;
    };


    /*
     * codecvt_utf16_wchar_t
     * Facet class for converting between wchar_t and UTF-16 multibyte sequences
     * Implementation details: https://docs.microsoft.com/en-us/cpp/standard-library/codecvt-class
     */
    class codecvt_mode_utf16 : public codecvt_mode_base
    {
    public:
        codecvt_mode_utf16()
            : codecvt_mode_base()
        {}
        explicit codecvt_mode_utf16(const endianess::byte_order order)
            : codecvt_mode_base(), m_byte_order(order), m_is_byte_order_assigned(true)
        {}
        explicit codecvt_mode_utf16(const codecvt_headers headers_mode)
            : codecvt_mode_base(headers_mode)
        {}
        codecvt_mode_utf16(const endianess::byte_order order, const codecvt_headers headers_mode)
            : codecvt_mode_base(headers_mode), m_byte_order(order), m_is_byte_order_assigned(true)
        {}
    public:
        endianess::byte_order byte_order() const noexcept { return m_byte_order; }
        void byte_order(const endianess::byte_order value) noexcept
        {
            m_byte_order = value;
            m_is_byte_order_assigned = true;
        }
        bool is_byte_order_assigned() const noexcept { return m_is_byte_order_assigned; }
    protected:
        endianess::byte_order m_byte_order = endianess::platform_value();
        bool m_is_byte_order_assigned = false;
    };


    class codecvt_utf16_wchar_t : public codecvt<wchar_t, char, mbstate_t>
    {
    protected:
        static const int bytes_per_character = 2;
    public:
        typedef codecvt<wchar_t, char, mbstate_t> codecvt_base_t;
        typedef typename codecvt_base_t::result result_t;
    public:
        explicit codecvt_utf16_wchar_t(size_t refs = 0)
            : codecvt_utf16_wchar_t(codecvt_mode_utf16(), refs)
        { }
        explicit codecvt_utf16_wchar_t(const codecvt_mode_utf16& mode, size_t refs = 0)
            : codecvt_base_t(refs), m_cvt_mode(mode)
        { }
        virtual ~codecvt_utf16_wchar_t() noexcept { }
    public:
        result_t mb_to_utf16(mbstate_t& state, const std::string& mbs, std::wstring& ws);
        result_t mb_to_utf16(const std::string& mbs, std::wstring& ws);
        result_t utf16_to_mb(mbstate_t& state, const std::wstring& ws, std::string& mbs);
        result_t utf16_to_mb(const std::wstring& ws, std::string& mbs);
    protected:
        // codecvt implementation
        virtual result_t do_in(mbstate_t& state,
            const extern_type* first1, const extern_type* last1, const extern_type*& next1,
            intern_type* first2, intern_type* last2, intern_type*& next2) const override;
        virtual result_t do_out(mbstate_t& state,
            const intern_type* first1, const intern_type* last1, const intern_type*& next1,
            extern_type* first2, extern_type* last2, extern_type*& next2) const override;
        virtual result_t do_unshift(mbstate_t&, extern_type* first2, extern_type*, extern_type*& next2) const noexcept override;
        virtual int do_length(mbstate_t& state, const extern_type* first1, const extern_type* last1, size_t len2) const noexcept override;
        virtual bool do_always_noconv() const noexcept override { return false; }
        virtual int do_max_length() const noexcept override;
        virtual int do_encoding() const noexcept override;
    private:
        codecvt_mode_utf16 m_cvt_mode;
    };

    /*
     * codecvt_ansi_utf16_wchar_t
     * Facet class for converting between UTF-16 wchar_t and ANSI sequences
     */
    enum class ansi_encoding
    {
        by_name,
        cp1250,
        cp1251,
        cp1252
    };
    enum class ansi_encoding_naming
    {
        iconv,
        windows
    };
    std::string to_encoding_name(const ansi_encoding encoding, const ansi_encoding_naming naming);

    class codecvt_mode_ansi : public codecvt_mode_base
    {
    public:
        codecvt_mode_ansi()
            : codecvt_mode_base()
        {}
        explicit codecvt_mode_ansi(const ansi_encoding encoding)
            : codecvt_mode_base(),
              m_encoding(encoding),
              m_encoding_assigned(true)
        {}
        explicit codecvt_mode_ansi(const char* encoding_name)
            : codecvt_mode_base(),
              m_encoding(ansi_encoding::by_name),
              m_encoding_name(encoding_name),
              m_encoding_assigned(true)
        {}
        codecvt_mode_ansi(const ansi_encoding encoding, const codecvt_headers headers_mode)
            : codecvt_mode_base(headers_mode),
              m_encoding(encoding),
              m_encoding_assigned(true)
        {}
        codecvt_mode_ansi(const char* encoding_name, const codecvt_headers headers_mode)
            : codecvt_mode_base(headers_mode),
              m_encoding(ansi_encoding::by_name),
              m_encoding_name(encoding_name),
              m_encoding_assigned(true)
        {}
    public:
        inline static ansi_encoding default_encoding() noexcept { return ansi_encoding::cp1252; }
        ansi_encoding encoding() const noexcept { return m_encoding; }
        std::string encoding_name(const ansi_encoding_naming naming) const noexcept
        {
            if (m_encoding == ansi_encoding::by_name)
                return m_encoding_name;
            else
                return to_encoding_name(m_encoding, naming);
        }
        std::string encoding_name_iconv() const noexcept { return encoding_name(ansi_encoding_naming::iconv); }
        std::string encoding_name_windows() const noexcept { return encoding_name(ansi_encoding_naming::windows); }
        bool encoding_assigned() const noexcept { return m_encoding_assigned; }
    protected:
        ansi_encoding m_encoding = default_encoding();
        std::string m_encoding_name;
        bool m_encoding_assigned = false;
    };

    class codecvt_ansi_utf16_wchar_t : public codecvt<wchar_t, char, mbstate_t>
    {
    public:
        typedef codecvt<wchar_t, char, mbstate_t> codecvt_base_t;
        typedef typename codecvt_base_t::result result_t;
    public:
        explicit codecvt_ansi_utf16_wchar_t(size_t refs = 0)
            : codecvt_ansi_utf16_wchar_t(codecvt_mode_ansi(), refs)
        { }
        explicit codecvt_ansi_utf16_wchar_t(const codecvt_mode_ansi& mode, size_t refs = 0)
            : codecvt_base_t(refs), m_cvt_mode(mode)
        { }
        virtual ~codecvt_ansi_utf16_wchar_t() noexcept { }
    public:
        result_t ansi_to_utf16(mbstate_t& state, const std::string& s, std::wstring& ws) const;
        result_t ansi_to_utf16(const std::string& mbs, std::wstring& ws) const;
        result_t utf16_to_ansi(mbstate_t& state, const std::wstring& ws, std::string& s) const;
        result_t utf16_to_ansi(const std::wstring& ws, std::string& s) const;
    protected:
        // codecvt implementation
        virtual result_t do_in(mbstate_t& state,
                               const extern_type* first1, const extern_type* last1, const extern_type*& next1,
                               intern_type* first2, intern_type* last2, intern_type*& next2) const override;
        virtual result_t do_out(mbstate_t&,
                                const intern_type* first1, const intern_type* last1, const intern_type*& next1,
                                extern_type* first2, extern_type* last2, extern_type*& next2) const override;
        virtual result_t do_unshift(mbstate_t&, extern_type* first2, extern_type*, extern_type*& next2) const noexcept override;
        virtual int do_length(mbstate_t& state, const extern_type* first1, const extern_type* last1, size_t maxlen2) const noexcept override;
        virtual bool do_always_noconv() const noexcept override { return false; }
        virtual int do_max_length() const noexcept override;
        virtual int do_encoding() const noexcept override;
    private:
        codecvt_mode_ansi m_cvt_mode;
    };

}


