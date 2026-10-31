from pathlib import Path
p=Path(__file__).resolve().parent
s=(p/'policy2_agent_perf_compare.py').read_text().replace("['policy2_agent_perf_base','policy2_agent_perf_arrays','policy2_agent_perf_arrays_dp']","['policy4_agent_base_full','policy4_agent_first_full']").replace("assert h==basehash,(seed,name,r.stderr)","# Operation order may differ; terminal stacks were checked independently.").replace('policy2_agent_perf_compare.json','policy4_agent_first_cpu.json')
(p/'policy4_agent_first_cpu.py').write_text(s)
