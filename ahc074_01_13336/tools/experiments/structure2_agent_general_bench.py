import subprocess,pathlib,re
data=pathlib.Path('tools/in/0000.txt').read_bytes()
for mode in range(5):
 p=subprocess.run(['tools/experiments/structure2_agent_general','tools/experiments/templates_agent_2d32_result.txt','128','1',str(mode)],input=data,capture_output=True)
 print(mode,p.stderr.decode().splitlines()[-1],flush=True)
