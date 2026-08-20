# 4D volumetric clouds

`VolumetricCloudMaterial4D` renders a procedurally generated 4D density field as a layer above a `WorldEnvironment4D`. It can be combined with a gradient, physical, or plain 4D sky, or rendered over the black fallback used when there is no base sky.

## Model

For a camera at 4D position `c` with basis `B`, each visible sky direction `d` defines the 4D ray:

```
r(s) = c + B * (d.x, d.y, d.z, 0) * s
```

The shader intersects that ray with a slab bounded along the environment's local Y axis, then integrates density and lighting along the intersecting segment. Moving or rotating the 4D camera therefore selects a genuinely different 3D cross-section of one continuous 4D cloud volume.

The density comes from textureless 4D simplex noise. Its dominant shape is a configurable three-octave fractal, followed by an optional finer erosion sample. Successive shape octaves have roughly twice the spatial frequency and use `shape_fractal_strength` as their geometric amplitude multiplier. Setting that property to zero skips both additional shape octaves. The approximate self-shadow probe uses only the first shape octave to contain the performance cost. Time evolution combines 4D wind advection with a slow 4D coordinate rotation. Both use the shared double-precision `RenderingServer4D.render_time`, allowing games to synchronize or restore every time-based 4D rendering effect from one clock. The CPU reduces that time to small per-octave wind offsets and an evolution cosine/sine pair before sending ordinary floats to the shader.

Opacity uses Beer-Lambert extinction. Lighting combines a configurable ambient term with the first enabled `DirectionalLight4D`, a 4D phase approximation, and one coarse density sample toward the light for self-shadowing. Scene geometry does not cast shadows into the cloud volume.

## Rendering paths

Forward+ and Mobile evaluate the selected cloud sampler in Godot's quarter-resolution sky subpass and composite the result over the full-resolution base sky. The Compatibility renderer uses a full-resolution fallback because sky subpasses do not compile reliably on the oldest supported Godot branches.

`sampling_method` selects one of two algorithms. `World Grid` integrates segments bounded by crossings of a stationary 4D grid. Its grid spacing is `maximum_ray_distance / sampling_steps`, while the number of segments traversed by each ray also depends on its 4D direction. Lower sampling settings produce larger, softer grid cells, which can be more apparent at low cloud coverage. `Jittered Raymarch` divides each ray's cloud-layer intersection into exactly `sampling_steps` intervals and offsets the sample positions per pixel. It preserves finer variation along the ray at high settings, while insufficient sampling density appears as grain. The shader evaluates the selected method for each ray.

Both methods filter noise octaves that are too fine for their sampling interval toward the noise mean. Increasing `sampling_steps` therefore restores small structure as well as improving integration accuracy.

The cloud subpass packs ambient scattering, direct scattering, and opacity into RGB. This is necessary because the Mobile sky subpass buffer may not preserve alpha.

Changing an animated sky uniform can cause Godot to update the sky radiance cubemap every frame. Disable `animation_enabled` for static clouds, or disable `affect_radiance` to omit the cloud ray march from cubemap passes. The latter still allows the visible clouds to animate.

Automatic composition currently supports `GradientSkyMaterial4D`, `PhysicalSkyMaterial4D`, `PlainSkyMaterial4D`, and no base sky. A custom `SkyMaterial4D` remains usable by itself, but its arbitrary shader code cannot be automatically merged with the cloud shader.

The generated C++ embeds each base-sky prefix, the shared noise and density code, the shared cloud rendering code, and each sky entry point as separate string literals. `VolumetricCloudMaterial4D` concatenates these pieces when it initializes its shaders. This keeps the repeated cloud implementation in two shared constants and keeps every generated literal below MSVC's size limit.

## References and attribution

- Ian McEwan et al., [Efficient computational noise in GLSL](https://arxiv.org/abs/1204.1461).
- Ashima Arts and Stefan Gustavson, [webgl-noise](https://github.com/ashima/webgl-noise). `simplex_noise_4d.inc.glsl` is the only implementation file derived from webgl-noise and contains the applicable MIT license notice. The other cloud implementation files were written independently for Godot 4D.
- Andrew Schneider and Nathan Vos, [The Real-time Volumetric Cloudscapes of Horizon: Zero Dawn](https://advances.realtimerendering.com/s2015/The%20Real-time%20Volumetric%20Cloudscapes%20of%20Horizon%20-%20Zero%20Dawn%20-%20ARTR.pdf), used as general background on density shaping, ray marching, and performance trade-offs.
- Huw Bowles and Daniel Zimmermann, [A Novel Sampling Algorithm for Fast and Stable Real-Time Volume Rendering](https://advances.realtimerendering.com/s2015/), used as background on deterministic volume sampling. The World Grid sampler here is an independent design rather than an implementation of their structured sampling algorithm.
- [Godot sky shader documentation](https://docs.godotengine.org/en/stable/tutorials/shaders/shader_reference/sky_shader.html), for sky subpass and radiance behavior.

The cloud model, 4D coordinate evolution, shader composition, and renderer fallbacks in this folder were independently designed for Godot 4D. No Unreal Engine shader implementation was copied.
