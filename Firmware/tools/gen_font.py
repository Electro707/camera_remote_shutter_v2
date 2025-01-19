from PIL import Image, ImageFont, ImageDraw

chars = list(range(0x20, 0x7F))
totalChars = len(chars)
charWidth = 16
charHeight = 32

img = Image.new('L', (charWidth*totalChars, charHeight), color=0)

font = ImageFont.load("/home/electro/Downloads/spleen-16x32.pil")

draw = ImageDraw.Draw(img)
draw.fontmode = "1"

for i, t in enumerate(chars):
    s = font.getbbox(chr(t))
    print(s)
    print(s[2]-s[0], s[3]-s[1])
    draw.text((charWidth*i, 0), chr(t), font=font, fill='white')

toPrint = f"uint32_t spleenFont32[{totalChars*charWidth}] = {{\n\t"
toPrintLen = 0

for i, t in enumerate(chars):
    for x in range(charWidth):
        colBit = 0
        for y in range(charHeight):
            pixelIdx = x + (charWidth*i), y

            color = img.getpixel(pixelIdx)
            if color != 0:
                colBit |= (1 << y)

        toPrint += f"0x{colBit:04x},"
        toPrintLen += 1
    toPrint += '\n\t'

toPrint = toPrint[:-3]
toPrint += "};"

print(toPrintLen)
print(toPrint)
# img.show()
