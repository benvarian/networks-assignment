import random
from get_questions import *

def sample_questions(type, num):
    questions = get_questions(type)
    return random.sample(questions, num)
    
print(sample_questions('Maths', 10))
