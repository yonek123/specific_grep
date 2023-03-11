#include <iostream>
#include <cstring>
#include <sys/stat.h>

using namespace std;

int main(int argc, char *argv[]) {
    string dir = "";
    int threadNum = 4;
    string logFileName = argv[0];
    string resultFileName = argv[0];
    auto pos = logFileName.find_last_of("/");
    if ( pos != std::string::npos) {
        logFileName = logFileName.substr(pos+1);
        resultFileName = resultFileName.substr(pos+1);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if(argc < i + 2) {
                cout << "Parameter '" << argv[i] << "' requires an argument." << endl;
                return 1;
            }
            struct stat sb;
            if (stat(argv[i + 1], &sb) != 0) {
                cout << "Path '" << argv[i + 1] <<"' does not exists. Make sure you used / instead of \\." << endl;
                return 1;
            }
            dir = argv[i + 1];
            i++;
        }

        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log_file") == 0) {
            if(argc < i + 2) {
                cout << "Parameter '" << argv[i] << "' requires an argument." << endl;
                return 1;
            }
            string tmp = argv[i+1];
            if (tmp.find_first_of("/\\:*<>?\"|") != std::string::npos) {
                cout << "Log filename cannot contain these characters: /\\:*<>?\"|" << endl;
                return 1;
            }
            logFileName = tmp;
            i++;
        }

        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--result_file") == 0) {
            if(argc < i + 2) {
                cout << "Parameter '" << argv[i] << "' requires an argument." << endl;
                return 1;
            }
            string tmp = argv[i+1];
            if (tmp.find_first_of("/\\:*<>?\"|") != std::string::npos) {
                cout << "Result filename cannot contain these characters: /\\:*<>?\"|" << endl;
                return 1;
            }
            resultFileName = tmp;
            i++;
        }

        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0) {
            if(argc < i + 2) {
                cout << "Parameter '" << argv[i] << "' requires an argument." << endl;
                return 1;
            }
            string tmp = argv[i+1];
            for (char const &ch : tmp) {
                if (isdigit(ch) == 0) {
                    cout << "Parameter '" << argv[i] << "' must be a number." << endl;
                    return 1;
                }
            }
            threadNum = stoi(tmp);
            i++;
        }

        else {
            cout << "specific_grep: invalid option -- " << argv[i] << endl;
            return 1;
        }
    }
    cout << dir << endl;
    cout << logFileName << endl;
    cout << resultFileName << endl;
    cout << threadNum << endl;
    return 0;
}