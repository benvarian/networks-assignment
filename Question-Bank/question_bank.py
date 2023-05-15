from get_questions import *
import random
import sys
import os
from ctypes import cdll, c_char_p
from distutils.ccompiler import new_compiler

key = {
    'P': "./QuestionCSV/QuestionsP.csv",
    'C': "./QuestionCSV/QuestionsC.csv"
}

class QB_DB:
    """ 
        Stores Questions, implements functions to mark and
        return questions for the QB to use.
    """

    def __init__(self, type):
        self.type = type
        if (type not in key):
            raise ValueError("type of q not recognised.")
        self.questions = get_questions(key[type])

    def get_q_by_id(self, qid):
        # returns q, or none if it does not exist
        try:
            return self.questions[qid]
        except:
            print("Question not found.")
            return None
        

    def get_rand_qs(self, q_num):
        # returns q_num questions as json array. -- unsure if json is the play for now.
        qids = self.questions.keys()
        if (q_num > len(qids)):
            raise Exception("Too many questions requested, cannot comply.")
        elif (q_num <= 0):
            raise Exception("Too few questions requested, cannot comply.")
        rand_ids = random.sample(list(qids), q_num)
        rand_qs = [[id] + self.questions[id] for id in rand_ids]
        return rand_qs

    def mark(self, qid, ans):
        # [type, q, a]
        q_obj = self.get_q_by_id(qid)
        # TODO: if q_obj is none
        # programming
        if (q_obj[0] == 'P'):
            # MITCH WORK
            if qid % 2 == 1: lang = "P"
            else: lang = "C"
            student_ans = execute_function(lang, ans) # holds the error code here for now
            true_ans = execute_function(lang, q_obj[2])
            if(true_ans == student_ans): return 1
            else: return 0
        elif(q_obj[0] == 'M'):
            return 1 if q_obj[2] == ans else 0
        # raise error...?
        return
    
    # TODO: send answer

# OTHER MITCH WORK
# Executes a function passed to it, with a specified language (P for Python C for C)
# If it runs into an error, it returns the exit code it ran into when running the program
def execute_function(lang, script):
    if(script == ''): return '' # hardcoding out empty string answers, as it causes issues when compiling in C
    if(lang == "P"):
        try:
            exec(script)
            result = locals().get('function')
            if(result == None): return ("Error: Function not named correctly")
            return result()
        except Exception as e: return (f"Error: {e}")
    elif (lang == "C"):
        main =  """\n\nint main() {\nchar *result = function();\nreturn 0;}"""
        try:
            result = None
            # Tries to compile the code - should work independently of any C libraries being installed, and across OS's
            compiler = new_compiler()
            with open('temp.c', 'w') as file:
                file.write("#include<stdio.h>\n")
                file.write(script)
                file.write(main)
            compiler.compile(['temp.c'])
            compiler.link_shared_object(['temp.o'], 'temp.so')
            main = cdll.LoadLibrary("./temp.so")
            main.function.restype = c_char_p
            # Runs the code and returns the result in UTF-8 format
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

def test():
    qb = QB_question_database("C")
    print(qb.questions, "\n")

    # test get q by id.
    print(qb.get_q_by_id(2))
    print(qb.get_q_by_id(4))
    print("Shouldnt be None : ", qb.get_q_by_id(10000)) # doesn't exist
    print("")

    # test mark multi choice
    q1 = qb.get_q_by_id(2)
    q2 = qb.get_q_by_id(4)
    print("should be 1", qb.mark(2, q1[2]))
    print("should be 0", qb.mark(2, q1[1]))
    print("")

    print("should be 1", qb.mark(4, q2[2]))
    print("should be 0", qb.mark(4, 1))
    print("should be 0", qb.mark(4, None))
    print("")

    # mark Programming
    print("Not yet implemented:")
    print("should be 1", qb.mark(30, """#include<stdio.h>\nchar *function() {return("Hello World");}""")) # suppose student submits proper code
    print("should be 0", qb.mark(30, ""))
    print("should be 0", qb.mark(30, "hey"))
    print("should be 0", qb.mark(30, "function"))
    print("")

    # test get random qs
    print(qb.get_random_qs(10), "\n")
    print(qb.get_random_qs(10), "\n")
    print(qb.get_random_qs(10), "\n")

    print("should fail from too many nums:")
    try:
        print(qb.get_random_qs(100), "\n")
    except Exception as e:
        print(e)

    print("should fail from too few nums:")
    try:
        print(qb.get_random_qs(0), "\n")
    except Exception as e:
        print(e)

    print("should fail from too few nums:")
    try:
        print(qb.get_random_qs(-1), "\n")
    except Exception as e:
        print(e)
    print("")

#test()
