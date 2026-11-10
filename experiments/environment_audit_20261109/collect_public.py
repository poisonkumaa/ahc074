"""Archive publicly documented settings; inspect zip in memory without extracting."""
from pathlib import Path
import concurrent.futures
import difflib
import hashlib
import io
import json
import urllib.request
import zipfile

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
sources={
 'language_list.html':'https://img.atcoder.jp/file/language-update/2025-10/language-list.html',
 'compiler_install.toml':'https://img.atcoder.jp/file/language-update/2025-10/116-23_clang_19-1-3.toml',
 'official_tools.zip':'https://img.atcoder.jp/ahc074/YAAu43G9.zip',
}
def fetch(item):
    name,url=item
    try:
        with urllib.request.urlopen(url,timeout=20) as r:
            data=r.read(20*1024*1024+1)
            if len(data)>20*1024*1024: raise ValueError('response exceeds limit')
        (HERE/name).write_bytes(data)
        result={'url':url,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
        if name.endswith('.zip'):
            archive=zipfile.ZipFile(io.BytesIO(data))
            result['comparisons']=[]
            for suffix in ['src/lib.rs','src/bin/tester.rs','Cargo.toml']:
                members=[m for m in archive.namelist() if m.endswith('/'+suffix) or m==suffix]
                if len(members)!=1:
                    result['comparisons'].append({'file':suffix,'matches':members});continue
                remote=archive.read(members[0]);local=(ROOT/'tools'/suffix).read_bytes()
                same=remote.replace(b'\r\n',b'\n')==local.replace(b'\r\n',b'\n')
                result['comparisons'].append({'file':suffix,'same_normalized':same,'remote_sha256':hashlib.sha256(remote).hexdigest(),'local_sha256':hashlib.sha256(local).hexdigest()})
                if not same:
                    diff=''.join(difflib.unified_diff(remote.decode().splitlines(True),local.decode().splitlines(True),fromfile='official/'+suffix,tofile='local/'+suffix))
                    (HERE/('diff_'+suffix.replace('/','_')+'.txt')).write_text(diff)
        return name,result
    except Exception as e: return name,{'url':url,'error':str(e)}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
    results=dict(pool.map(fetch,sources.items()))
(HERE/'public_fetch.json').write_text(json.dumps(results,ensure_ascii=False,indent=2))
print(json.dumps(results,ensure_ascii=False,indent=2))
