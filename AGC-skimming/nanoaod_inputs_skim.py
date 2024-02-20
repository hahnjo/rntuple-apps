import json

with open("nanoaod_inputs.json") as f:
    inputs = json.load(f)

inputs_skimmed = {}
for process, variations in inputs.items():
    inputs_skimmed[process] = {}
    for variation, d in variations.items():
        nevts = 0
        for f in d["files"]:
            nevts += f["nevts"]
        if nevts != d["nevts_total"]:
            print(f"mismatch: {nevts} vs. {d['nevts_total']} total")
        inputs_skimmed[process][variation] = {
            "nevts_total": nevts,
            "files": [{"path": f"{process}.{variation}.root", "nevts": nevts}],
        }

print(json.dumps(inputs_skimmed))
