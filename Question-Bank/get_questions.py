import os
from re import findall
from random import randint

# Opens csv file and handles errors
# Returns a dictionary of questions, or None if file not found
# The key is the question ID
# The value is a size-2 array: [question, answer]
# NOTE: If a QID is a duplicate or corrupt
# (e.g. A string instead of an int) the line is skipped
def get_questions(filePath):
    try:
        with open(filePath, 'r', encoding='utf-8-sig') as data:
            questions = {}
            lines = data.readlines()
            # Iterate over each row, adding to the dictionary
            for row in lines[1:]:
                row = row.split(',')
                # If a QID is duplicated or corrupt, skip the line
                try:
                    row[0] = int(row[0])
                except ValueError:
                    continue
                if row[0] in questions:
                    continue
                else:
                    # Replace \\n with \n because when \n is stored in the csv file
                    # it is changed to \\n when read in by Python, this will help 
                    # question/answer formatting
                    row[1] = row[1].replace("\\n", "\n")
                    row[1] = row[1].replace("\\t", "\t")
                    row[2] = row[2].replace("\\n", "\n")
                    row[2] = row[2].replace("\\t", "\t")
                    questions[row[0]] = [row[1], row[2]]
            return questions
    # File Path not found
    except FileNotFoundError:
        print(f"""Filepath "{filePath}" not found""")
        return None

# WORK IN PROGRESS
# Given a generic question string it will randomize
# each variable stored in curly brackets and return
# the question with the placeholders replaced with values
def randomize_question(questionString):
    dict = {}
    # credit to chatgpt for the regex to capture the values inside {} whilst negating double brackets {{}}
    values = findall(r"\{([^{}\d]*[^{}]*?)\}", f"{questionString}")
    for i in values:
        if i not in dict:
            dict[i] = randint(1,100) # HARDCODED ALL VARIABLES TO BE WITHIN 1 AND 100 FOR NOW -- NEED TO REVIST LATER
            questionString = questionString.replace(f"{{{i}}}", f"{dict[i]}")
    return questionString
    
    

# WORK IN PROGRESS
# Given a question string it writes to a tmp file
# then executes the string, and returns the answer
# deletes the tmp file afterwards
def get_answer(questionNum, questionString):
    # if question number is odd it is a python question
    if questionNum % 2 == 1:
        return None
    # else treat it like a C question
    else:
        return None

# TESTING
if __name__ == "__main__":
    pythonDict = get_questions("Question-Bank\QuestionCSV\QuestionsP.csv")
    for question in pythonDict:
        pythonDict[question][0] = randomize_question(pythonDict[question][0])
    for key in pythonDict:
        print(f"Question ID: {key}\nQuestion: {pythonDict[key][0]}\n\nAnswer: {pythonDict[key][1]}\n")
