# Eleven Meter 2:1 banners

- [Animated GIF](eleven_meter_banner.gif): the README banner.
- [Static PNG cover](eleven_meter_banner_2x1.png): 1280×640 (2:1), suitable for sharing
  or uploading as a repository social preview. Clicking the README banner opens it;
  adding this file does not automatically change GitHub repository settings.

The PNG is a pixel-identical RGB export of the GIF's first frame, with no cropping,
stretching, new sprites, or edits to the original captures.

[`eleven_meter_banner.gif`](eleven_meter_banner.gif) is a generated documentation banner,
not a new firmware screen or camera recording. It follows the visual structure of the
[Zomwar README banner](https://github.com/caotrongphuoc/zomwar-game): a 2:1 canvas, large
pixel title, a sprite row, and five preview panels. No Zomwar artwork or source code is
copied; the green stadium treatment and composition are implemented locally.

The banner uses the existing Eleven Meter OLED font, isolated Shooter/Ball/Keeper GIFs,
animated menu, round-4 UART gameplay playback, static Full Time/final-score previews,
and the repository's AK Base Kit photograph. The source bitmap arrays are unchanged.
The sprite row cycles isolated documentation animations independently; only the gameplay
panel represents the saved round. The result panel is an illustrative screen preview.

The 1280×640 GIF contains 23 frames with 160 ms holds, looping in 3.68 seconds. A fixed
128-color palette avoids frame-to-frame color shifts. Sprite and screen resizing uses
nearest-neighbor sampling; the kit photograph uses Lanczos resizing.

Development renderers were removed during engineer-approved final cleanup. To regenerate
these assets, restore the former banner, screen, object, and shared-theme renderers from
Git history; Pillow is required. The GIF, PNG, source previews, and captures remain here.

The banner and the README's [screen cards](screens/framed/README.md) use the same
background, colors, and pixel lettering. Cards use child crops of that background.
The banner uses raw previews rather than framed outputs, avoiding nested frames.
