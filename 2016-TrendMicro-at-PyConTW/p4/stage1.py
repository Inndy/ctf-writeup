import mclevel  # https://github.com/mcedit/pymclevel.git
import PIL.Image

level = mclevel.fromFile("level.dat")
s = set() # 3, 45, 99

# search for height

# slow!!! QQ
for h in range(128):
    for a in range(512):
        for b in range(512):
            if level.blockAt(a, h, b) == 49:
                s.add(h)

def dump_at_height(h, fn):
    img = PIL.Image.new('RGB', (512,512))

    for x in range(512):
        for y in range(512):
            if level.blockAt(x, 3, y) != 49: img.putpixel((x, y), (255, 255,255))
    img.save(open(fn, 'wb'), 'PNG')

for i, h in enumerate(s):
    dump_at_height(h, '%d.png' % (i+1))
