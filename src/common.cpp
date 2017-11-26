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
