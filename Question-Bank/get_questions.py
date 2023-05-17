import os

def get_questions(filePath):
    """Opens and parses question csv file. '\c' maps to ',' and '\y' maps to newline.

    Args:
        filePath (String): path to csv to be opened

    Returns:
        dict: dictionary of arrays representing questions. 
            Key is Qid, array is [type, question, answer]
    """
    
    try:
        with open(filePath, 'r', encoding='utf-8-sig') as data:
            questions = { }
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
                    row[2] = row[2].replace("\\n", "\n")
                    row[3] = row[3].replace("\\n", "\n")

                    # \y is a new line in coding questions.
                    row[1] = row[1].replace("\y", "\n")
                    row[2] = row[2].replace("\y", "\n")
                    row[3] = row[3].replace("\y", "\n")

                    # \c is a comma in coding questions.
                    row[1] = row[1].replace("\c", ",")
                    row[2] = row[2].replace("\c", ",")
                    row[3] = row[3].replace("\c", ",")
                    questions[row[0]] = [row[1], row[2], row[3]]
            return questions
        
    # File Path not found
    except FileNotFoundError:
        print(f"""Filepath "{filePath}" not found""")
        return None
