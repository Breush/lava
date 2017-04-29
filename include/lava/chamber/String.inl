namespace lava {
    template <typename T>
    String String::fromUtf8(T begin, T end)
    {
        String string;
        Utf8::toUtf32(begin, end, std::back_inserter(string.m_string));
        return string;
    }

    template <typename T>
    String String::fromUtf16(T begin, T end)
    {
        String string;
        Utf16::toUtf32(begin, end, std::back_inserter(string.m_string));
        return string;
    }

    template <typename T>
    String String::fromUtf32(T begin, T end)
    {
        String string;
        string.m_string.assign(begin, end);
        return string;
    }

    inline bool operator==(const String& left, const String& right) { return left.m_string == right.m_string; }

    inline bool operator!=(const String& left, const String& right) { return !(left == right); }

    inline bool operator<(const String& left, const String& right) { return left.m_string < right.m_string; }

    inline bool operator>(const String& left, const String& right) { return right < left; }

    inline bool operator<=(const String& left, const String& right) { return !(right < left); }

    inline bool operator>=(const String& left, const String& right) { return !(left < right); }

    inline String operator+(const String& left, const String& right)
    {
        String string = left;
        string += right;

        return string;
    }
}
