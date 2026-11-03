#!/usr/bin/env python3
"""Generate the 360 degree equirectangular sky texture for The End.

Output: assets/textures/environment/end_sky.png (2048x1024, RGB).

Everything is computed from 3D view directions, so the left and right edges meet
exactly and nothing is stretched at the poles. Layers, kept deliberately subdued:
  - a dark navy-violet sky with a thin glowing horizon,
  - stars (faint, medium, rare bright) with real star colors, plus a faint galactic band
    and two soft nebulae,
  - a black hole just above the horizon: dark shadow, thin photon ring, lensed arc and a
    thin accretion disc seen almost edge-on,
  - a dark sea of clouds below the horizon, lit from the side of the black hole,
    with a dim reflection of the ring.
Deterministic (fixed seed). Requires numpy and Pillow.

Usage: python3 scripts/generate_end_sky_texture.py [output.png]
"""
import sys
import numpy as np
from PIL import Image

W, H = 2048, 1024
SEED = 20261103
OUT = sys.argv[1] if len(sys.argv) > 1 else "assets/textures/environment/end_sky.png"

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

# ----------------------------------------------------------------------------- black hole
BH_AZ = -1.15      # azimuth (radians, same convention as the shader: atan2(x, z))
BH_EL = 0.060      # elevation of the center above the horizon (radians)
BH_RS = 0.105      # radius of the shadow (radians)

def bh_basis():
    c = np.array([np.sin(BH_AZ) * np.cos(BH_EL), np.sin(BH_EL), np.cos(BH_AZ) * np.cos(BH_EL)], np.float32)
    up = np.array([0.0, 1.0, 0.0], np.float32)
    e1 = np.cross(c, up); e1 /= np.linalg.norm(e1)
    e2 = up - np.dot(up, c) * c; e2 /= np.linalg.norm(e2)
    return c, e1, e2

def black_hole(d):
    """Returns (emission rgb, shadow mask 0..1) for directions d."""
    c, e1, e2 = bh_basis()
    dc = np.sum(d * c, -1)
    ok = dc > 0.2
    x = np.where(ok, np.sum(d * e1, -1) / np.maximum(dc, 0.2), 9.0)
    y = np.where(ok, np.sum(d * e2, -1) / np.maximum(dc, 0.2), 9.0)
    R = BH_RS
    r = np.sqrt(x * x + y * y)
    shadow = 1.0 - ss(R * 0.93, R * 1.03, r)

    # Thin photon ring, brighter on the approaching side (right).
    ring = np.exp(-((r - 1.10 * R) / (0.075 * R)) ** 2) * (0.65 + 0.35 * np.clip(x / np.maximum(r, 1e-4), -1, 1))
    # Lensed back of the disc: a wider arc that bends over the top of the shadow.
    top = ss(-0.25, 0.45, y / R)
    arc = np.exp(-((r - 1.55 * R) / (0.40 * R)) ** 2) * top
    arc += 0.35 * np.exp(-((r - 2.2 * R) / (0.9 * R)) ** 2) * top * top

    # Thin disc seen almost edge-on, with streaks along it.
    ax = np.abs(x)
    thick = 0.11 * R * (1.0 + 0.30 * ax / R)
    prof = np.exp(-(y / thick) ** 2)
    radial = np.exp(-ax / (4.2 * R)) * np.clip(1.25 * R / np.maximum(ax, 1e-4), 0, 1) ** 0.8
    gate = ss(1.05 * R, 1.35 * R, ax)
    streak = fbm(np.stack([x * 9.0, y * 140.0, np.zeros_like(x)], -1), SEED + 700, 3)
    disc = prof * radial * gate * (0.55 + 0.9 * streak)

    glow = 0.13 * np.exp(-(r / (3.0 * R)) ** 2) + 0.05 * np.exp(-(r / (9.0 * R)) ** 2)

    ring_c = np.array([0.80, 0.84, 1.00], np.float32)
    disc_c = np.array([0.66, 0.68, 1.00], np.float32)
    glow_c = np.array([0.20, 0.26, 0.62], np.float32)
    emit = ring[..., None] * ring_c * 0.70 + arc[..., None] * ring_c * 0.34 \
        + disc[..., None] * disc_c * 0.80 + glow[..., None] * glow_c
    emit = emit * ok[..., None]
    shadow = np.where(ok, shadow, 0.0)
    return emit.astype(np.float32), shadow.astype(np.float32)

# ----------------------------------------------------------------------------- image
def render_rows(j0, j1):
    jj, ii = np.mgrid[j0:j1, 0:W]
    u = (ii + 0.5) / W
    v = (jj + 0.5) / H
    ang = (u - 0.5) * 2.0 * np.pi
    th = v * np.pi
    d = np.stack([np.sin(ang) * np.sin(th), np.cos(th), np.cos(ang) * np.sin(th)], -1).astype(np.float32)
    y = d[..., 1]

    # --- sky: navy gradient, brighter toward the horizon
    hz = np.exp(-np.abs(y) / 0.22)
    base = np.array([0.010, 0.012, 0.030], np.float32) + hz[..., None] * np.array([0.020, 0.036, 0.085], np.float32)

    # --- faint galactic band and nebulae (subtle)
    n = np.array([0.30, 0.86, 0.41], np.float32); n /= np.linalg.norm(n)
    lat = np.sum(d * n, -1)
    warp = 0.20 * (fbm(d * 2.2, SEED + 10, 3) - 0.5)
    band = np.exp(-((lat + warp) / 0.17) ** 2)
    mist = fbm(d * 3.0 + 4.0, SEED + 20, 5)
    dust = ss(0.52, 0.72, fbm(d * 5.5 + 9.0, SEED + 30, 5)) * np.exp(-((lat + warp * 0.5) / 0.12) ** 2)
    glow = band * (0.45 + 0.9 * ss(0.30, 0.80, mist)) * (1.0 - 0.8 * dust)
    band_rgb = glow[..., None] * np.array([0.42, 0.38, 0.62], np.float32) * 0.10
    n1 = ss(0.55, 0.82, fbm(d * 1.5 + 21.0, SEED + 40, 5)) ** 1.6
    n2 = ss(0.58, 0.84, fbm(d * 1.9 + 53.0, SEED + 50, 5)) ** 1.6
    neb = n1[..., None] * np.array([0.30, 0.10, 0.42], np.float32) * 0.22 \
        + n2[..., None] * np.array([0.06, 0.22, 0.30], np.float32) * 0.20

    # --- stars (fewer and finer than before)
    boost = 1.0 + 1.0 * band
    st = star_layer(d, 70.0, 0.012, 0.0020, 0.12, 0.55, 3.0, SEED + 100, boost)
    st += star_layer(d, 38.0, 0.007, 0.0020, 0.28, 0.95, 4.0, SEED + 200, boost)
    st += star_layer(d, 16.0, 0.004, 0.0024, 0.55, 1.70, 5.0, SEED + 300)

    # --- horizon limb: thin blue glow, stronger toward the black hole
    ch = np.array([np.sin(BH_AZ), np.cos(BH_AZ)], np.float32)
    hh = np.stack([d[..., 0], d[..., 2]], -1)
    hh = hh / np.maximum(np.linalg.norm(hh, axis=-1, keepdims=True), 1e-4)
    toward = np.clip(np.sum(hh * ch, -1), 0, 1) ** 2
    limb = np.exp(-np.abs(y) / 0.016) * (0.55 + 1.6 * toward)
    limb_rgb = limb[..., None] * np.array([0.050, 0.100, 0.260], np.float32)

    # --- sea of clouds below the horizon
    below = y < 0.0
    ys = np.minimum(y, -0.035)
    t = 1.0 / (-ys)
    P = np.stack([d[..., 0] * t, d[..., 2] * t, np.zeros_like(t)], -1) * 1.5
    q = fbm(P * 0.5 + 7.0, SEED + 800, 3)
    Pw = P + np.stack([q - 0.5, (q - 0.5) * 0.7, np.zeros_like(q)], -1) * 1.3
    def cloud_height(Pq):
        return 0.62 * fbm(Pq, SEED + 810, 5) + 0.38 * fbm(Pq * 3.7 + 11.0, SEED + 815, 4)
    cn = cloud_height(Pw)
    Ldir = np.array([np.sin(BH_AZ), np.cos(BH_AZ)], np.float32) * 0.07
    cl_n = cloud_height(Pw + np.array([Ldir[0], Ldir[1], 0.0], np.float32))
    dens = ss(0.43, 0.56, cn)
    puff = fbm(Pw * 3.2 + 3.0, SEED + 820, 3)
    dens = np.clip(dens * (0.70 + 0.6 * puff), 0.0, 1.0)
    lit = np.clip(0.50 + (cn - cl_n) * 14.0, 0.0, 1.0) * (0.35 + 0.65 * dens)
    near = np.clip(np.sum(hh * ch, -1), 0, 1)
    bright = 0.45 + 1.15 * near ** 2
    sea = np.array([0.010, 0.016, 0.040], np.float32)
    c_shadow = np.array([0.030, 0.044, 0.100], np.float32)
    c_lit = np.array([0.30, 0.37, 0.60], np.float32)
    cloud = sea + (c_shadow + (c_lit - c_shadow) * lit[..., None] * bright[..., None] - sea) * dens[..., None]
    w = (1.0 - ss(-0.22, -0.015, y)) * below          # clouds fade into the haze toward the horizon
    cover = w * dens                          # how much of the sky below is hidden by cloud

    # --- black hole (emission) and its dim reflection on the clouds
    emit, shadow = black_hole(d)
    dm = d.copy(); dm[..., 1] = -dm[..., 1]
    emit_m, _ = black_hole(dm)

    sky = (base + band_rgb + neb) * (1.0 - shadow[..., None])
    stars_vis = np.where(below, 1.0 - cover, 1.0)
    sky = sky + st * (stars_vis * (1.0 - shadow))[..., None]
    sky = sky + emit * np.where(below, 1.0 - np.clip(w * (0.45 + 0.55 * dens), 0, 1), 1.0)[..., None]
    sky = sky + limb_rgb
    # Below the horizon: haze first, then the clouds in front of it.
    haze = base + limb_rgb
    col_below = haze * (1.0 - w[..., None]) + cloud * w[..., None]
    col_below = col_below + emit_m * (0.22 * (w * (0.3 + 0.7 * lit))[..., None]) * 0.0
    refl = emit_m * (0.20 * w * (0.25 + 0.75 * dens))[..., None]
    col_below = col_below + refl + sky * (1.0 - w[..., None]) * 0.0
    out = np.where(below[..., None], col_below + (sky - haze) * (1.0 - w[..., None]), sky)
    return out + np.array([0.004, 0.003, 0.010], np.float32)

def main():
    rows = []
    step = 64
    for j in range(0, H, step):
        rows.append(render_rows(j, min(j + step, H)))
        print(f"row {j}/{H}", end="\r", flush=True)
    img = np.concatenate(rows, 0)
    img = 1.0 - np.exp(-img * 1.25)
    img = np.clip(img, 0, 1) ** (1.0 / 1.15)
    Image.fromarray((img * 255 + 0.5).astype(np.uint8), "RGB").save(OUT, optimize=True)
    print(f"\nwrote {OUT}  mean={img.mean():.4f}  max={img.max():.3f}")

if __name__ == "__main__":
    main()
