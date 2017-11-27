#pragma once
#include <vector>
#include <string>
#include <exception>

namespace wordalyzer {
    typedef unsigned char byte;

    void serialize_size(size_t sz, std::vector<byte>& dest);
    void serialize_double(double d, std::vector<byte>& dest);

    size_t deserialize_size(std::vector<byte>::const_iterator& it);
    double deserialize_double(std::vector<byte>::const_iterator& it);

    bool starts_with(const std::string& s, const std::string& prefix);
    bool ends_with(const std::string& s, const std::string& suffix);

    class format_exception : public std::exception
    {
    private:
        std::string message;

    public:
        format_exception(const std::string& _message) : message(_message) {}

        const char* what() const throw()
        {
            return message.c_str();
        }
    };

    int string_to_integer(const std::string& s);
}
