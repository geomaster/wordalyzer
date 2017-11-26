#include <iostream>
#include <string>

#include "database.hpp"

using namespace wordalyzer;
using namespace std;

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
        "       diff [db_opts] <start_vector_1> <start_vector_2> <count> <outfile>",
        "           shows a diff between word vectors, given the words to test,",
        "           offsets (in windows) within those words, the number of succeeding",
        "           vectors to test, and outputs the diagram to the given output file",
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

enum CommandType {
    CMD_DB_LIST,
    CMD_DB_ADD,
    CMD_DB_REMOVE,
    CMD_DB_DIFF
};

string db_name = "lpc.db";
string clip_name = "";
int window_size = 1024;
CommandType command;

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

void parse(int argc, char* argv[])
{
    string cmd1 = argv[1];
    if (cmd1 == "db") {
        if (argc < 3) {
            throw command_line_exception("Not enough arguments");
        } else {
            string cmd2 = argv[2];
            if (cmd2 == "add") {
                cout << "add to db" << endl;
            } else if (cmd2 == "remove") {
                int i = 1 + 2 + parse_db_opts(argc - 1 - 2, argv + 1 + 2);
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
        cout << "diff me baybay" << endl;
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
        cerr << "Error in command line: " << e.what() << endl << endl;
        print_usage(argv[0]);
        return -1;
    }

    try {
        switch(command) {
        case CMD_DB_LIST: do_db_list(); break;
        case CMD_DB_REMOVE: do_db_remove(); break;
        default: cerr << "not impl'd" << endl;
        }
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
