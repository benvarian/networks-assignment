from get_questions import *
import random
import json
import sys

key = {
    'p': "./QuestionCSV/QuestionsP.csv",
    'c': "./QuestionCSV/QuestionsC.csv"
}

class QB_question_database:
    """ 
        Remembers Questions, implements functions to mark and
        return questions for the QB to use.
    """

    def __init__(self, type):
        self.type = type
        if (type not in key):
            raise ValueError("type of q not recognised.")
        self.questions = get_questions(key[type])

    def get_q_by_id(self, qid):
        return self.questions[qid]

    def get_random_qs(self, q_num):
        # returns q_num questions as json array.

        qids = self.questions.keys()
        rand_ids = random.sample(list(qids), q_num)
        rand_qs = [self.questions[id] for id in rand_ids]
        json_qs = json.dumps(random.sample(rand_qs, q_num))
        return json_qs

    def mark(self, qid, ans):
        # [type, q, a]
        q_obj = self.get_q_by_id(qid)
        # programming
        if (q_obj[0] == 'P'):
            return execute_script(q_obj[0], q_obj[1])
        elif(q_obj[0] == 'M'):
            # multichoice
            return q_obj[2][0] == ans
        # raise error...
        return

def execute_script(type, script):
    return 0