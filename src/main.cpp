#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <omp.h>
#include <filesystem>

using namespace std;

void printInfo() {
    cout << "To run the program use:" << endl;
    cout << "    specific_grep [OPTIONS] \"<pattern>\"" << endl;
    cout << "where <pattern> is a string you are looking for in files" << endl << endl;
    cout << "Options:" << endl;
    cout << "    -d or --dir - start directory where program needs to look for files (also in subfolders); Default: current directory" << endl;
    cout << "    -l or --log_file - name of the log file; Default: <program name>.log" << endl;
    cout << "    -r or --result_file - name of the file where result is given; Default: <program name>.txt" << endl;
    cout << "    -t or --threads - number of threads in the pool; Default: 4" << endl;
}

int main(int argc, char *argv[]) {
    double start = omp_get_wtime( ); //start timer
    string pattern = "";
    string dir = filesystem::current_path(); //get current directory
    int threadNum = 4;
    string logFileName = argv[0];
    string resultFileName = argv[0];
    string path = filesystem::current_path();
    auto pos = logFileName.find_last_of("/");
    if ( pos != std::string::npos) {
        logFileName = logFileName.substr(pos+1); //default log file name
        resultFileName = resultFileName.substr(pos+1); //default result file name
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) { //custom directory
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

        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log_file") == 0) { //custom log file name
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

        else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--result_file") == 0) { //custom result file name
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

        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0) { //custom number of threads
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


	else if (pattern == "")
	    pattern = argv[i];
        
	else {
            cout << "specific_grep: invalid option -- " << argv[i] << endl;
	    printInfo();
	    return 1;
        }
    }
    if (pattern == "") {
	cout << "No pattern found!" << endl;
	printInfo();
	return 1;
    }

    int searchedFiles = 0;
    int filesWithPattern = 0;
    int patternsNumber = 0;
    string resultFile = path + "/" + resultFileName + ".txt";
    string logFile = path + "/" + logFileName + ".log";

    //recursive file search
    for (filesystem::recursive_directory_iterator i(dir), end; i != end; ++i) 
        if (!is_directory(i->path())) {
            cout << filesystem::absolute(i->path()) << endl;
	}





    //get time
    double end = omp_get_wtime( );
    double elapsedTime = end - start;

    cout << "Searched files: " << searchedFiles << endl;
    cout << "Files with pattern: " << filesWithPattern << endl;
    cout << "Patterns number: " << patternsNumber << endl;
    cout << "Result file: " << resultFile << endl;
    cout << "Log file: " << logFile << endl;
    cout << "Used threads: " << threadNum << endl;
    cout << "Elapsed time: " << elapsedTime << "s" << endl;
    return 0;
}