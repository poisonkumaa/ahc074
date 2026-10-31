"""Expand local C++ headers into one submission file; retain standard includes."""
import pathlib
import re
import sys

source=pathlib.Path(sys.argv[1]).resolve()
output=pathlib.Path(sys.argv[2]).resolve()
seen=set()

def expand(path):
    path=path.resolve()
    if path in seen:
        return ''
    seen.add(path)
    lines=[]
    for line in path.read_text().splitlines(keepends=True):
        match=re.fullmatch(r'\s*#include\s+"([^"\n]+)"\s*\n?',line)
        if match:
            lines.append(expand(path.parent/match.group(1)))
        elif line.strip()!='#pragma once':
            lines.append(line)
    return ''.join(lines)+'\n'

output.write_text(expand(source))
print(f'{len(seen)} local sources -> {output.name}')
