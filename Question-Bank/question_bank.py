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
        return ["this is the qid", "this is the question", "this is the id."]

    def get_random_qs(self, q_num):
        # returns q_num questions as json array.

        qids = self.questions.keys()
        rand_ids = random.sample(list(qids), q_num)
        rand_qs = [self.questions[id] for id in rand_ids]
        json_qs = json.dumps(random.sample(rand_qs, q_num))
        return json_qs

    def mark(self, qid, ans):
        return 1
