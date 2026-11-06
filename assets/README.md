# Layered kitchen scenery

The game draws one plain architectural background, then five separate
transparent environmental images. Decorative objects are not baked into the
background, and every object has its own texture and draw call.

| File | Environmental element | Horizontal position |
| --- | --- | --- |
| kitchen-background.png | Mint tiles, clear counter, plain wooden ledge | Full window |
| kitchen-pot.png | Lidded cooking pot with handles | 10% |
| kitchen-herb.png | Basil plant in a terracotta pot | 30% |
| kitchen-jar.png | Ceramic cookie jar | 50% |
| kitchen-toaster.png | Toaster with slots, lever and dial | 70% |
| kitchen-mug.png | Coffee mug with handle | 90% |
| food-plate.png | Separate serving plate | Left end of each lane |
| food-bread.png | Bread loaf | Lanes 1 and 4 |
| food-apple.png | Apple with stem and leaf | Lane 2 |
| food-cheese.png | Cheese wedge | Lane 3 |

The five objects sit on the lower kitchen ledge, outside the playable lanes.
The home base now draws each plate and food item as separate image layers with
`drawFoodPlate()`. Gameplay keeps its original base positions, lane selection
and collision boundaries. The intro reuses the same bread, apple and cheese
images. The optional exporter is the only place that constructs these foods
from shapes; the running game only draws their PNG textures.
Change their individual positions and sizes in the `decorations` array near the
top of Renderer.cpp. Transparent bottom padding is measured during loading to
align their bases to the same shelf. Alpha blending preserves the background
around each object and through transparent areas such as handles.

`drawKitchenBackground()` draws only the wall, counter and ledge bands.
`drawKitchenDecorations()` draws the five image layers above them. Main.cpp
calls both before drawing the gameplay scene; the rendering test verifies that
every decoration contributes visible pixels and none reaches into the lanes.

The full-width counter, lane highlights, HUD anchors and resize behavior are
preserved. The texture band boundaries remain `WALL_TEXTURE_END` and
`COUNTER_TEXTURE_END` in Renderer.cpp. All scene textures load once and are
released before the OpenGL context closes. Reconfigure CMake after moving the
source folder, and include the entire assets folder when sharing the game.

The five decoration PNGs and four food/plate PNGs are exported with the same solid-color OpenGL
rectangles, circles and triangles used by Player.cpp and UI.cpp. They use the
existing kitchen/character palette, with no gradients, texture, lighting or
painted highlights. The game continues to load separate transparent PNG files;
the shape models are only in the optional offline exporter.

To regenerate the nine PNGs after changing their offline models:

```sh
cmake -S . -B build -DBUILD_KITCHEN_ASSET_EXPORTER=ON
cmake --build build --target KitchenAssetExporter
./build/KitchenAssetExporter assets
```

The exporter source is tools/ExportKitchenAssets.cpp and is disabled by default
for normal builds. The plain architectural background remains the previously
generated image. Historical generation prompts for the superseded props and
current base background are kept in [ART_PROMPTS.md](ART_PROMPTS.md).

## Picture-based intro

The intro uses eight swatter animation frames, four fly wing frames, five
heading/sign images, five dialogue/recipe-card images, two Skip button images,
and separate steam, contact-flash and progress images. All text in these cards
is baked into the PNGs by the same offline exporter; no live shape text is
drawn in the intro. The five story beats keep their existing timings.

The sign sits on the tiled backsplash. The recipe card and progress indicator
are positioned above the lower ledge so they leave the five props visible.
All assets use the same flat palette and font as the original game.

Regenerating the exporter also rebuilds these `intro-*.png` assets.

## Gameplay characters

`player-TYPE-POSE-FRAME.png` files contain the original swatter, spray and sticky
sheet artwork. Melee and ranged have four walking frames and four attack
frames in normal and duck poses; defense has one stationary frame. Jumping
uses the normal frames at the existing vertical jump offset.

`enemy-TYPE-FRAME.png` has one idle and four attack frames for each enemy,
including the fly gun, ant slingshot and cockroach claymore. Their original
weapons are baked into the sprites. `projectile-spray.png` and the two
`projectile-enemy-TYPE.png` files replace the moving projectile shapes.

The exporter uses the original models in a fixed world-size capture so image
placement preserves hitboxes, attack reach and projectile origins. Runtime
health tint mixes each sprite toward red, then resets before drawing the next
object. Game mechanics and health bars keep their existing code. The running
player and enemy modules no longer construct character models from shapes.

## Image menus, text, HUD and indicators

`font.png` contains the original 5-by-7 lettering and digits in a 16-by-8
character grid. TextRenderer.cpp draws textured glyphs, so currency, prices,
unlock hints and health values remain live without constructing letters from
squares. Static headings and labels are baked into menu and button images.

The four `menu-*.png` backdrops contain their panel, heading and decoration
artwork, with buttons exported separately. Each of the six menu/HUD buttons
has normal and hover PNGs. Restart uses the same blue button in every menu.
The original input handlers, hit areas and menu transitions are unchanged.

`hud-bar.png`, `hud-coins.png` and the four `hud-card-*.png` images replace HUD
panels and card backgrounds. Separate `icon-*.png` assets include colored and
grayscale variants for locked cards. Three `wave-*.png` images contain the
wave announcement labels and panels. Dropped collectibles reuse the coin PNG.

`ui-fill.png` is a white fill texture tinted and resized for health bars, cost
strips, lane separator/selection strips and protection underlines. This keeps
their changing sizes and colors dynamic while using the same image renderer.

The original UI and glyph modeling code is kept only in the offline exporter
sources tools/FlatUIArtwork.cpp and tools/FlatTextArtwork.cpp. The running game
links UI.cpp and TextRenderer.cpp, which draw images. Primitives remain in the
renderer for displaying image quads and in the disabled lab example scene.
