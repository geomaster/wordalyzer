#include "audio.hpp"

using namespace wordalyzer;
using namespace std;

vector<byte> wordalyzer::serialize_word(const word_t& word)
{
    return vector<byte>{ 'S', 'E', 'R', 'I', 'A', 'L', 'I', 'Z', 'Y', 'Y' };
}
