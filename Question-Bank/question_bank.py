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
        return self.questions[qid]

    def get_random_qs(self, q_num):
        # returns q_num questions as json array. -- unsure if json is the play for now.
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
    print(qb.questions)

    print(qb.get_q_by_id)

test()