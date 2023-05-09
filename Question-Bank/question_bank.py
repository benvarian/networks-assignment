from get_questions import *
import random

key = {
    'p': "./QuestionCSV/QuestionsP.csv",
    'c': "./QuestionCSV/QuestionsC.csv"
}

def gen_qs(type):
    return random.sample(get_questions(key[type]), 10)