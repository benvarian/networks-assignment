def execute_script(lang, script):
    if(lang == "P"):
        exec(script)
        result = locals().get(script)
        return result()

result = execute_script("P", """def test():\n    return("Hello World")""")
print(result)