#include <iostream>
#include <string>
#include <cassert>
#include <fstream>

#include "common.hpp"
#include "wav.hpp"
#include "endpointing.hpp"
#include "database.hpp"
#include "record.hpp"
#include "lpc.hpp"

#include "gui.hpp"
#include "diff_diagram.hpp"

using namespace wordalyzer;
using namespace std;

enum CommandType {
    CMD_DB_LIST,
    CMD_DB_ADD,
    CMD_DB_REMOVE,
    CMD_DIFF
};

struct duration_t {
    int n;
    bool is_ms;
};

// Config options
CommandType command;
string db_name = "lpc.db";
string clip_name = "";

// db add
duration_t window_size = { 1024, false };
duration_t window_stride = { 512, false };
WindowFunction window_fn = WINDOW_HANN;
bool source_wav = false;
string source_filename = "";
int vector_size = 16;

// diff
string diff_clip_1 = "";
string diff_clip_2 = "";
int word_idx_1 = 0;
int word_idx_2 = 0;
int vector_offset_1 = 0;
int vector_offset_2 = 0;
int vector_count = 0;

void print_usage(string program_name)
{
    string lines[] = {
        "Usage: " + program_name + " <command>",
        "",
        "   <command> is one of:",
        "",
        "       db list [db_opts]",
        "           list all clips in the database",
        "",
        "       db add [db_opts] <name> <source> [source_opts]",
        "           add a clip to the database",
        "",
        "       db remove [db_opts] <name>",
        "           remove a clip from the database",
        "",
        "       diff [db_opts] <start_vector_1> <start_vector_2> <count>",
        "           shows a diff between word vectors, given the words to test,",
        "           offsets (in windows) within those words, the number of succeeding",
        "           vectors to test, and shows the diagram in a window",
        "",
        "   <source> is one of:",
        "       wav=<filename>: use a .wav file as a source",
        "       record: record from the microphone",
        "",
        "   <start_vector> is <clip>:<word_index>[:offset]:",
        "       clip: name of the clip",
        "       word_index: zero-based index of the word within the clip to analyze",
        "       offset: offset, in windows, from the beginning of the word where to start ",
        "               the comparison.",
        "",
        "   [source_opts] is zero or more of:",
        "       -p <vector_size>: create vectors of a given size (default: 16)",
        "       -w <window_size>: use windows of a given size (default: 1024)",
        "       -s <window_stride>: use a given stride (space between window centers) (default: 512)",
        "       -f <hamming|hann|none>: use a given window function (default: hann)",
        "",
        "       (All sizes can be also given with a suffix of 'ms' to interpret them as",
        "        milliseconds instead of samples.)",
        "",
        "   [db_opts] is zero or more of:",
        "       -d <file>: use <file> as the database (default: lpc.db)",
        ""
    };

    for (size_t i = 0; i < sizeof(lines) / sizeof(string); i++) {
        cout << lines[i] << endl;
    }
}

class command_line_exception : public exception
{
private:
    string message;

public:
    command_line_exception(const string& _message) : message(_message) {}

    const char* what() const throw()
    {
        return message.c_str();
    }
};

int parse_db_opts(int argc, char* argv[])
{
    if (argc < 2) {
        return 0;
    }

    if (string(argv[0]) != "-d") {
        return 0;
    }

    db_name = argv[1];
    return 2;
}

void parse_source(const string& s)
{
    if (starts_with(s, "wav=")) {
        source_wav = true;
        source_filename = s.substr(string("wav=").length());
    } else if (s == "record") {
        source_wav =  false;
    } else {
        throw command_line_exception("Invalid source specification: `" + s + "`");
    }
}

duration_t parse_duration(const string& s)
{
    if (ends_with(s, "ms")) {
        return { string_to_integer(s.substr(0, s.length() - string("ms").length())), true };
    } else {
        return { string_to_integer(s), false };
    }
}

int duration_to_samples(const audio_t& audio, duration_t duration)
{
    if (duration.is_ms) {
        return audio.ms_to_samples(duration.n);
    } else {
        return duration.n;
    }
}

void parse_start_vector(const string& option, string& clip_name, int& word_idx, int& offset)
{
    size_t colon_pos = option.find(':');
    if (colon_pos == string::npos) {
        throw command_line_exception("Invalid start vector specification: `" + option + "`");
    }

    clip_name = option.substr(0, colon_pos);
    string rest = option.substr(colon_pos + 1);
    size_t colon_pos_2 = rest.find(':');
    if (colon_pos_2 == string::npos) {
        word_idx = string_to_integer(rest);
        offset = 0;
    } else {
        word_idx = string_to_integer(rest.substr(0, colon_pos_2));
        offset = string_to_integer(rest.substr(colon_pos_2 + 1));
    }
}

void do_diff()
{
    database db(db_name);
    clip_t clip_1 = db.get_clip(diff_clip_1);
    clip_t clip_2 = db.get_clip(diff_clip_2);

    if (word_idx_1 >= clip_1.words.size()) {
        throw command_line_exception("Index " + to_string(word_idx_1) + " is out of range for clip `" + diff_clip_1 + "`");
    }

    if (word_idx_2 >= clip_2.words.size()) {
        throw command_line_exception("Index " + to_string(word_idx_1) + " is out of range for clip `" + diff_clip_2 + "`");
    }

    word_t& word_1 = clip_1.words[word_idx_1];
    word_t& word_2 = clip_2.words[word_idx_2];

    if (vector_offset_1 + vector_count > word_1.coeff_vectors.size()) {
        throw command_line_exception("Offset " + to_string(vector_offset_1) + " and count " + to_string(vector_count) + " are out of "
                "range for clip `" + diff_clip_1 + "`");
    }

    if (vector_offset_2 + vector_count > word_2.coeff_vectors.size()) {
        throw command_line_exception("Offset " + to_string(vector_offset_2) + " and count " + to_string(vector_count) + " are out of "
                "range for clip `" + diff_clip_2 + "`");
    }

    gui::diff_diagram diagram(word_1.coeff_vectors.begin() + vector_offset_1,
                              word_1.coeff_vectors.begin() + vector_offset_1 + vector_count,
                              word_2.coeff_vectors.begin() + vector_offset_2,
                              word_2.coeff_vectors.begin() + vector_offset_2 + vector_count);

    gui::diagram_window window(&diagram);
    window.start();
}

void do_db_list()
{
    database db(db_name);
    vector<string> clips = db.get_all_clip_names();
    cout << "Clips in the database:" << endl;
    if (clips.size() > 0) {
        for (const auto& s : clips) {
            cout << "\t- " << s << endl;
        }
    } else {
        cout << "\tNo clips." << endl;
    }
}

void do_db_remove()
{
    database db(db_name);
    db.remove_clip(clip_name);
}

void do_db_add()
{
    audio_t audio;
    if (source_wav) {
        std::ifstream wf(source_filename);
        audio = audio_from_wav(wf);
    } else {
        audio = record_audio();
    }

    vector<pair<int, int>> ep = compute_endpoints(audio);
    cout << "[*] Got " << ep.size() << " words:" << endl;
    for (auto p : ep) {
        cout << "[|]\t" << audio.samples_to_ms(p.first) << "ms - " << audio.samples_to_ms(p.second) << "ms" << endl;
    }

    cout << "[*] Analyzing, please wait..." << endl;
    clip_t clip;
    clip.window_size = duration_to_samples(audio, window_size);
    clip.window_stride = duration_to_samples(audio, window_stride);
    clip.vector_size = vector_size;
    clip.name = clip_name;
    for (auto p : ep) {
        clip.words.push_back(analyze_word(audio.samples.begin() + p.first,
                                          audio.samples.begin() + p.second,
                                          clip.window_size,
                                          clip.window_stride,
                                          vector_size,
                                          window_fn));
    }
    cout << "[+] Done!" << endl;

    database db(db_name);
    db.add_clip(clip);

    cout << "[*] Added clip `" << clip.name << "` with " << clip.words.size() << " words" << endl;
}

int parse_source_opts(int argc, char* argv[])
{
    if (argc == 0) {
        return 0;
    }

    if (argc % 2 != 0) {
        throw command_line_exception("Extra source options present");
    }

    for (int j = 0; j < argc; j+= 2) {
        string opt = argv[j];
        if (opt == "-p") {
            vector_size = string_to_integer(argv[j + 1]);
        } else if (opt == "-w") {
            window_size = parse_duration(argv[j + 1]);
        } else if (opt == "-s") {
            window_stride = parse_duration(argv[j + 1]);
        } else if (opt == "-f") {
            string fn = argv[j + 1];
            if (fn == "hamming") {
                window_fn = WINDOW_HAMMING;
            } else if (fn == "hann") {
                window_fn = WINDOW_HANN;
            } else if (fn == "none")  {
                window_fn = WINDOW_NONE;
            } else {
                throw command_line_exception("Unknown window type: `" + fn + "`");
            }
        } else {
            throw command_line_exception("Unknown option: `" + opt + "`");
        }
    }

    if (window_size.n <= 0) {
        throw command_line_exception("Window size must be greater than 0");
    }

    if (window_stride.n <= 0) {
        throw command_line_exception("Window stride must be greater than 0");
    }

    if (vector_size <= 0) {
        throw command_line_exception("Vector size must be greater than 0");
    }

    return argc;
}

void parse(int argc, char* argv[])
{
    string cmd1 = argv[1];
    if (cmd1 == "db") {
        if (argc < 3) {
            throw command_line_exception("Not enough arguments");
        } else {
            string cmd2 = argv[2];
            const int offset = 1 + 2;

            if (cmd2 == "add") {
                int i = offset + parse_db_opts(argc - offset, argv + offset);
                if (i > argc - 2) {
                    throw command_line_exception("Not enough arguments for 'db add'");
                }

                clip_name = argv[i++];
                parse_source(argv[i++]);
                try {
                    i = parse_source_opts(argc - i, argv + i);
                } catch (format_exception& e) {
                    throw command_line_exception(e.what());
                }

                command = CMD_DB_ADD;
            } else if (cmd2 == "remove") {
                int i = offset + parse_db_opts(argc - offset, argv + offset);

                if (i + 1 < argc) {
                    throw command_line_exception("Extra arguments for 'db remove'");
                }

                if (i > argc - 1) {
                    throw command_line_exception("Not enough arguments for 'db remove'");
                }

                clip_name = argv[i];
                command = CMD_DB_REMOVE;
            } else if (cmd2 == "list") {
                int i = 1 + 2 + parse_db_opts(argc - 1 - 2, argv + 1 + 2);
                if (i < argc) {
                    throw command_line_exception("Extra arguments for 'db list'");
                }

                command = CMD_DB_LIST;
            } else {
                throw command_line_exception("Unknown command: `" + cmd1 + " " + cmd2 + "`");
            }
        }
    } else if (cmd1 == "diff") {
        const int offset = 1 + 1;
        int i = offset + parse_db_opts(argc - offset, argv + offset);

        if (i + 3 < argc) {
            throw command_line_exception("Extra arguments for 'diff'");
        }

        if (i > argc - 3) {
            throw command_line_exception("Not enough arguments for 'diff'");
        }

        parse_start_vector(argv[i], diff_clip_1, word_idx_1, vector_offset_1);
        parse_start_vector(argv[i + 1], diff_clip_2, word_idx_2, vector_offset_2);
        vector_count = string_to_integer(argv[i + 2]);

        command = CMD_DIFF;
    } else {
        throw command_line_exception("Unknown command: `" + cmd1 + "`");
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2 || string(argv[1]) == "help") {
        print_usage(argv[0]);
        return -1;
    }

    try {
        parse(argc, argv);
    } catch (command_line_exception& e) {
        cerr << "[-] Error in command line: " << e.what() << endl << endl;
        print_usage(argv[0]);
        return -1;
    }

    try {
        switch(command) {
        case CMD_DB_LIST: do_db_list(); break;
        case CMD_DB_REMOVE: do_db_remove(); break;
        case CMD_DB_ADD: do_db_add(); break;
        case CMD_DIFF: do_diff(); break;
        default: cerr << "Unknown command"; return -2;
        }
    } catch (exception& e) {
        cerr << "[-] Error: " << e.what() << endl;
    }

    return 0;
}
