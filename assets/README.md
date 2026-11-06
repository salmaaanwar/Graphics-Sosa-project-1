# Kitchen background

`kitchen-background.png` is the shared backdrop for the menu, intro and game.
It fills the current window, cropping around its center to keep its proportions.
The four-lane board, food target, characters and controls retain their existing
world coordinates. Resizing never creates new playable lanes or placement areas.

The image loads once with `external/stb_image.h` (the same library supplied in the
Lab 4 model template). `main.cpp` draws it before the existing scene, using the
existing rectangle geometry and a texture/solid-color shader switch. Textures
are released before the OpenGL context closes.

Reconfigure CMake after moving the source folder so `GAME_ASSET_DIR` points to
this folder. Include the assets folder when sharing the source.

Artwork generated with the built-in image-generation tool, using a screenshot
of the original intro as the style reference. Prompt: a full-bleed, front-on
geometric 2D kitchen, pale mint tiles, warm brown cabinetry, cream counter trim,
an uncluttered center, and a herb pot, canister, coral toaster and mug near the
outer sides; simple muted flat colors, no characters, text, UI or lane markings.
