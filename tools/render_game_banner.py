#!/usr/bin/env python3
"""Build a self-contained SVG banner with the OLED font and saved gameplay pixels."""

from render_game_screens import FONT_SOURCE, GAMEPLAY_CAPTURE, ROOT, Oled, parse_array, render_gameplay_capture


OUTPUT = ROOT / "resources/images/eleven_meter_banner.svg"


def pixel_path(image) -> str:
    """Encode lit horizontal runs without resampling the source image."""
    commands = []
    for y in range(image.height):
        x = 0
        while x < image.width:
            if not image.getpixel((x, y)):
                x += 1
                continue
            start = x
            while x < image.width and image.getpixel((x, y)):
                x += 1
            width = x - start
            commands.append(f"M{start} {y}h{width}v1h-{width}z")
    return "".join(commands)


def main() -> None:
    font = parse_array(FONT_SOURCE, "font")
    title = Oled(font)
    title.text(0, 0, "ELEVEN")
    title.text(0, 10, "METER")
    gameplay = render_gameplay_capture(font, GAMEPLAY_CAPTURE)
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" width="1200" height="420" viewBox="0 0 1200 420" role="img" aria-labelledby="title description">
  <title id="title">Eleven Meter — penalty shootout on AK Embedded Base Kit</title>
  <desc id="description">Pixel lettering beside an unchanged OLED gameplay capture showing the original shooter, ball, and diving goalkeeper. Five kicks. One winner.</desc>
  <defs>
    <linearGradient id="background" x2="1" y2="1">
      <stop stop-color="#0b1721"/>
      <stop offset="1" stop-color="#102c2a"/>
    </linearGradient>
    <pattern id="grid" width="24" height="24" patternUnits="userSpaceOnUse">
      <path d="M24 0H0V24" fill="none" stroke="#8ecbb7" stroke-opacity=".06"/>
    </pattern>
  </defs>
  <rect width="1200" height="420" rx="20" fill="url(#background)"/>
  <rect width="1200" height="420" rx="20" fill="url(#grid)"/>
  <path d="M32 32h56M32 32v56M1168 388h-56M1168 388v-56" fill="none" stroke="#79f2b0" stroke-width="3"/>
  <g font-family="monospace" font-size="14" letter-spacing="2">
    <text x="66" y="66" fill="#79f2b0">AK EMBEDDED BASE KIT</text>
    <text x="66" y="310" fill="#e4f3ed" font-size="18" letter-spacing="1">FIVE KICKS. ONE WINNER.</text>
    <text x="66" y="344" fill="#9dbab2" letter-spacing="1">A PENALTY SHOOTOUT IN 128 × 64 PIXELS</text>
    <text x="712" y="66" fill="#9dbab2">ON-DEVICE GAMEPLAY</text>
  </g>
  <path d="{pixel_path(title.image)}" transform="translate(66 100) scale(9)" fill="#f2fff8" shape-rendering="crispEdges"/>
  <rect x="688" y="84" width="432" height="248" rx="14" fill="#07100f" stroke="#35594f" stroke-width="2"/>
  <circle cx="704" cy="100" r="3" fill="#79f2b0"/>
  <circle cx="1104" cy="316" r="3" fill="#35594f"/>
  <rect x="712" y="112" width="384" height="192" fill="#000"/>
  <path d="{pixel_path(gameplay.image)}" transform="translate(712 112) scale(3)" fill="#fff" shape-rendering="crispEdges"/>
  <g font-family="monospace" font-size="12" fill="#9dbab2" letter-spacing="1">
    <text x="712" y="362">STM32L151</text>
    <text x="856" y="362">ACTIVE KERNEL</text>
    <text x="1040" y="362">C++</text>
  </g>
</svg>
'''
    OUTPUT.write_text(svg, encoding="utf-8")
    print(f"Rendered {OUTPUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
