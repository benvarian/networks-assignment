import os
from ctypes import cdll
from distutils.ccompiler import new_compiler
import subprocess

def execute_script(lang, script):
    if(lang == "P"):
        exec(script)
        result = locals().get('function')
        return result()
    elif (lang == "C"):
        main =  """\n\nint main() {\nchar *result = function();\nprintf("%s", result);\nreturn 0;}"""
        try:
            compiler = new_compiler()
            with open('temp.c', 'w+') as file:
                file.write(script)
                file.write(main)
            compiler.compile(['temp.c'])
            compiler.link_shared_object(['temp.o'], 'temp.so')
            main = cdll.LoadLibrary("./temp.so")
            subprocess.run(['ls', '-al']) # Just printing out the directory to see if objects are created
            print(main.function())
            result = main.function()
            return result
        finally:
            os.remove('temp.c')
            os.remove('temp.so')
            print("done")
    else: return("Error: Invalid Programming Language")

result = execute_script("P", """def function():\n\treturn ("Hello World")""")
print(result)
result = execute_script("C", """#include<stdio.h>\nchar *function() {return("Hello World");}""")
print(result)