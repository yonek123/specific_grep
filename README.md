# specific_grep

The program has one obligatory parameter which is <pattern (a string)> and can have four optional parameters (if user does not specify then default values are used). Makefile designated for Linux use.


## Making and running program

To make the program use:
    make -f Makefile

To run the program use:
    specific_grep [OPTIONS]... "<pattern>"
where <pattern> is a string you are looking for in files

Options:
    -d or --dir - start directory where program needs to look for files (also in subfolders); Default: current directory
    -l or --log_file - name of the log file; Default: <program name>.log
    -r or --result_file - name of the file where result is given; Default: <program name>.txt
    -t or --threads - number of threads in the pool; Default: 4


## Changelog

V1 - Receiving and processing parameters

V2 - Get current directory, console output, information on console how to use program, recursive file search

V3 - Searching for pattern in files and lines, sort files by how many pattern occurrences they have, save data to result file; tl;dr everything except threads and log

V4 - Usage of threads pool with OpenMP, save thread usage to log file