#include "common.hpp"

using namespace wordalyzer;
using namespace std;

void wordalyzer::serialize_size(size_t sz, vector<byte>& dest)
{
    for (size_t i = 0; i < sizeof(sz); i++) {
        dest.push_back(static_cast<byte>(sz >> (8 * i)));
    }
}

void wordalyzer::serialize_double(double d, vector<byte>& dest)
{
    union {
        double d;
        byte b[sizeof(double)];
    } u;
    u.d = d;

    for (size_t i = 0; i < sizeof(d); i++) {
        dest.push_back(u.b[i]);
    }
}

size_t wordalyzer::deserialize_size(vector<byte>::const_iterator& it)
{
    size_t res = 0;
    for (size_t i = 0; i < sizeof(size_t); i++) {
        res |= static_cast<size_t>(*it++) << (8 * i);
    }

    return res;
}

double wordalyzer::deserialize_double(vector<byte>::const_iterator& it)
{
    union {
        double d;
        byte b[sizeof(double)];
    } u;

    for (size_t i = 0; i < sizeof(double); i++) {
        u.b[i] = *it++;
    }

    return u.d;
}

bool wordalyzer::starts_with(const string& s, const string& prefix)
{
    if (s.length() < prefix.length()) {
        return false;
    }

    return s.substr(0, prefix.length()) == prefix;
}

bool wordalyzer::ends_with(const string& s, const string& suffix)
{
    if (suffix.length() == 0) {
        return true;
    }

    if (s.length() < suffix.length()) {
        return false;
    }

    for (size_t ai = s.length() - suffix.length(), bi = 0; ai < s.length(); ai++, bi++) {
        if (s[ai] != suffix[bi]) {
            return false;
        }
    }

    return true;
}

int wordalyzer::string_to_integer(const string& s)
{
    int pow10 = 1, res = 0;
    for (auto it = s.rbegin(); it != s.rend(); it++) {
        if (*it < '0' || *it > '9') {
            throw format_exception("Invalid number: `" + s + "`");
        }

        res = (*it - '0') * pow10 + res;
        pow10 *= 10;
    }

    return res;
}

