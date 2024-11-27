# Chess-Engine-AI
Hi This is Repo where I have created Chess Engine + Chess AI Bots

Build this code with

Setup Virtual Environments
```
python3 -m venv chess-ai
```

Install Requirements
```
pip install -r requirements.txt
```

Compile the Chess Engine code to so file
```
clang++ -shared -o libchess_agent.so chess_agent.cpp -fPIC
```