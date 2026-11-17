#!/usr/bin/env python3
"""Render test_contour --dump output as a standalone synthetic comparison.

SVG generation uses only the Python standard library. Optional PNG sheet
output uses Pillow when explicitly requested. Neither output simulates native
XGL lighting or claims to show a Sun screenshot.
"""

import argparse
import base64
import html
import pathlib
import struct
import zlib


CASES = ("panel", "diagonal", "bricks", "circle", "arch")
KINDS = ("source", "uniform", "contour")
WIDTH = 1080
TOP = 125
ROW = 298
SIZE = 256
LEFT = (195, 480, 765)
HEIGHT = TOP + ROW * len(CASES) + 35
BACKGROUND = "#f6f7f8"
INK = "#172536"
WIRE = "#069bab"
TITLE = "Texture contours: synthetic, unlit comparison"
SUBTITLE = "Same limit: 32 triangles per face. Material colors only; no native XGL rendering or lighting."


def read_ppm(path):
    data = path.read_bytes()
    cursor = 0

    def token():
        nonlocal cursor
        while cursor < len(data):
            if data[cursor:cursor + 1] == b"#":
                end = data.find(b"\n", cursor)
                if end < 0:
                    raise ValueError(f"unterminated PPM comment: {path}")
                cursor = end + 1
            elif chr(data[cursor]).isspace():
                cursor += 1
            else:
                break
        start = cursor
        while cursor < len(data) and not chr(data[cursor]).isspace():
            cursor += 1
        if start == cursor:
            raise ValueError(f"incomplete PPM header: {path}")
        return data[start:cursor]

    if token() != b"P6":
        raise ValueError(f"expected binary P6 RGB image: {path}")
    width, height, maximum = (int(token()) for _ in range(3))
    if not (0 < width <= 8192 and 0 < height <= 8192 and maximum == 255):
        raise ValueError(f"unsupported PPM dimensions or range: {path}")
    # Exactly the separator belongs to the header: the first binary pixel
    # channel may itself have the value of an ASCII whitespace character.
    if cursor == len(data) or not chr(data[cursor]).isspace():
        raise ValueError(f"missing PPM data separator: {path}")
    cursor += 2 if data[cursor:cursor + 2] == b"\r\n" else 1
    pixels = data[cursor:]
    if len(pixels) != width * height * 3:
        raise ValueError(f"incorrect PPM byte count: {path}")
    return width, height, pixels


def encode_png(width, height, pixels):
    def chunk(kind, payload):
        return (struct.pack(">I", len(payload)) + kind + payload +
                struct.pack(">I", zlib.crc32(kind + payload) & 0xffffffff))

    stride = width * 3
    rows = b"".join(b"\0" + pixels[y * stride:(y + 1) * stride]
                    for y in range(height))
    header = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) +
            chunk(b"IDAT", zlib.compress(rows, 9)) + chunk(b"IEND", b""))


def read_case(directory, name):
    images = [read_ppm(directory / f"{name}-{kind}.ppm") for kind in KINDS]
    lines = (directory / f"{name}-mesh.txt").read_text().splitlines()
    triangles = int(lines[0])
    mesh = []
    for line in lines[1:]:
        coordinates = [float(value) for value in line.split()]
        if len(coordinates) != 6 or any(not -0.00001 <= value <= 1.00001
                                       for value in coordinates):
            raise ValueError(f"invalid normalized triangle in {name}")
        mesh.append(list(zip(coordinates[::2], coordinates[1::2])))
    if not 0 < triangles <= 32 or len(mesh) != triangles:
        raise ValueError(f"unexpected triangle count in {name}")
    return {"name": name, "triangles": triangles, "images": images, "mesh": mesh}


def write_svg(path, cases):
    lines = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{HEIGHT}" '
             f'viewBox="0 0 {WIDTH} {HEIGHT}">',
             f'<rect width="100%" height="100%" fill="{BACKGROUND}"/>',
             '<g font-family="sans-serif" fill="' + INK + '">',
             '<text x="25" y="38" font-size="25" font-weight="600">' + html.escape(TITLE) + '</text>',
             '<text x="25" y="65" font-size="14">' + html.escape(SUBTITLE) + '</text>']
    headings = ("Source texture", "Existing uniform: 32 triangles", "Contour mesh + wireframe")
    for left, heading in zip(LEFT, headings):
        lines.append(f'<text x="{left}" y="103" font-size="15">{html.escape(heading)}</text>')
    for index, case in enumerate(cases):
        top = TOP + index * ROW
        lines.extend([f'<text x="25" y="{top + 28}" font-size="21" font-weight="600">'
                      f'{html.escape(case["name"].capitalize())}</text>',
                      f'<text x="25" y="{top + 53}" font-size="14">'
                      f'{case["triangles"]} contour triangles</text>'])
        for left, image in zip(LEFT, case["images"]):
            encoded = base64.b64encode(encode_png(*image)).decode("ascii")
            lines.append(f'<image x="{left}" y="{top}" width="{SIZE}" height="{SIZE}" '
                         f'href="data:image/png;base64,{encoded}" style="image-rendering:pixelated"/>')
            lines.append(f'<rect x="{left}" y="{top}" width="{SIZE}" height="{SIZE}" '
                         'fill="none" stroke="#9ba7b5" stroke-width="1"/>')
        for triangle in case["mesh"]:
            points = " ".join(f"{LEFT[2] + x * SIZE:.3f},{top + y * SIZE:.3f}" for x, y in triangle)
            lines.append(f'<polygon points="{points}" fill="none" stroke="{WIRE}" '
                         'stroke-width="0.8" stroke-opacity="0.8"/>')
    lines.extend([f'<text x="25" y="{HEIGHT - 21}" font-size="13">'
                  'Geometric contours stay on the original flat surface; cyan lines show triangle edges.</text>',
                  '</g></svg>'])
    path.write_text("\n".join(lines) + "\n")


def write_png_sheet(path, cases):
    try:
        from PIL import Image, ImageDraw, ImageFont
    except ImportError as error:
        raise SystemExit("PNG sheet output requires Pillow; SVG output was created without it.") from error

    def font(size):
        try:
            return ImageFont.truetype("DejaVuSans.ttf", size)
        except OSError:
            return ImageFont.load_default()

    result = Image.new("RGB", (WIDTH, HEIGHT), BACKGROUND)
    draw = ImageDraw.Draw(result)
    draw.text((25, 15), TITLE, fill=INK, font=font(25))
    draw.text((25, 51), SUBTITLE, fill=INK, font=font(14))
    for left, heading in zip(LEFT, ("Source texture", "Existing uniform: 32 triangles", "Contour mesh + wireframe")):
        draw.text((left, 87), heading, fill=INK, font=font(15))
    for index, case in enumerate(cases):
        top = TOP + index * ROW
        draw.text((25, top + 8), case["name"].capitalize(), fill=INK, font=font(21))
        draw.text((25, top + 36), f'{case["triangles"]} contour triangles', fill=INK, font=font(14))
        for left, (width, height, pixels) in zip(LEFT, case["images"]):
            image = Image.frombytes("RGB", (width, height), pixels)
            result.paste(image.resize((SIZE, SIZE), Image.Resampling.NEAREST), (left, top))
            draw.rectangle((left, top, left + SIZE, top + SIZE), outline="#9ba7b5")
        for triangle in case["mesh"]:
            points = [(LEFT[2] + x * SIZE, top + y * SIZE) for x, y in triangle]
            draw.line(points + [points[0]], fill=WIRE, width=1)
    draw.text((25, HEIGHT - 34),
              "Geometric contours stay on the original flat surface; cyan lines show triangle edges.",
              fill=INK, font=font(13))
    result.save(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=pathlib.Path, help="directory populated by test_contour --dump")
    parser.add_argument("svg", type=pathlib.Path, help="standalone SVG output")
    parser.add_argument("--png", type=pathlib.Path, help="optional PNG comparison sheet (requires Pillow)")
    args = parser.parse_args()
    cases = [read_case(args.directory, name) for name in CASES]
    write_svg(args.svg, cases)
    print(args.svg)
    if args.png:
        write_png_sheet(args.png, cases)
        print(args.png)


if __name__ == "__main__":
    main()
