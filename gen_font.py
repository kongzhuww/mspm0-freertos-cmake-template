import urllib.request
import re

url = 'https://raw.githubusercontent.com/torvalds/linux/master/lib/fonts/font_8x16.c'
req = urllib.request.urlopen(url)
data = req.read().decode('utf-8')

# The data is formatted like:
#	/* 32 0x20 ' ' */
#	0x00, /* 00000000 */
#	0x00, /* 00000000 */

chars = {}
lines = data.split('\n')
current_char = -1
byte_count = 0
char_data = []

for line in lines:
    line = line.strip()
    m = re.match(r'/\*\s+(\d+)\s+0x[0-9a-fA-F]+\s+.*?\*/', line)
    if m:
        current_char = int(m.group(1))
        char_data = []
        byte_count = 0
        continue
    
    if current_char >= 0 and byte_count < 16:
        m2 = re.match(r'(0x[0-9a-fA-F]+),', line)
        if m2:
            char_data.append(int(m2.group(1), 16))
            byte_count += 1
            if byte_count == 16:
                chars[current_char] = char_data
                current_char = -1

out_lines = []
for code in range(32, 127):
    if code not in chars:
        print(f"Missing character {code}")
        row = [0]*16
    else:
        row = chars[code]
    
    top = []
    bottom = []
    for c in range(8):
        t_val = 0
        b_val = 0
        for r in range(8):
            bit = (row[r] >> (7 - c)) & 1
            t_val |= (bit << r)
            
            bit_b = (row[r+8] >> (7 - c)) & 1
            b_val |= (bit_b << r)
        top.append(t_val)
        bottom.append(b_val)
    
    hex_str = ",".join(f"0x{v:02X}" for v in top + bottom)
    char_disp = chr(code)
    # Escape backslash and quotes for the comment
    if char_disp == '\\': char_disp = '\\\\'
    out_lines.append(f"    {{{hex_str}}}, /* {char_disp} {code} */")

with open('d:/mspm0/components/oled/src/oled.c', 'r', encoding='utf-8') as f:
    content = f.read()

# Replace the array
start_marker = "static const uint8_t oled_font_8x16[][16] = {"
end_marker = "};"
start_idx = content.find(start_marker)
if start_idx != -1:
    end_idx = content.find(end_marker, start_idx)
    if end_idx != -1:
        new_content = content[:start_idx + len(start_marker)] + "\n" + "\n".join(out_lines) + "\n" + content[end_idx:]
        with open('d:/mspm0/components/oled/src/oled.c', 'w', encoding='utf-8') as f:
            f.write(new_content)
        print("Updated oled.c successfully")
else:
    print("Could not find array in oled.c")
