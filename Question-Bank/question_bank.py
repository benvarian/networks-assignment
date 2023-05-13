from get_questions import *
import random
import json
import sys

key = {
    'P': "./QuestionCSV/QuestionsP.csv",
    'C': "./QuestionCSV/QuestionsC.csv"
}

class QB_question_database:
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
        

    def get_random_qs(self, q_num):
        # returns q_num questions as json array. -- unsure if json is the play for now.
        qids = self.questions.keys()
        if (q_num > len(qids)):
            raise Exception("Too many questions requested, cannot comply.")
        elif (q_num <= 0):
            raise Exception("Too few questions requested, cannot comply.")
        rand_ids = random.sample(list(qids), q_num)
        rand_qs = [self.questions[id] for id in rand_ids]
        json_qs = json.dumps(random.sample(rand_qs, q_num))
        return json_qs

    def mark(self, qid, ans):
        # [type, q, a]
        q_obj = self.get_q_by_id(qid)
        # TODO: if q_obj is none
        # programming
        if (q_obj[0] == 'P'):
            # MITCH WORK
            return execute_script(q_obj[0], q_obj[1])
        elif(q_obj[0] == 'M'):
            return 1 if q_obj[2] == ans else 0
        # raise error...?
        return
    
    # TODO: send answer

# OTHER MITCH WORK
def execute_script(type, script):
    return 0

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
    print("should be 1", qb.mark(30, qb.get_q_by_id(30)[2]))
    print("should be compilation issue", qb.mark(30, "hey"))
    print("should be 0", qb.mark(30, ""))
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

# test()
