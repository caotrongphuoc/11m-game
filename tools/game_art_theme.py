"""Shared banner background, pixel lettering, and colors for documentation artwork."""

from PIL import Image, ImageDraw


WIDTH, HEIGHT = 1280, 640
WHITE = (235, 248, 242)
GREEN = (90, 221, 152)


def stadium_background():
    canvas = Image.new("RGB", (WIDTH, HEIGHT))
    for y in range(HEIGHT):
        for x in range(WIDTH):
            glow = max(0, 1 - ((x - 640) / 800) ** 2 - ((y - 270) / 440) ** 2)
            shade = 0.85 if y % 3 == 0 else 1
            canvas.putpixel((x, y), tuple(int((a + b * glow) * shade)
                                        for a, b in ((4, 4), (10, 25), (12, 14))))
    draw = ImageDraw.Draw(canvas)
    draw.ellipse((432, 6, 848, 422), outline="#183c30", width=2)
    draw.line((28, 416, 1252, 416), fill="#32644b", width=2)
    draw.rectangle((28, 300, 1252, 416), outline="#204a37", width=2)
    draw.rectangle((28, 318, 124, 416), outline="#204a37", width=2)
    draw.rectangle((1156, 318, 1252, 416), outline="#204a37", width=2)
    return canvas


def lettering(font, text, scale):
    mask = Image.new("1", (len(text) * 6 - 1, 7))
    for index, character in enumerate(text):
        for column in range(5):
            for row in range(7):
                if font[ord(character) * 5 + column] & (1 << row):
                    mask.putpixel((index * 6 + column, row), 1)
    return mask.resize((mask.width * scale, 7 * scale), Image.NEAREST)


def label(canvas, font, text, y, scale=2, color=GREEN, center=WIDTH // 2):
    mask = lettering(font, text, scale)
    canvas.paste(color, (center - mask.width // 2, y), mask)
