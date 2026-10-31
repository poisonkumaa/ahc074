from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy_agent_fast100000_02.cpp').read_text()
b=s.index(' vector<uint8_t> make_policy(');e=s.index(' // Estimate transition costs',b)
m=(p/'policy_agent_mdp.txt').read_text()
for ui,gi in [(8,1),(8,3),(8,6)]:
    out=s[:b]+m.replace('MDP_UNIFORM',str(ui)).replace('MDP_GROUP',str(gi))+s[e:]
    out=out.replace('while(display[s]!=t)', 'while(policy[t*states+s]!=255)')
    out=out.replace('while(gr.display[g.state]!=t)', 'while(g.policy[t*gr.states+g.state]!=255)')
    out=out.replace('const int STEPS=100000;', 'const int STEPS=30000;')
    (p/f'policy_agent_mdp{ui}_{gi}.cpp').write_text(out)
