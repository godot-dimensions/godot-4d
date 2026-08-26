# 4D Editor Icon Colors

Every color used by the SVG icons in this folder, with the role it plays. Godot's own
editor icons color-code by dimension: 2D nodes are blue, 3D nodes are red, and GUI nodes
are green. Godot 4D continues that scheme by claiming **yellow** for 4D, so essentially
all icons here are some shade of yellow, with only two deliberate exceptions.

Shades are listed brightest first within each group. Duplicates across groups are
intentional: the same hex can play more than one role.

## Yellow highlights (bright)

These are the lit faces, near faces, and highlight details of a 4D shape or object.

| Color     | Description                                                                                 |
| --------- | ------------------------------------------------------------------------------------------- |
| `#fff6a2` | Near-white yellow, the brightest value in the set; the highlight of `ConcaveMeshShape4D`.   |
| `#fe9`    | The standard highlight tint, used for the top or lit face of nearly every `*Shape4D` solid. |
| `#fe7`    | A one-off intermediate tint, giving `OrthoplexShape4D` a fourth distinguishable facet.      |

## Yellow base tones (mid)

| Color  | Description                                                                                                                                 |
| ------ | ------------------------------------------------------------------------------------------------------------------------------------------- |
| `#fe5` | The primary 4D color and the single most common value here; the default fill for concrete node and resource icons.                          |
| `#fd3` | A slightly deeper base used for the front or unshaded face of a solid shape, and for `PlaneShape4D`'s infinite plane.                       |
| `#fd0` | The most saturated yellow, used for some letters in `Vector4D` etc, and as the shadow tone in `ConvexHullShape4D` and `ConcaveMeshShape4D`. |

## Yellow shadows (dark)

| Color  | Description                                                                                                                         |
| ------ | ----------------------------------------------------------------------------------------------------------------------------------- |
| `#dc3` | The shading tone, used for the side or bottom face turned away from the light, and for translucent fills.                           |
| `#ba3` | The abstract color: a dimmed, desaturated yellow marking base classes, such as `Mesh4D`, `Shape4D`, `Light4D`, and `PhysicsBody4D`. |

## Non-yellow exceptions

| Color     | Description                                                                                                        |
| --------- | ------------------------------------------------------------------------------------------------------------------ |
| `#e0e0e0` | Neutral light gray for `4D.svg`, which is a UI element rather than a class icon, so it takes no dimensional color. |
| `#8eef97` | Godot's GUI green, used by `QuadSplitContainer.svg`, the only Control-derived icon in this folder.                 |

## Rainbow gradients

The `Material4D` family is the one place a rainbow appears, standing in for arbitrary
per-element coloring. Each gradient is a stack of hard bands rather than a smooth blend.

### Bright rainbow

Used by `TetraMaterial4D` and `WireMaterial4D`, the concrete material resources. Every
band is a fully saturated hue pinned to a `45` floor, keeping the ramp uniformly vivid.

| Color     | Description                                                                                   |
| --------- | --------------------------------------------------------------------------------------------- |
| `#ff4545` | Red, the topmost band.                                                                        |
| `#ffe345` | Yellow.                                                                                       |
| `#80ff45` | Yellow-green.                                                                                 |
| `#45ffa2` | Spring green.                                                                                 |
| `#45d7ff` | Cyan.                                                                                         |
| `#8045ff` | Violet.                                                                                       |
| `#ff4596` | Pink, the bottommost band.                                                                    |
| `#fe5`    | The primary 4D yellow, reused for the plus sign badge that marks these as material resources. |

### Muted rainbow

Used by `Material4D`, the abstract base class. The same hues appear darkened and
desaturated to match the abstract tone, and the yellow band is dropped, leaving six bands
instead of seven.

| Color  | Description                                                                             |
| ------ | --------------------------------------------------------------------------------------- |
| `#b33` | Dull red, the topmost band.                                                             |
| `#5a3` | Olive green.                                                                            |
| `#3b7` | Sea green.                                                                              |
| `#39b` | Steel blue.                                                                             |
| `#63c` | Muted violet.                                                                           |
| `#b37` | Dusty magenta, the bottommost band.                                                     |
| `#ba3` | The abstract color, reused for this icon's plus sign badge instead of the usual `#fe5`. |
