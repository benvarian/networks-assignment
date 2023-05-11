from get_questions import *
import random

key = {
    'p': "./QuestionCSV/QuestionsP.csv",
    'c': "./QuestionCSV/QuestionsC.csv"
}

def gen_qs(type):
    return random.sample(get_questions(key[type]), 10)

def mark(qid, ans):
    return 1

def get_JSON_qs(q_num, q_type):
    return '["question1", "answer14"]'