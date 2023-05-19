# networks-assignment
The repo for a group project for CITS3002

The memebers of the group can be found in the contributor section of the main page

The detailed project description can be found in [here](https://teaching.csse.uwa.edu.au/units/CITS3002/project/)

The main idea for this project is to code a Test Manager and Question Bank in their respective languages and create a socket server between the two that allows for requests that a client makes on their browser to be fulfilled. 

## How to run 
### How to run Test Manager
Before running the Test Manager please make sure your in the Test-Manager directory. 
```
    make
```

```
    ./server [port]
```

## How to run Question Bank
Before running the Question Bank please make sure your in the Question-Bank directory. 
```
    python3 qb_socket.py [ip] [port] [type: C | PYTHON]
```


## View output 
Simply go to your browser and go to [ip]:[port] and you will see the magic happen!!