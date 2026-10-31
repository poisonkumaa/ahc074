import os,signal,sys
matches=[]
for n in os.listdir('/proc'):
    if not n.isdigit():continue
    try:a=open('/proc/'+n+'/cmdline','rb').read().split(bytes([0]))
    except OSError:continue
    if len(a)>2 and a[1].endswith(b'round5_sampling_bench.py') and a[2]==b'round5_sampling_adaptive':matches.append(int(n))
for pid in matches:os.kill(pid,signal.SIGCONT if sys.argv[1]=='resume' else signal.SIGSTOP)
print(matches)
