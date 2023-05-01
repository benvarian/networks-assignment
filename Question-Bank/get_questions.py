import os

# Opens csv file and handles errors
# Returns a dictionary of questions, or None if file not found
# The key is the question ID
# The value is a size-2 array: [question, answer]
# NOTE: If a QID is a duplicate or corrupt
# (e.g. A string instead of an int) the line is skipped
def get_questions(filePath):
    try:
        with open(filePath, 'r', encoding='utf-8-sig') as data:
            questions = []
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
                    
                    # remove newline in q.
                    row[-1].replace("//n", "")

                    questions.append({'questionID': row[0], 'question': row[1], 'answer': row[2], 'fake1': row[3], 'fake2': row[4], 'fake3': row[5]})
            return questions
    # File Path not found
    except FileNotFoundError:
        print(f"""Filepath "{filePath}" not found""")
        return None

# # TESTING
# if __name__ == "__main__":
#     pythonDict = getQuestions("QuestionCSV\QuestionsP.csv")
#     for key in pythonDict:
#         print(f"Question ID: {key}\nQuestion: {pythonDict[key][0]}\nAnswer: {pythonDict[key][1]}\n")
