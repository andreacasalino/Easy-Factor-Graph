import re, os, json
import argparse
import subprocess

def thisFileFolder():
    return os.path.dirname(__file__)

class Scanner:
    def __init__(self, folder=None):
        if folder == None:
            folder = thisFileFolder()
        self.labels = {}
        for filename in os.listdir(folder):
            if Scanner.isTestSource_(filename):
                self.scanFile_(os.path.join(folder, filename))
    
    def scanFile_(self, filename):
        with open(filename, 'r') as stream:
            for expr in re.findall(r'TEST_CASE\(\"(.*?)\", \"\[(.*?)\]', stream.read()):
                label = expr[1]
                name = expr[0]
                if not label in self.labels:
                    self.labels[label] = {}
                if not filename in self.labels[label]:
                    self.labels[label][filename] = [name]
                else:
                    self.labels[label][filename].append(name)

    def isTestSource_(filename):
        return (filename.find('Test') == 0)
    
    def files(self):
        res = set()
        for _, info in self.labels.items():
            for filename in info.keys():
                res.add(filename)
        return [name for name in res]

    def get(folder):
        return Scanner(folder=folder)

class CMakeFile:
    def __init__(self, scanner):   
        self.content ='{}\n{}\n{}'.format(CMakeFile.showTarget_()\
                                         ,'\n'.join([CMakeFile.singleLabelTarget_(label, scanner.labels[label].keys()) for label in scanner.labels.keys()])\
                                         ,'\n'.join([CMakeFile.singleFileTarget_(filename) for filename in scanner.files()])\
                                         )     

    def showTarget_():
        return """
add_custom_target(__show__ ALL 
    COMMAND "{0}" "{1}"
    "-m" "show"
)""".format('${Python3_EXECUTABLE}', '${SCRIPT}')

    def singleLabelTarget_(label, files):
        return """
add_custom_target({0} ALL 
    COMMAND "{1}" "{2}"
    "-m" "run"
    "-f" "[{0}]"
    "-p" "{3}"
    SOURCES {4}
)
add_dependencies({0} Tests)""".format(label, '${Python3_EXECUTABLE}', '${SCRIPT}', '${TESTS_BIN_LOCATION}', ' '.join(files))

    def singleFileTarget_(filename):
        target_name = os.path.basename(filename)
        target_name = os.path.splitext(target_name)[0]

        return """
add_executable({0}
    {1}/../Utils.h
    {1}/../Utils.cpp
    {1}/../ModelLibrary.h
    {1}/../ModelLibrary.cpp
    {2}
)
        
target_link_libraries({0} PUBLIC
    Catch2::Catch2
    Catch2::Catch2WithMain
    EFG
)""".format(target_name, '${CMAKE_CURRENT_SOURCE_DIR}', filename)

    def __str__(self):
        return self.content

def show(args):
    print(json.dumps(Scanner.get(args.dir).labels, indent = 1))

def run(args):
    if args.filter == None:
        raise Exception('filter was not specified')
    if args.process == None:
        raise Exception('process was not specified')
    
    cmd = [args.process, args.filter]
    print('running {}'.format(' '.join(cmd)))
    hndlr = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, text=True)
    out, err = hndlr.communicate()
    print('{}{}'.format(out, err))

def terraform(args):
    if args.out == None:
        raise Exception('out dir was not specified')

    print('-- ===> terraforming into {}'.format(args.out))
    os.makedirs(args.out, exist_ok=True)
    with open(os.path.join(args.out, 'CMakeLists.txt'), 'w') as stream:   
        cmake_file = CMakeFile(Scanner.get(args.dir))
        stream.write(str(cmake_file))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--mode", default='show')
    parser.add_argument("-d", "--dir", default=None)
    parser.add_argument("-f", "--filter", default=None)
    parser.add_argument("-p", "--process", default=None)
    parser.add_argument("-o", "--out", default=None)
    args = parser.parse_args()

    if args.mode == 'show':
        show(args)    
    elif args.mode == 'run':
        run(args)    
    elif args.mode == 'terraform':
        terraform(args)
