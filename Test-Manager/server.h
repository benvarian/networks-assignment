#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>
#include <ctype.h>

#include "Data-Structures/Dictionary/Dictionary.h"

#define BACKLOG 10
#define MAXDATASIZE 4095
#define BSIZE 1024
#define SOCKET int
#define NUM_QB 2

enum HTTPMethods
{
    GET,
    POST,
    PUT,
    HEAD,
    PATCH,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
};

enum QBType
{
    NONE,
    PYTHON,
    C
};

typedef struct HTTPRequest
{
    struct Dictionary request_line;
    struct Dictionary header_fields;
    struct Dictionary body;
} HTTPRequest;

typedef struct QBInformation
{
    int socket;
    enum QBType type;
} QBInformation;

// define functions
void usage(void);

void drop_client(SOCKET socket);

const char *get_content_type(const char *path);

void sigchld_handler(int s);

void *get_in_addr(struct sockaddr *sa);

void send_200(SOCKET socket);

void send_400(SOCKET socket);

void send_404(SOCKET socket);

void handle_get(SOCKET socket, HTTPRequest request);

void handle_post(HTTPRequest response, SOCKET socket);

void parse_request(char *response_string, SOCKET socket);

void received(int new_fd, int numbytes, char *buf);

void manage_connection(SOCKET sockfd);

SOCKET bind_socket(struct addrinfo *servinfo);

struct addrinfo *get_info(char *port);

void http_request_destructor(HTTPRequest *request);

void extract_request_line_fields(HTTPRequest *request, char *request_line);

void extract_header_fields(HTTPRequest *request, char *header_fields);

void extract_body(HTTPRequest *request, char *body);

void handle_post(HTTPRequest response, SOCKET socket);

void addq_to_hashtable(char *student_name, int qnum, char *qid, char *type);

void ping_QB(SOCKET socket, int qb_num);

void increment_question(char *student_name);

void answer_correct(char *student_name, int qid);

void answer_incorrect(char *student_name, int qid);

int connect_QB(SOCKET socket, enum QBType type);

int get_questions(char *student);

void send_webpage(SOCKET socket, char *centre, const char *start, const char *end);

static const char *summary_start = "<html><head><title>Quiz</title></head><body><h1>Test HomePage</h1><button onclick=\"window.location.href='quiz/start'\" id=\"quiz\" class=\"ml-4\">Attempt Test</button> <div class=\"Summary-Area\" style=\"flex: justify-center;width: 60%;height: 500px;margin: 100px 20% 0 20%;background: rgba(0,0,0,0.1);overflow: auto;\">";
static const char *summary_end = "</body></html>";

static const char *first_multi = "<!DOCTYPE html>\n<html lang='en' dir='ltr'>\n  <head>\n    <meta charset='utf-8'>\n    <meta name='viewport' content='width=device-width, initial-scale=1.0' />\n    <script src='https://cdn.tailwindcss.com'></script>\n<style>.hide {display: none;}</style>\n</head>\n <body>\n<div>\n<nav class='bg-slate-100 shadow flex justify-between sticky top-0 z-50 place-items-center w-full'>\n        <div class='flex justify-center'>\n          <h1 class='mx-4'>CITS3002 Project</h1>\n          <button class='ml-4'>logout</button>\n          <a class='ml-4' href='profile.html'>back</a>\n        </div>\n      </nav>\n    </div>\n    <div class='Quiz-Area' style='flex: justify-center;\n    width: 60%;\n    height: 500px;\n    margin: 100px 20% 0 20%;\n    border-radius: 10px;\n    background: rgba(0,0,0,0.1);\n    box-shadow: 0 0 10px 2px rgba(100,100,100,0.1);\n    overflow: auto;\n'>      <div style='display: block;'class='Quiz-Header slide'>\n<h1 style='font-size: 30px;\n        color: #3d3d3d;\n        text-align: center;'>Question</h1><br>\n        <div class='Question-Area' style='width: 90%;\n        height: 70%;\n        border-bottom: 2px solid #3d3d3d;\n        margin: 0 5%;'>";
static const char *last_multi = "</div>\n        <div class='Answer-Area' style='display: flex;\n        width: 90%;\n        height: 20%;\n        margin: 0 5%;'>\n          <div class='Half-Answer-Area' style='width: 50%;\n          height: 100%;\n          margin: 0;'>\n            <ul style='list-style-type: none;\n            padding: 0;'>\n              <li style='font-size: 1.2rem;\n              height: 20%;\n              margin: 2% 8%;'>\n                <input type='radio' name='q' id='1a' class='answer'>\n                <label for='1a' id='a_text'>A</label>\n              </li>\n              <li style='font-size: 1.2rem;\n              height: 20%;\n              margin: 2% 8%;'>\n                <input type='radio' name='q' id='1b' class='answer'>\n                <label for='1b' id='a_text'>B</label>\n              </li>\n            </ul style='list-style-type: none;\n            padding: 0;'>\n          </div>\n          <div class='Half-Answer-Area' style='width: 50%;\n          height: 100%;\n          margin: 0;'>\n            <ul style='list-style-type: none;\n            padding: 0;'>\n              <li style='font-size: 1.2rem;\n              height: 20%;\n              margin: 2% 8%;'>\n                <input type='radio' name='q' id='1c' class='answer'>\n                <label for='1c' id='a_text'>C</label>\n              </li>\n              <li style='font-size: 1.2rem;\n              height: 20%;\n              margin: 2% 8%;'>\n                <input type='radio' name='q' id='1d' class='answer'>\n                <label for='1d' id='a_text'>D</label>\n              </li>\n            </ul>\n          </div>\n\n        </div>\n\n      </div>\n\n      <div class='Quiz-Bottom' style='\n      width: 94%;\n      height: 14%;\n      margin: 0 3%;'>\n\n        <input class='bottom-button' onclick='submitPressed()' type='submit' name='' value='Submit Question' style=\"width: 40%;\n        height: 100%;\n        width: 80%;\n        margin: 0 10%;\n        background-color: #80d9ff;\n        cursor: pointer;\n        border-radius: 10px;\n\">\n<button id=\"real-submit\"  class=\"text-blue-500\" onclick=\"reload()\">Next Question</button>\n<p class=\"text-green-500\" id=\"correct\">Answer Correct</p> <p class=\"text-red-700\" id=\"incorrect\">Answer incorrect</p></div>\n</div>\n<script>\n let qtnArea = document.getElementsByClassName('Question-Area');\n let quizArea = document.getElementsByClassName('Quiz-Area');\nlet id = document.getElementById(\"incorrect\"); window.addEventListener(\"load\", ()=>{ console.log(\"page loaded\"); id.classList.add(\"hide\"); correct.classList.add(\"hide\"); real.classList.add('hide'); }); \nlet submitBtn = document.getElementsByClassName('submit-button');\nlet radio = document.getElementsByName('q'); let correct = document.getElementById(\"correct\"); let real = document.getElementById('real-submit')\nlet radioAnsArea = document.getElementsByClassName('Half-Answer-Area');\nlet ans = \"\";\nconst addAnswer = (question, answer) => {\nans = `qid=${question}&sans=${answer}&ans=${answer}\\0`;}\nvar xhr = new XMLHttpRequest();\nxhr.onreadystatechange = () => {if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 418) {incorrect.classList.remove('hide'); correct.classList.add('hide'); real.classList.add('hide') } else if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 201) {real.classList.remove('hide');}  else {incorrect.classList.add('hide'); correct.classList.remove('hide'); real.classList.remove('hide');}}; \nconst submit = async (answers) => {\nxhr.open('POST', window.location.href, true);\nxhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');\nawait xhr.send(ans);\n}\n//function called when the submit button is pressed\nfunction submitPressed() {\nlet selections = ['a','b','c','d'];\nfor(let k = 0; k < 4; k++) {\nif(radio[k].checked) {\naddAnswer(1,selections[k]);\nsubmit(ans);\n}}} function reload() {if (incorrect.classList.contains(\"hide\") === true )window.location.reload();} </script>\n</body>\n</html>\n";

static const char *first_input = "<!DOCTYPE html><html lang='en' dir='ltr'> <head> <meta charset='utf-8'/> <meta name='viewport' content='width=device-width, initial-scale=1.0'/> <script src='https://cdn.tailwindcss.com'></script> <style>.hide{display: none;}</style> </head> <body> <div> <nav class='bg-slate-100 shadow flex justify-between sticky top-0 z-50 place-items-center w-full' > <div class='flex justify-center'> <h1 onclick='window.location.href='/'' class='mx-4'>CITS3002 Project</h1> <button onclick='window.location.href='/logout'' class='ml-4'>logout</button> </div></nav> </div><div class='Quiz-Area flex flex-col justify-center items-center h-[calc(100vh-72px)]'> <div class='Quiz-Header slide'> <div class='Question-Area'>";
static const char *last_input = "</div><div class='Answer-Area'> <div class='Text-Answer-Area'> <textarea class='codeInput border-solid border-2 border-slate-500' rows='8' cols='80'></textarea> </div></div></div><div class='Quiz-Bottom'> <input class='bottom-button bg-yellow-300 rounded-md p-2 m-2 pb-2' onclick='submitPressed()' type='submit' name='' value='Submit Answer'/> <button id='real-submit' class='text-blue-500' onclick='reload()'> Next Question </button> <p class='text-red-700' id='incorrect'>Answer incorrect</p><p class='text-green-500' id='correct'>Answer Correct</p></div></div><script>let qtnArea=document.getElementsByClassName('Question-Area'); let quizArea=document.getElementsByClassName('Quiz-Area'); let submitBtn=document.getElementsByClassName('submit-button'); let textAns=document.getElementsByClassName('codeInput'); let textAnsArea=document.getElementsByClassName('Text-Answer-Area'); let radioAnsArea=document.getElementsByClassName('Half-Answer-Area'); let sumQtnDisplay=document.getElementsByClassName('sumQtnDisplay'); let sumAnsDisplay=document.getElementsByClassName('sumAnsDisplay'); let incorrect=document.getElementById('incorrect'); let real=document.getElementById('real-submit'); let correct=document.getElementById('correct'); let answers=''; window.addEventListener('load', ()=>{console.log('page loaded'); incorrect.classList.add('hide'); correct.classList.add('hide'); real.classList.add('hide');}); var xhr=new XMLHttpRequest(); const addAnswer=(question, answer)=>{ans=`qid=${question}&sans=${answer}&ans=${answer}`;}; xhr.onreadystatechange=()=>{if (xhr.readyState===XMLHttpRequest.DONE && xhr.status===418){incorrect.classList.remove('hide'); real.classList.add('hide'); correct.classList.add('hide');}else if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 201) {real.classList.remove('hide');}  else{incorrect.classList.add('hide'); correct.classList.remove('hide'); real.classList.remove('hide');}}; const submit=async (answers)=>{xhr.open('POST', window.location.href, true); xhr.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' ); await xhr.send(ans);}; function reload (){window.location.reload();}function submitPressed(){if (textAns[0].value !=''){addAnswer(1, textAns[0].value); submit(ans);}}</script> </body></html>";
