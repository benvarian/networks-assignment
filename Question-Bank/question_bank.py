from get_questions import *
import random
import json

key = {
    'P': "./QuestionCSV/QuestionsP.csv",
    'C': "./QuestionCSV/QuestionsC.csv"
}

def gen_qs(type):
    return random.sample(get_questions(key[type]), 10)

def mark(qid, ans):
    return 1

def get_JSON_qs(q_type, q_num):
    qs = get_questions(key[q_type])
    
    q_num_clamped = max(0, min(q_num, len(qs)-1))
    asdf = random.sample(qs, q_num_clamped)
    json_qs = json.dumps(asdf)
    return json_qs
