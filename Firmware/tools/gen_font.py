#!/bin/python3
from PIL import Image, ImageFont, ImageDraw

chars = list(range(0x20, 0x7F))
totalChars = len(chars)

def generateFontLUT(varName, fontFile, charWidth, charHeight):
    img = Image.new('L', (charWidth*totalChars, charHeight), color=0)

    font = ImageFont.load(fontFile)

    draw = ImageDraw.Draw(img)
    draw.fontmode = "1"

    for i, t in enumerate(chars):
        s = font.getbbox(chr(t))
        # print(s)
        # print(s[2]-s[0], s[3]-s[1])
        draw.text((charWidth*i, 0), chr(t), font=font, fill='white')

    toPrint = f"const uint32_t {varName}[{totalChars*charWidth}] = {{\n\t"
    toPrintLen = 0

    for i, t in enumerate(chars):
        for x in range(charWidth):
            colBit = 0
            for y in range(charHeight):
                pixelIdx = x + (charWidth*i), y

                color = img.getpixel(pixelIdx)
                if color != 0:
                    colBit |= (1 << y)

            toPrint += f"0x{colBit:08x},"
            toPrintLen += 1
        toPrint += '\n\t'

    toPrint = toPrint[:-3]
    toPrint += "};\n\n"

    # print(toPrintLen)
    return toPrint
    # img.show()


fileC = "#ifndef _GC9A01_FONT_H\n#define _GC9A01_FONT_H\n\n#include <stdint.h>\n"
fileC += generateFontLUT('spleenFont32', "/home/electro/Downloads/spleen-16x32.pil", 16, 32)
fileC += generateFontLUT('spleenFont24', "/home/electro/Downloads/spleen-12x24.pil", 12, 24)
fileC += generateFontLUT('spleenFont16', "/home/electro/Downloads/spleen-8x16.pil", 8, 16)
fileC += "#endif"
print(fileC)
