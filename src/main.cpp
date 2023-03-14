#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <omp.h>
#include <filesystem>
#include <iostream>
#include <fstream> 
#include <vector> 

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
    unsigned int threadNum = 4;
    string logFileName = argv[0];
    string resultFileName = argv[0];
    string path = filesystem::current_path();
    auto pos = logFileName.find_last_of("/");
    if ( pos != std::string::npos) {
        logFileName = logFileName.substr(pos+1);
        resultFileName = resultFileName.substr(pos+1);
    }
    pos = logFileName.find_last_of("."); 
    if ( pos != std::string::npos) {
	logFileName = logFileName.substr(0, pos);  //default log file name without path and extension
	resultFileName = resultFileName.substr(0, pos);  //default result file name without path and extension
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
                if (isdigit(ch) == 0 || ch == '0') {
                    cout << "Parameter '" << argv[i] << "' must be a number greater than 0." << endl;
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

    unsigned int searchedFiles = 0;
    unsigned int filesWithPattern = 0;
    unsigned int patternsNumber = 0;
    string resultFilePath = path + "/" + resultFileName + ".txt";
    string logFilePath = path + "/" + logFileName + ".log";

    ifstream fileInput;
    string line;
    unsigned int currentLine;
    int index;
    vector<string> filenames;
    vector<vector<string>> linesWithPattern;
    vector<vector<int>> linesNumbers;

    //recursive file search
    for (filesystem::recursive_directory_iterator i(dir), end; i != end; ++i) {
	//check if it is file
        if (!is_directory(i->path())) {
	    //thread start
	    unsigned int localIndex = 0;
	    searchedFiles++;
	    currentLine = 0;

	    //open file
	    fileInput.open(i->path().c_str());
	    if(fileInput.is_open()) {
		bool fileCountedFlag = false; 

		//read lines		
		while(getline(fileInput, line)) {
		    currentLine++;
		    index = 0;
		    bool lineCountedFlag = false;

		    //search for pattern in line
		    while ((index = line.find(pattern, index)) != string::npos) {
			if (!fileCountedFlag) {//makes sure file is counted only once for files with pattern
			    filesWithPattern++;
			    fileCountedFlag = true;
			    //critical section
			    filenames.push_back(i->path());
			    linesWithPattern.push_back(vector<string>());
			    linesNumbers.push_back(vector<int>());
			    localIndex = filenames.size()-1;
			}

			if (!lineCountedFlag) {//makes sure line is counted only once for lines with pattern
			    linesWithPattern[localIndex].push_back(line); //keeps all lines with pattern from file
			    linesNumbers[localIndex].push_back(currentLine); //keeps indexes of lines with pattern
			    lineCountedFlag = true;
			}
			patternsNumber++;
			index += pattern.length();
		    }
		}
	    fileInput.close();
	    }
	//thread end
	}
    }

    //sort files by how many pattern occurrences they have
    for (int i = 0; i < linesWithPattern.size() - 1; i++) {
	for (int j = i + 1; j < linesWithPattern.size(); j++) {
	    if (linesWithPattern[i].size() < linesWithPattern[j].size()) {
		vector<string> tmpVectorString = linesWithPattern[i];
		linesWithPattern[i] = linesWithPattern[j];
		linesWithPattern[j] = tmpVectorString;

		vector<int> tmpVectorInt = linesNumbers[i];
		linesNumbers[i] = linesNumbers[j];
		linesNumbers[j] = tmpVectorInt;

		string tmpString = filenames[i];
		filenames[i] = filenames[j];
		filenames[j] = tmpString;
	    }
	}
    }

    //open and save data to result file
    ofstream resultFile;
    resultFile.open (resultFilePath);
    for (int i = 0; i < filenames.size(); i++) {
	for (int j = 0; j < linesWithPattern[i].size(); j++) {
	    resultFile << filenames[i] << ":" << linesNumbers[i][j] << ": " << linesWithPattern[i][j] << endl;
	}
    }
    resultFile.close();

    //get time
    double end = omp_get_wtime( );
    double elapsedTime = end - start;

    cout << "Searched files: " << searchedFiles << endl;
    cout << "Files with pattern: " << filesWithPattern << endl;
    cout << "Patterns number: " << patternsNumber << endl;
    cout << "Result file: " << resultFilePath << endl;
    cout << "Log file: " << logFilePath << endl;
    cout << "Used threads: " << threadNum << endl;
    cout << "Elapsed time: " << elapsedTime << "s" << endl;
    return 0;
}