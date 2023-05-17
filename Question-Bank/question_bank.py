from get_questions import *
import random
import sys
import os
from ctypes import c_char_p, CDLL
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

    def __init__(self, subject):
        """inits QB_DB

        Args:
            type (String): The subject of the QB_DB (Python/C)

        Raises:
            ValueError: If subject is not recognised
        """
        self.type = subject
        if (subject not in key):
            raise ValueError("subject of q not recognised.")
        self.questions = get_questions(key[subject])

    def get_q_by_id(self, qid):
        """gets question

        Args:
            qid (int): question id to get

        Returns:
            Array: [type, question, answer]
                or None if question doesn't exist
        """
        try:
            return self.questions[qid]
        except:
            print("Question not found.")
            return None
        

    def get_rand_qs(self, q_num):
        """gets random questions

        Args:
            q_num (int): Number of questions to get

        Raises:
            Exception: Too many questions requested
            Exception: Too few questions requested

        Returns:
            array: array of questions
        """
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
        """marks a question

        Args:
            qid (int): question id
            ans (String): answer to be marked

        Returns:
            integer: mark of answer
        """
        # [type, q, a]
        q_obj = self.get_q_by_id(qid)
        ans = ans.rstrip('\x00')
        # TODO: if q_obj is none
        # programming
        try:
            if (q_obj[0] == 'P'):
                if qid % 2 == 1: lang = "P"
                else: lang = "C"
                student_ans = execute_function(lang, ans) # holds the error code here for now
                true_ans = execute_function(lang, q_obj[2])

                if(true_ans == student_ans): return 1
                else: return 0
            elif(q_obj[0] == 'M'):
                answer = ans.strip().rstrip('\x00')
                return 1 if (q_obj[2].strip() == answer) else 0
        except:
            pass
        # raise error...?
        # something went wrong.
        return 0

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
        try:
            os.remove('temp.c')
        except FileNotFoundError: pass
        try:
            os.remove('temp.so')
        except FileNotFoundError: pass
        try:
            os.remove('temp.o')
        except FileNotFoundError: pass

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
            main = CDLL("./temp.so")
            main.function.restype = c_char_p
            # Runs the code and returns the result in UTF-8 format
            result = main.function()
            # while isLoaded('./temp.so'):
            CDLL(None).dlclose(main._handle)
            os.remove('temp.c')
            os.remove('temp.so')
            os.remove('temp.o')
            return result.decode('utf-8')
        except Exception as e: 
            pass
            try:
                os.remove('temp.c')
            except FileNotFoundError: pass
            try:
                os.remove('temp.so')
            except FileNotFoundError: pass
            try:
                os.remove('temp.o')
            except FileNotFoundError: pass
            return (f"Error: {e}")
    else: return("Error: Invalid Programming Language")

def test():
    qb = QB_DB("C")
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
