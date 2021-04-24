from FileHandler import *
import os
import subprocess
import shutil

# clean up
shutil.rmtree("./latex", ignore_errors=True)
shutil.rmtree("./html", ignore_errors=True)

# run doxygen
os.system("\"C:\\Program Files\\doxygen\\bin\\doxygen\" doxy_config")

# modify latex src
texHandler = FileHandler("./latex/refman.tex")
texHandler.addAfter("\\renewcommand{\\numberline}[1]{#1~}" , FileHandler("./src/packages.tex").getContent(), 1)
texHandler.addAfter("%--- Begin generated contents ---" , FileHandler("./src/additional_Sections.tex").getContent(), 1)
texHandler.reprint("./latex/refman.tex")

# modify compile latex script
docGenHandler = FileHandler("./latex/make.bat")
docGenHandler.addBefore("setlocal enabledelayedexpansion", "bibtex refman")
docGenHandler.replaceLine("set count=8", "set count=4")
docGenHandler.addBefore("cd /D %Dir_Old%", "COPY refman.pdf \"../EFG.pdf\"")
docGenHandler.replaceLine("cd /D %Dir_Old%", "")
docGenHandler.replaceLine("set Dir_Old=", "")
docGenHandler.reprint("./latex/make.bat")

# generate pdf
subprocess.call([r'.\\latex\\make.bat'])

# clean up
shutil.rmtree("./latex", ignore_errors=True)
shutil.rmtree("./html", ignore_errors=True)
shutil.rmtree("__pycache__", ignore_errors=True)
