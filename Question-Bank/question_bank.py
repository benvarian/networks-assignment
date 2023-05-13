from get_questions import *
import random
import json

key = {
    'p': "./QuestionCSV/QuestionsP.csv",
    'c': "./QuestionCSV/QuestionsC.csv"
}

def gen_qs(type):
    return random.sample(get_questions(key[type]), 10)

def mark(qid, ans):
    return 1

def get_JSON_qs(q_type, q_num):
    json_qs = "e"
    qs = get_questions(key[q_type])
    asdf = random.sample(qs, q_num)
    json_qs = json.dumps(asdf)
    return json_qs
