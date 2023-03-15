#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <omp.h>
#include <filesystem>
#include <iostream>
#include <fstream> 
#include <vector> 
#include <algorithm>

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
    if ( pos != string::npos) {
        logFileName = logFileName.substr(pos+1);
        resultFileName = resultFileName.substr(pos+1);
    }
    pos = logFileName.find_last_of("."); 
    if ( pos != string::npos) {
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
            if (tmp.find_first_of("/\\:*<>?\"|") != string::npos) {
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
            if (tmp.find_first_of("/\\:*<>?\"|") != string::npos) {
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
                    cout << "Parameter '" << argv[i] << "' must be a number greater than 0." << endl;
                    return 1;
                }
            }
            threadNum = stoi(tmp);
            if (threadNum < 1) {
                cout << "Parameter '" << argv[i] << "' must be a number greater than 0." << endl;
                return 1;
            }
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

    omp_set_num_threads(threadNum);
    unsigned int searchedFiles = 0;
    unsigned int filesWithPattern = 0;
    unsigned int patternsNumber = 0;
    string resultFilePath = path + "/" + resultFileName + ".txt";
    string logFilePath = path + "/" + logFileName + ".log";

    vector<string> filesPaths;
    for (filesystem::recursive_directory_iterator i(dir), end; i != end; ++i)
        if (!is_directory(i->path()))
            filesPaths.push_back(i->path());

    string filenames[filesPaths.size()];
    vector<string> linesWithPattern[filesPaths.size()];
    vector<int> linesNumbers[filesPaths.size()];

    int threadIDs[filesPaths.size()];
    string fileInThread[filesPaths.size()];

    //recursive file search
    #pragma omp parallel for schedule(static) reduction(+:searchedFiles, patternsNumber) shared(filenames, linesWithPattern, linesNumbers, threadIDs, fileInThread, filesWithPattern) 
    for (int i = 0; i < filesPaths.size(); i++) {
        searchedFiles++;
        ifstream fileInput;
        string line;
        int index;
        unsigned int localIndex = 0;
        unsigned int currentLine = 0;
        unsigned int filesWithPatternPrivate = 0;
        
        vector<string> linesWithPatternPrivate;
        vector<int> linesNumbersPrivate;

        //open file
        fileInput.open(filesPaths[i].c_str());
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
                        fileCountedFlag = true;
                        filesWithPatternPrivate++;
                    }

                    if (!lineCountedFlag) {//makes sure line is counted only once for lines with pattern
                        linesWithPatternPrivate.push_back(line); //keeps all lines with pattern from file
                        linesNumbersPrivate.push_back(currentLine); //keeps indexes of lines with pattern
                        lineCountedFlag = true;
                    }
                    patternsNumber++;
                    index += pattern.length();
                }
            }
            fileInput.close();
        }
        linesWithPattern[i] = linesWithPatternPrivate;
        threadIDs[i] = omp_get_thread_num();
        if(filesWithPatternPrivate > 0) {
            filesWithPattern += filesWithPatternPrivate;
            filenames[i] = filesPaths[i];
            linesNumbers[i] = linesNumbersPrivate;
            auto tmpPos = filesPaths[i].find_last_of("/");
            if ( tmpPos != string::npos) {
                fileInThread[i] = filesPaths[i].substr(tmpPos+1);
            }
        }
        else {
            fileInThread[i] = "|";
        }
    }

    if(filesWithPattern > 0) {
        //sort files by how many pattern occurrences they have
        for (int i = 0; i < filesPaths.size() - 1; i++) {
            for (int j = i + 1; j < filesPaths.size(); j++) {
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
        for (int i = 0; i < filesPaths.size(); i++) {
            if(!linesNumbers[i].empty())
                for (int j = 0; j < linesWithPattern[i].size(); j++) {
                    resultFile << filenames[i] << ":" << linesNumbers[i][j] << ": " << linesWithPattern[i][j] << endl;
                }
            else
                break;
        }
        resultFile.close();

        //assign files to threads
        vector<int> threadsAssigned;
        vector<vector<string>> filesAssigned;
        for (int i = 0; i < filesPaths.size(); i++) {
            if(fileInThread[i] != "|") {
                auto it = find(threadsAssigned.begin(),threadsAssigned.end(), threadIDs[i]);
                if (it != threadsAssigned.end())
                    filesAssigned[it - threadsAssigned.begin()].push_back(fileInThread[i]);
                else {
                    threadsAssigned.push_back(threadIDs[i]);
                    filesAssigned.push_back(vector<string>());
                    filesAssigned.back().push_back(fileInThread[i]);
                }
            }
            else {
                auto it = find(threadsAssigned.begin(),threadsAssigned.end(), threadIDs[i]);
                if (it == threadsAssigned.end()) {
                    threadsAssigned.push_back(threadIDs[i]);
                    filesAssigned.push_back(vector<string>());
                }
            }
        }
        
        //sort threads by how many files they scanned
        for (int i = 0; i < filesAssigned.size() - 1; i++) {
            for (int j = i + 1; j < filesAssigned.size(); j++) {
                if (filesAssigned[i].size() < filesAssigned[j].size()) {
                    int tmpThreads = threadsAssigned[i];
                    threadsAssigned[i] = threadsAssigned[j];
                    threadsAssigned[j] = tmpThreads;

                    vector<string> tmpVectorFiles = filesAssigned[i];
                    filesAssigned[i] = filesAssigned[j];
                    filesAssigned[j] = tmpVectorFiles;
                }
            }
        }

        //open and save data to log file
        ofstream logFile;
        logFile.open (logFilePath);
        for (int i = 0; i < threadsAssigned.size(); i++) {
            logFile << threadsAssigned[i] << ":";
            for (int j = 0; j < filesAssigned[i].size(); j++) {
                logFile << filesAssigned[i][j] << ", ";
            }
            logFile << endl;
        }
        logFile.close();
    }
    
    //get time
    double end = omp_get_wtime( );
    double elapsedTime = end - start;

    cout << "Searched files: " << searchedFiles << endl;
    cout << "Files with pattern: " << filesWithPattern << endl;
    cout << "Patterns number: " << patternsNumber << endl;
    cout << "Result file: " << resultFilePath << endl;
    cout << "Log file: " << logFilePath << endl;
    cout << "Used threads: " << threadNum << endl;
    cout << "Elapsed time: " << int(elapsedTime*1000) << "ms" << endl;
    return 0;
}