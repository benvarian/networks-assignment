## Authors
- Nicholas Davies (22967042) (QB & protocol)
- Mitchell Otley (23475725) (execute_function code, get_questions.py)  

With help from Ben and James
## Coded with:
Python
## Question Subjects Implemented:
C, Python

## To run:
Within the Question-Bank folder, run   
```python Question_Bank.py {host} {port} {'C' or 'PYTHON'}```  
Note: run 2 instances of the Question_Bank, one for C and one for Python for the TM to work.  
QBs do not need to be run on the same machine.  

# Database of questions -- (implemented as csv)
## CSV formatting:
The CSV's are formatted with the following variables:  
```
QID,T,Q,A
```
qid is the id of the question. -- an even qid means it is a python question, and odd means it is C  

T is the 'type' of question -- M means multi choice, P means Programming. 
  
Q is the question -- in multi choice questions this will include each of the choices.  

A is the answer -- in programming questions it will contain a script that writes the correct answer.  
    in multi choice questions it will contain either A, B, C, or D which is what the web side returns on a submission attempt  

In order to avoid unwanted bugs and generally make life easier,  
    '\y' maps to '\n'
    '\c' maps tp ',' as commas are, of course, a part of csv format.  
    '\t' maps to '{tabspace}' (python doesn't otherwise recognise \t as an escaped sequence.)  

Note: Answers for programming questions is executable code that will return the correct answer.

## Question Subjects Implemented:  
C, Python
  
# QB-TM Protocol
It is worth to first note that The QB connects to the TM -- not the other way around. Connections are therefore long lived and do not (intentionally) close.  

The QB-TM Protocol was designed to loosely follow HTML protocol, so that the parsing on the TM's side would be simpler, as  
it already had functions to parse the Web's HTML requests'.  

The QB connects to the TM, sends an 'init' handshake to initiate the connection, and from there it's function is entirely dependent on the TM.  

If at any time the QB disconnects from the TM, it will attempt to connect to the TM's address every 2 seconds.  

# Protocol details
All protocols have a header that ends in '\r\n\r\n' in order to replicate HTML's formatting. Having short or cryptic protocols made requests very difficult or annoying to interpret and debug, so they were made to be human readable. As such, all integers are sent and received as strings in the utf-8 format.  
  
Init 'Handshake' from QB:  
```
QB {subject}  

```
Where {subject} is either 'PYTHON' or 'C', representing the type of QB for the TM to know.
## Getting Questions:
### Request:
```
QUESTIONS  
{subject}:{num}  

```
'subject' represents the subject of questions wanted (currently left over from when QB was singular and not split up.)  
'num' represents the number of questions to be returned.  

### Response:
```
QB {subject}
QUESTIONS

&{qid_1}:{type_1}&{qid_2}:{type_2}&...
```
continuing for every question randonly chosen from the csv.
Note: type is not subject, type is either "P" or "M" (programming or multi choice)
Also Note: Questions themselves are not sent. Just the random qid's and their types for the TM to store.

## Getting Singluar Question:
### Request:
```
GETQUESTION
{qid}
```
### Response:
```
QB {subject}
GETQUESTION

{question_string}
```
where question_string is the question itself (eg. "What is 1+3"), not including anything else. 

## Marking A question:
### Request:
```
MARK
{qid}:{ans}
```
where qid is the id of the question to be marked, and ans is the answer to be marked.
### Response:
```
QB {subject}
MARK

{mark (as string)}
```
Nothing too complex -- Mark will only ever be 1 or 0, for correct or wrong
If the question is a programming question, the QB will run the code, terminating at 5 seconds.
Protections against malicious code is not given
However, protection against the ever so common "segmentation fault", and all other Runtime errors,
is implemented.
## Getting an Answer as a string:
### Request:
```
ANSWER
{qid}
```
### Response:
```
QB {subject}
ANSWER

{answer (as string)}
```
This is the only way for the TM to get the correct answer for a question.
## Pinging the QB:
### Request:
```
TM
```
### Response:
```
ACCEPTED PING
```
There is always a place for pinging -- very useful when debugging, but also to give the TM more control over protocols without ever having to change the QB's code.

# Python Files
## get_questions.py
This file get's and parses questions from the csv.
## qb_database.py
This file contains the "QB_DB" class. It stores questions and serves up many functions for parsing and accessing questions.
questions are stored within a dictionary within an instance of QB_DB - the key is the qid, and the value is [q_type, q, ans]
## qb_socket.py
This file contains the "QB_Socket_Connection" class. It implements all of the protocols needed for sending and receiving questions, and 
uses an infinite main loop to wait for messages from the TM at any time. 
If any errors shall occur, it should always react to them, and never crash (except for on mac, for unknown reasons.)
Shall anything 'bad' enough happen, the QB will simply restart its socket, and reconnect to the TM.
## Question_Bank.py
This file contains the main process of the QB, init'ing variables, parsing command line arguments and otherwise beginning the program.
