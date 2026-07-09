import os,sys,shutil,subprocess

flags:str = "-Wall -Werror -Wno-unused-variable -Wno-unused-function -O3 -lm"

def main(argv:list):
    argv.pop(0)

    compiler:str = None

    if len(argv) > 0:
        if len(argv[0]) >= 2:
            if argv[0][0:2] == '--':
                compiler = argv[0][2:]
                argv.pop(0)

    main_file_name:str = ""

    if os.name == 'posix':
        main_file_name = "guinea"
    elif os.name == 'nt':
        main_file_name = "guinea.exe"
    else:
        print("Error: Unknown OS")
        return 1

    if compiler:
        subprocess.run(f"{compiler} {flags} -o {main_file_name} src/main.c", shell=True)
    elif shutil.which("clang"):
        subprocess.run(f"clang {flags} -o {main_file_name} src/main.c", shell=True)
    elif shutil.which("gcc"):
        subprocess.run(f"gcc {flags} -o {main_file_name} src/main.c", shell=True)
    else:
        print("Error: No known compiler to use, recommended to state what compiler to use flag --<compiler name>\nnexample: py execute.py --gcc")
        return 1
    command:str = main_file_name
    for arg in argv:command += f" {arg}"
    subprocess.run(command, shell=True)
    return 0

try:
    sys.exit(main(sys.argv))
except KeyboardInterrupt:
    pass
