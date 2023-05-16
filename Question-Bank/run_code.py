import os
from ctypes import cdll, c_char_p
from distutils.ccompiler import new_compiler
from distutils import errors

# Executes a function passed to it, with a specified language (P for Python C for C)
# If it runs into an error, it returns the exit code it ran into when running the program
def execute_function(lang, script):
    if(lang == "P"):
        try:
            exec(script)
            result = locals().get('function')
            if(result == None): return ("Error: Function not named correctly")
            return result()
        except Exception as e: return (f"Error: {e}")
    elif (lang == "C"):
        main =  """\n\nint main() {\nchar *result = function();\nprintf("%s", result);\nreturn 0;}"""
        try:
            compiler = new_compiler()
            with open('temp.c', 'w') as file:
                file.write(script)
                file.write(main)
            compiler.compile(['temp.c'])
            compiler.link_shared_object(['temp.o'], 'temp.so')
            main = cdll.LoadLibrary("./temp.so")
            main.function.restype = c_char_p
            result = main.function()
            os.remove('temp.c')
            os.remove('temp.so')
            return result.decode('utf-8')
        except Exception as e: 
            try:
                os.remove('temp.c')
                os.remove('temp.so')
            except FileNotFoundError: pass
            return (f"Error: {e}")
    else: return("Error: Invalid Programming Language")

result = execute_function("P", """def function():\n\treturn ("Hello World")""")
print(result)
result = execute_function("C", """#include<stdio.h>\nchar *function() {return("Hello World");}""")
print(result)
result = execute_function("P", """hey""")
print(result)