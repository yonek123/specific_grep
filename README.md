# specific_grep

The program has one obligatory parameter which is <pattern (a string)> and can have four optional parameters (if user does not specify then default values are used).


### Making and running program

To make the program use:
    make -f Makefile

To run the program use:
    ./out/specific_grep [OPTIONS]

Options:
    -d or --dir - start directory where program needs to look for files (also in subfolders); Default: current directory
    -l or --log_file - name of the log file; Default: <program name>.log
    -r or --result_file - name of the file where result is given; Default: <program name>.txt
    -t or --threads - number of threads in the pool; Default: 4


### Changelog

V1 - Receiving and processing parameters