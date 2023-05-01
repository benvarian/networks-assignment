import random
from get_questions import *

questions = get_questions('Question-Bank/QuestionCSV/Maths.csv')

sample_questions = random.sample(questions, 10)

print(sample_questions)