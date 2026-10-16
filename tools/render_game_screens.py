#!/usr/bin/env python3
"""Render reproducible OLED previews from the firmware font and bitmaps."""

from pathlib import Path
import re

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
BITMAP_SOURCE = ROOT / "application/sources/app/screens/screens_bitmap.cpp"
FONT_SOURCE = ROOT / "application/sources/driver/Adafruit_oled_drv/glcdfont.cpp"
OUTPUT_DIR = ROOT / "resources/images/screens"
WIDTH = 128
HEIGHT = 64
SCALE = 4


def parse_array(source: Path, symbol: str) -> list[int]:
	text = source.read_text(encoding="utf-8")
	match = re.search(
		rf"\b{re.escape(symbol)}\s*\[\s*\]\s*=\s*\{{(.*?)\}};", text, re.DOTALL
	)
	if match is None:
		raise ValueError(f"cannot find {symbol} in {source}")
	return [int(value, 16) for value in re.findall(r"0x[0-9a-fA-F]+", match.group(1))]


class Oled:
	def __init__(self, font: list[int]):
		self.image = Image.new("1", (WIDTH, HEIGHT), 0)
		self.draw = ImageDraw.Draw(self.image)
		self.font = font

	def pixel(self, x: int, y: int, color: int = 1) -> None:
		if 0 <= x < WIDTH and 0 <= y < HEIGHT:
			self.image.putpixel((x, y), color)

	def line(self, x0: int, y0: int, x1: int, y1: int, color: int = 1) -> None:
		self.draw.line((x0, y0, x1, y1), fill=color)

	def rect(self, x: int, y: int, width: int, height: int, color: int = 1) -> None:
		self.draw.rectangle((x, y, x + width - 1, y + height - 1), outline=color)

	def fill_rect(self, x: int, y: int, width: int, height: int, color: int = 1) -> None:
		self.draw.rectangle((x, y, x + width - 1, y + height - 1), fill=color)

	def circle(self, x: int, y: int, radius: int, color: int = 1) -> None:
		self.draw.ellipse((x - radius, y - radius, x + radius, y + radius), outline=color)

	def triangle(self, points: tuple[tuple[int, int], ...], color: int = 1) -> None:
		self.draw.polygon(points, outline=color)

	def fill_triangle(self, points: tuple[tuple[int, int], ...], color: int = 1) -> None:
		self.draw.polygon(points, fill=color)

	def bitmap(self, x: int, y: int, width: int, height: int, data: list[int]) -> None:
		byte_width = (width + 7) // 8
		if len(data) != byte_width * height:
			raise ValueError(f"bitmap {width}x{height} has {len(data)} bytes")
		for row in range(height):
			for column in range(width):
				value = data[row * byte_width + column // 8]
				if value & (0x80 >> (column & 7)):
					self.pixel(x + column, y + row)

	def text(self, x: int, y: int, value: str, color: int = 1, size: int = 1) -> None:
		for character in value:
			code = ord(character)
			if code >= len(self.font) // 5:
				code = ord("?")
			for column in range(5):
				bits = self.font[code * 5 + column]
				for row in range(8):
					if bits & (1 << row):
						self.fill_rect(x + column * size, y + row * size, size, size, color)
			x += 6 * size

	def save(self, name: str) -> None:
		OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
		preview = self.image.resize((WIDTH * SCALE, HEIGHT * SCALE), Image.NEAREST)
		preview.save(OUTPUT_DIR / name, optimize=True)


def draw_trophy(oled: Oled, y_offset: int = 0) -> None:
	oled.triangle(((8, 40 + y_offset), (11, 46 + y_offset), (5, 46 + y_offset)))
	oled.line(3, 42 + y_offset, 3, 44 + y_offset)
	oled.pixel(4, 42 + y_offset)
	oled.pixel(4, 44 + y_offset)
	oled.line(13, 42 + y_offset, 13, 44 + y_offset)
	oled.pixel(12, 42 + y_offset)
	oled.pixel(12, 44 + y_offset)
	oled.line(8, 46 + y_offset, 8, 49 + y_offset)
	oled.line(5, 49 + y_offset, 11, 49 + y_offset)


def render_menu(font: list[int]) -> Oled:
	oled = Oled(font)
	oled.circle(7, 7, 5)
	oled.line(4, 4, 10, 10)
	oled.line(10, 4, 4, 10)
	oled.text(25, 3, "ELEVEN METER")
	oled.rect(113, 2, 13, 10)
	oled.line(113, 2, 119, 7)
	oled.line(126, 2, 120, 7)
	oled.line(119, 7, 113, 11)
	oled.line(120, 7, 126, 11)
	oled.text(37, 15, "DIFFICULTY")
	for x, width, label, selected in ((1, 36, "EASY", False), (39, 50, "NORMAL", True), (91, 36, "HARD", False)):
		if selected:
			oled.fill_rect(x, 24, width, 12)
		else:
			oled.rect(x, 24, width, 12)
		text_x = x + (width - len(label) * 6) // 2
		oled.text(text_x, 26, label, 0 if selected else 1)
	draw_trophy(oled)
	oled.text(18, 41, "BEST 4 / HARD")
	oled.fill_triangle(((118, 40), (114, 44), (122, 44)))
	oled.fill_triangle(((118, 49), (114, 45), (122, 45)))
	oled.fill_rect(18, 52, 92, 12)
	oled.fill_triangle(((27, 55), (27, 61), (33, 58)), 0)
	oled.text(42, 54, "MODE  START", 0)
	return oled


def render_penalty(font: list[int], bitmaps: dict[str, list[int]],
				   shooter_pose: str = "prepare", ball_position: tuple[int, int] = (87, 47)) -> Oled:
	oled = Oled(font)
	oled.fill_rect(0, 0, 35, 11)
	oled.text(3, 2, "YOU 2", 0)
	oled.fill_rect(93, 0, 35, 11)
	oled.text(96, 2, "CPU 1", 0)
	oled.text(41, 2, "R3/5")
	oled.text(68, 2, "GOAL")
	oled.line(0, 13, 127, 13)
	for x in range(3, 126, 8):
		oled.pixel(x, 11)
	oled.line(4, 16, 4, 25)
	oled.fill_triangle(((5, 16), (5, 21), (11, 18)))
	oled.line(123, 16, 123, 25)
	oled.fill_triangle(((122, 16), (122, 21), (116, 18)))
	oled.bitmap(17, 15, 93, 38, bitmaps["bitmap_penalty_goal"])
	oled.line(0, 53, 127, 53)
	oled.line(16, 53, 0, 63)
	oled.line(111, 53, 127, 63)
	oled.line(0, 63, 127, 63)
	for x in (63, 64, 65):
		oled.pixel(x, 59)
	oled.bitmap(21, 23, 40, 40, bitmaps["bitmap_penalty_keeper_dive_left"])
	shooter_y = 27 if shooter_pose == "kick" else 29
	oled.bitmap(24, shooter_y, 40, 40, bitmaps[f"bitmap_penalty_shooter_{shooter_pose}"])
	oled.bitmap(ball_position[0] - 3, ball_position[1] - 3, 7, 7, bitmaps["bitmap_penalty_ball"])
	return oled


def render_shooter_frames(font: list[int], bitmaps: dict[str, list[int]]) -> None:
	# Original sprites, unchanged: anchor (44, 63), sprite origin (24, 29).
	# Recover with the standing bitmap instead of the reversed-leg follow bitmap.
	poses = (("prepare", "prepare", (64, 59)), ("kick", "kick", (64, 59)), ("recover", "prepare", (75, 53)))
	strip = Image.new("RGB", (WIDTH * SCALE * len(poses), (HEIGHT + 12) * SCALE), "black")
	for index, (label_text, pose, ball) in enumerate(poses):
		oled = render_penalty(font, bitmaps, pose, ball)
		preview = oled.image.resize((WIDTH * SCALE, HEIGHT * SCALE), Image.NEAREST).convert("RGB")
		strip.paste(preview, (index * WIDTH * SCALE, 12 * SCALE))
		label = Oled(font)
		label.text(3, 2, label_text.upper())
		strip.paste(label.image.crop((0, 0, WIDTH, 12)).resize((WIDTH * SCALE, 12 * SCALE), Image.NEAREST).convert("RGB"), (index * WIDTH * SCALE, 0))
	strip.save(OUTPUT_DIR / "scr_game_shooter_frames.png", optimize=True)


def render_rip(font: list[int]) -> Oled:
	oled = Oled(font)
	oled.fill_rect(0, 0, 128, 17)
	oled.text(34, 5, "MATCH OVER", 0)
	oled.rect(33, 22, 62, 23)
	oled.line(33, 22, 43, 31)
	oled.line(95, 22, 85, 31)
	oled.line(43, 31, 85, 31)
	oled.line(43, 31, 33, 44)
	oled.line(85, 31, 95, 44)
	oled.circle(64, 38, 5)
	oled.line(61, 35, 67, 41)
	oled.line(67, 35, 61, 41)
	oled.fill_rect(22, 52, 84, 12)
	oled.fill_triangle(((29, 54), (29, 61), (36, 57)), 0)
	oled.text(42, 54, "CONTINUE", 0)
	return oled


def render_game_over(font: list[int]) -> Oled:
	oled = Oled(font)
	oled.text(37, 2, "GAME OVER")
	oled.line(18, 11, 110, 11)
	oled.fill_rect(16, 15, 96, 12)
	oled.text(31, 17, "PLAYER WINS!", 0)
	oled.text(13, 32, "YOU")
	oled.text(97, 32, "CPU")
	oled.text(46, 28, "3-2", size=2)
	oled.line(13, 40, 30, 40)
	draw_trophy(oled, 2)
	oled.text(18, 43, "BEST 4 / HARD")
	oled.rect(1, 52, 75, 12)
	oled.text(7, 54, "MODE RETRY")
	oled.fill_rect(78, 52, 49, 12)
	oled.text(84, 54, "UP HOME", 0)
	return oled


def main() -> None:
	font = parse_array(FONT_SOURCE, "font")
	bitmap_names = ("bitmap_penalty_goal", "bitmap_penalty_keeper_dive_left", "bitmap_penalty_shooter_prepare", "bitmap_penalty_shooter_kick", "bitmap_penalty_shooter_follow", "bitmap_penalty_ball")
	bitmaps = {name: parse_array(BITMAP_SOURCE, name) for name in bitmap_names}
	render_menu(font).save("scr_game_menu.png")
	render_penalty(font, bitmaps).save("scr_game_penalty.png")
	render_shooter_frames(font, bitmaps)
	render_rip(font).save("scr_game_rip.png")
	render_game_over(font).save("scr_game_over.png")


if __name__ == "__main__":
	main()
