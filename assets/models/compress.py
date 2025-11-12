import json
import os
import subprocess

with open("Sponza.gltf", "r") as file:
    gltf = json.load(file)

for image in gltf.get("images", []):
    prev_uri = image["uri"]
    base, _ = os.path.splitext(prev_uri)
    astc_uri = base + ".astc"

    image["uri"] = astc_uri
    image["mimeType"] = "image/astc-6x6"

    try:
        print(f"Converting {prev_uri} -> {astc_uri}")
        subprocess.run(["astcenc.exe", "-cl", prev_uri, astc_uri, "6x6", "-medium"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error converting {prev_uri}: {e}")
        continue

    if os.path.exists(prev_uri):
        os.remove(prev_uri)

with open("Sponza.gltf", "w") as file:
    json.dump(gltf, file, indent=4)