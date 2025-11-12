import json
import os
import subprocess

with open("Sponza.gltf", "r") as file:
    gltf = json.load(file)

for image in gltf.get("images", []):
    prev_uri = image["uri"]
    base, _ = os.path.splitext(prev_uri)
    bc_uri = base + ".dxt"

    image["uri"] = bc_uri
    image["mimeType"] = "image/dxt"

    try:
        print(f"Converting {prev_uri} -> {bc_uri}")
        subprocess.run(["bcnconv.exe", prev_uri, bc_uri, "auto"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error converting {prev_uri}: {e}")
        continue

    if os.path.exists(prev_uri):
        os.remove(prev_uri)

with open("Sponza.gltf", "w") as file:
    json.dump(gltf, file, indent=4)