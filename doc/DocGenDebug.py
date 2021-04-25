from FileHandler import *
import os
import subprocess
import shutil

class Writer:
    def __init__(self, fileName):
        self.stream = open(fileName, "w")
        
    def addLine(self, line):
        self.stream.write(line)
        self.stream.write("\n")
        
    def close(self):
        self.stream.close()

# create latex folder#
shutil.rmtree("./latex", ignore_errors=True)
os.mkdir("./latex")

# create latex source
latexSource = Writer("./latex/Main.tex")

# add packages
latexSource.addLine("\\documentclass{book}")
latexSource.addLine("\\author{Andrea Casalino}")
latexSource.addLine("\\title{Doc Debug}")
latexSource.addLine("\\RequirePackage[margin = 2cm]{geometry}")
latexSource.addLine("\\geometry{left = 2cm,right = 2cm,marginparwidth = 6.8cm,marginparsep = 1.5cm,top = 1.5cm,bottom = 1.5cm,footskip = 2\\baselineskip}")
latexSource.addLine("\\usepackage[T1]{fontenc}")
latexSource.addLine("\\usepackage[utf8]{inputenc}")
latexSource.addLine("\\usepackage[default]{lato}")
latexSource.addLine("\\usepackage{graphicx,color, import}")
latexSource.addLine("\\usepackage{amssymb, amsmath}")
latexSource.addLine("\\usepackage{url}")
latexSource.addLine("\\usepackage[]{algorithm2e}")
latexSource.addLine("\\usepackage[toc, page]{appendix}")
latexSource.addLine("\\begin{document}")
latexSource.addLine("\\maketitle")

# add contents
latexSource.addLine(FileHandler("./src/additional_Sections.tex").getContent())
    
latexSource.addLine("\\end{document}")        
latexSource.close()

# generate pdf
latexCompile = Writer("make.bat")
latexCompile.addLine("cd latex")
latexCompile.addLine("pdflatex Main.tex")
latexCompile.addLine("cd ../")
latexCompile.close()
subprocess.call([r'.\\make.bat'])

# clean up
shutil.copyfile("./latex/Main.pdf","EFG_debug.pdf")
os.remove("make.bat")
shutil.rmtree("./latex", ignore_errors=True)
shutil.rmtree("__pycache__", ignore_errors=True)
