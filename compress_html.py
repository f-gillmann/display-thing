import gzip
import os
import glob
import minify_html

Import("env")

def compress_web_files(source, target, env):
    """Minifies, then compresses a file and formats it as a C++ byte array."""
    source_path = str(source[0])
    target_path = str(target[0])

    print(f"Minifying and compressing {source_path} to {target_path}")

    with open(source_path, "r") as f_in:
        content = f_in.read()

    minified_content = minify_html.minify(content, minify_js=False, minify_css=False)
    compressed = gzip.compress(minified_content.encode("utf-8"), compresslevel=9)
    var_name = os.path.basename(source_path).upper().replace('.', '_').replace('-', '_')

    with open(target_path, "w") as f_out:
        f_out.write("#pragma once\n\n")
        f_out.write(f"const uint8_t {var_name}_GZ[] PROGMEM = {{\n  ")
        f_out.write(", ".join([f"0x{b:02x}" for b in compressed]))
        f_out.write("\n};\n")
        f_out.write(f"const size_t {var_name}_GZ_LEN = {len(compressed)};\n")

env.Append(BUILDERS={"Compress": Builder(action=compress_web_files)})

web_dir = os.path.join(env.subst("$PROJECT_SRC_DIR"), "web")
html_files = glob.glob(os.path.join(web_dir, "*.html"))

for html_path in html_files:
    header_path = html_path + ".h"
    env.Compress(header_path, html_path)