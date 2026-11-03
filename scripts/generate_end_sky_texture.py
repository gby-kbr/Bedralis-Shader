#!/usr/bin/env python3
"""Generate the 360 degree equirectangular outer-space texture for The End.

Output: assets/textures/environment/EndSkyTexture.png (2048x1024, RGB).

Everything is computed from 3D view directions, so the left and right edges meet
exactly and nothing is stretched at the poles. Layers:
  - a galactic band with a bright core and dust lanes,
  - two faint nebulae (violet and teal, matching the End palette),
  - three star layers (faint, medium, rare bright) with real star colors.
Deterministic (fixed seed). Requires numpy and Pillow.

Usage: python3 scripts/generate_end_sky_texture.py [output.png]
"""
import sys
import numpy as np
from PIL import Image

W, H = 2048, 1024
SEED = 20261103
OUT = sys.argv[1] if len(sys.argv) > 1 else "assets/textures/environment/EndSkyTexture.png"

# ----------------------------------------------------------------------------- hashing / noise
def hash3(ix, iy, iz, salt):
    """Integer lattice hash -> float in [0, 1). All arguments are int arrays."""
    h = (ix.astype(np.uint32) * np.uint32(73856093)) ^ (iy.astype(np.uint32) * np.uint32(19349663)) \
        ^ (iz.astype(np.uint32) * np.uint32(83492791)) ^ np.uint32((salt * 2654435761) & 0xFFFFFFFF)
    h ^= h >> np.uint32(13)
    h *= np.uint32(1274126177)
    h ^= h >> np.uint32(16)
    h *= np.uint32(2246822519)
    h ^= h >> np.uint32(13)
    return (h & np.uint32(0xFFFFFF)).astype(np.float32) / np.float32(0x1000000)

def value_noise(p, salt):
    """Smooth 3D value noise, p has shape (..., 3)."""
    i = np.floor(p).astype(np.int64)
    f = (p - i).astype(np.float32)
    f = f * f * (3.0 - 2.0 * f)
    out = 0.0
    for dx in (0, 1):
        for dy in (0, 1):
            for dz in (0, 1):
                w = (f[..., 0] if dx else 1 - f[..., 0]) * (f[..., 1] if dy else 1 - f[..., 1]) \
                    * (f[..., 2] if dz else 1 - f[..., 2])
                out = out + w * hash3(i[..., 0] + dx, i[..., 1] + dy, i[..., 2] + dz, salt)
    return out

def fbm(p, salt, octaves=5, lac=2.03, gain=0.5):
    total, amp, norm = 0.0, 1.0, 0.0
    for o in range(octaves):
        total = total + amp * value_noise(p * (lac ** o) + o * 17.3, salt + o)
        norm += amp
        amp *= gain
    return total / norm

def ss(a, b, x):
    t = np.clip((x - a) / (b - a), 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)

# ----------------------------------------------------------------------------- stars
STAR_COLORS = np.array([[0.62, 0.74, 1.00],   # blue-white
                        [0.85, 0.90, 1.00],   # white
                        [1.00, 0.95, 0.82],   # yellow-white
                        [1.00, 0.80, 0.55],   # orange
                        [1.00, 0.62, 0.45]],  # red-orange
                       np.float32)
STAR_COLOR_P = np.array([0.20, 0.35, 0.22, 0.15, 0.08])

def star_layer(d, radius, prob, sigma, bright_lo, bright_hi, power, salt, band_boost=None):
    """Stars on a lattice shell around the origin. d: (...,3) unit directions."""
    cell_p = d * radius
    base = np.floor(cell_p).astype(np.int64)
    acc = np.zeros(d.shape, np.float32)
    cum = np.cumsum(STAR_COLOR_P)
    for dx in (-1, 0, 1):
        for dy in (-1, 0, 1):
            for dz in (-1, 0, 1):
                ix, iy, iz = base[..., 0] + dx, base[..., 1] + dy, base[..., 2] + dz
                pr = prob if band_boost is None else prob * band_boost
                present = hash3(ix, iy, iz, salt) < pr
                if not present.any():
                    continue
                jx = hash3(ix, iy, iz, salt + 1); jy = hash3(ix, iy, iz, salt + 2); jz = hash3(ix, iy, iz, salt + 3)
                p = np.stack([ix + jx, iy + jy, iz + jz], -1).astype(np.float32)
                r = np.linalg.norm(p, axis=-1)
                along = np.sum(p * d, -1)
                perp = p - along[..., None] * d
                ang = np.linalg.norm(perp, axis=-1) / np.maximum(r, 1e-3)
                mag = hash3(ix, iy, iz, salt + 4)
                bright = bright_lo + (bright_hi - bright_lo) * mag ** power
                sg = sigma * (1.0 + 1.5 * mag ** (power * 2))     # bright stars are a little bigger
                prof = np.exp(-0.5 * (ang / sg) ** 2) + 0.06 * np.exp(-(ang / (sg * 5.0)))   # core + faint halo
                cidx = np.searchsorted(cum, hash3(ix, iy, iz, salt + 5) * cum[-1])
                col = STAR_COLORS[np.clip(cidx, 0, 4)]
                w = (present & (along > 0)) * bright * prof
                acc += w[..., None] * col
    return acc

# ----------------------------------------------------------------------------- image
def render_rows(j0, j1):
    jj, ii = np.mgrid[j0:j1, 0:W]
    u = (ii + 0.5) / W
    v = (jj + 0.5) / H
    ang = (u - 0.5) * 2.0 * np.pi
    th = v * np.pi
    d = np.stack([np.sin(ang) * np.sin(th), np.cos(th), np.cos(ang) * np.sin(th)], -1).astype(np.float32)

    # Galactic band: a great circle tilted away from the equator.
    n = np.array([0.30, 0.86, 0.41], np.float32); n /= np.linalg.norm(n)
    lat = np.sum(d * n, -1)
    core_dir = np.cross(n, np.array([0.0, 0.0, 1.0], np.float32)); core_dir /= np.linalg.norm(core_dir)
    along = np.sum(d * core_dir, -1)

    warp = 0.20 * (fbm(d * 2.2, SEED + 10, 3) - 0.5)
    band = np.exp(-((lat + warp) / 0.17) ** 2)
    mist = fbm(d * 3.0 + 4.0, SEED + 20, 5)
    cloud = band * (0.45 + 0.9 * ss(0.30, 0.80, mist))
    dust = ss(0.52, 0.72, fbm(d * 5.5 + 9.0, SEED + 30, 5)) * np.exp(-((lat + warp * 0.5) / 0.12) ** 2)
    core = np.exp(-(along / 0.65) ** 2) * np.exp(-((lat + warp) / 0.22) ** 2)
    glow = cloud * (1.0 - 0.80 * dust) + 0.9 * core * (1.0 - 0.6 * dust)

    band_col = np.stack([0.50, 0.40, 0.66], 0).astype(np.float32)       # cool violet
    core_col = np.array([0.95, 0.72, 0.62], np.float32)                 # warm core
    base = glow[..., None] * (band_col * (1 - core[..., None] * 0.7) + core_col * (core[..., None] * 0.7)) * 0.24

    # Nebulae: violet and teal, independent of the band, very soft.
    n1 = ss(0.52, 0.80, fbm(d * 1.5 + 21.0, SEED + 40, 5)) ** 1.6
    n2 = ss(0.56, 0.82, fbm(d * 1.9 + 53.0, SEED + 50, 5)) ** 1.6
    neb = n1[..., None] * np.array([0.34, 0.10, 0.46], np.float32) * 0.55 \
        + n2[..., None] * np.array([0.06, 0.26, 0.34], np.float32) * 0.45

    # Stars: more of them inside the band, rare bright ones everywhere.
    boost = 1.0 + 1.6 * band
    s = star_layer(d, 70.0, 0.016, 0.0022, 0.12, 0.60, 3.0, SEED + 100, boost)
    s += star_layer(d, 38.0, 0.009, 0.0022, 0.30, 1.00, 4.0, SEED + 200, boost)
    s += star_layer(d, 16.0, 0.006, 0.0026, 0.60, 2.00, 5.0, SEED + 300)
    # Unresolved faint stars in the band look like a fine glow.
    fine = ss(0.45, 0.85, fbm(d * 40.0, SEED + 60, 2)) * band * 0.06

    color = base + neb + s + fine[..., None] * np.array([0.8, 0.8, 1.0], np.float32)
    # A touch of deep violet so the darkest sky is not dead black (matches the End fog).
    color += np.array([0.010, 0.004, 0.020], np.float32)
    return color

def main():
    rows = []
    step = 64
    for j in range(0, H, step):
        rows.append(render_rows(j, min(j + step, H)))
        print(f"row {j}/{H}", end="\r", flush=True)
    img = np.concatenate(rows, 0)
    # Soft shoulder so bright stars do not clip harshly, then to 8-bit.
    img = 1.0 - np.exp(-img * 1.25)
    img = np.clip(img, 0, 1) ** (1.0 / 1.15)
    Image.fromarray((img * 255 + 0.5).astype(np.uint8), "RGB").save(OUT, optimize=True)
    print(f"\nwrote {OUT}  mean={img.mean():.4f}  max={img.max():.3f}")

if __name__ == "__main__":
    main()
