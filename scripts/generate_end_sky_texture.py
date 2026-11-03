#!/usr/bin/env python3
"""Generate end_sky.png: the 360 degree equirectangular sky of The End.

Output: assets/textures/environment/end_sky.png (2048x1024, RGB).

A black hole hangs just above the horizon of a bright, cloud-covered Earth seen from
above the clouds, under a star field. Everything is computed from 3D view directions,
so the left and right edges meet exactly and the poles are not stretched.

The black hole is ray traced (Schwarzschild null geodesics, offline, numpy): light is
bent around the shadow, the thin accretion disc appears above and below it, there is a
photon ring, the approaching side is brighter (Doppler beaming) and the stars behind it
are lensed. Layers: stars, faint galactic band and nebulae, atmosphere glow, cloud sea,
black hole. Deterministic (fixed seed). Requires numpy, scipy and Pillow (about 3 min)."""
import sys
import math
import numpy as np
from scipy.ndimage import gaussian_filter
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
                pd = np.linalg.norm(perp, axis=-1)                     # distance from the ray in cell units
                prof = np.exp(-0.5 * (ang / sg) ** 2) + 0.06 * np.exp(-(ang / (sg * 5.0))) * (1.0 - ss(0.35, 0.80, pd))   # core + faint halo (kept inside the cell)
                cidx = np.searchsorted(cum, hash3(ix, iy, iz, salt + 5) * cum[-1])
                col = STAR_COLORS[np.clip(cidx, 0, 4)]
                w = (present & (along > 0)) * bright * prof
                acc += w[..., None] * col
    return acc

# ----------------------------------------------------------------------------- black hole
BH_AZ = -1.15            # azimuth of the black hole (radians, atan2(x, z) as in the shader)
BH_EL = 0.10             # elevation of its center above the horizon
BH_THETA = 0.80          # radius of the patch that is ray traced (radians)
BH_D = 14.0              # observer distance, in Schwarzschild radii (shadow radius ~ 2.6/D rad)
BH_INC = math.radians(6.5)   # observer height above the disc plane
RIN, ROUT = 3.0, 9.0     # accretion disc radii (ISCO .. outer edge)
SS = 2                   # supersampling of the ray traced patch

def bh_frame():
    c = np.array([math.sin(BH_AZ) * math.cos(BH_EL), math.sin(BH_EL), math.cos(BH_AZ) * math.cos(BH_EL)], np.float64)
    up = np.array([0.0, 1.0, 0.0])
    e1 = np.cross(up, c); e1 /= np.linalg.norm(e1)
    e2 = np.cross(c, e1)
    return c, e1, e2

def trace(x, y):
    """Trace rays for gnomonic image coordinates (x right, y up). Returns
    emission (N,3), background weight (N,), outgoing direction in the camera frame (N,3)."""
    n = x.shape[0]
    O = np.array([BH_D * math.cos(BH_INC), BH_D * math.sin(BH_INC), 0.0])
    f = -O / BH_D
    R = np.array([0.0, 0.0, 1.0])
    U = np.cross(f, R)
    v = f[None] + x[:, None] * R[None] + y[:, None] * U[None]
    v /= np.linalg.norm(v, axis=1, keepdims=True)
    pos = np.tile(O, (n, 1))
    h2 = np.sum(np.cross(pos, v) ** 2, axis=1)
    emit = np.zeros((n, 3)); trans = np.ones(n); out = v.copy(); alive = np.ones(n, bool)
    hot = np.array([0.80, 0.90, 1.00]); cool = np.array([0.98, 0.88, 0.95])
    idx = np.arange(n)
    for step in range(900):
        if idx.size == 0:
            break
        p = pos[idx]; vv = v[idx]; hh = h2[idx]
        r = np.linalg.norm(p, axis=1)
        dt = 0.045 * r
        a1 = -1.5 * (hh / r ** 5)[:, None] * p
        pn = p + vv * dt[:, None] + 0.5 * a1 * (dt ** 2)[:, None]
        rn = np.linalg.norm(pn, axis=1)
        a2 = -1.5 * (hh / rn ** 5)[:, None] * pn
        vn = vv + 0.5 * (a1 + a2) * dt[:, None]
        vn /= np.linalg.norm(vn, axis=1, keepdims=True)

        # disc crossing (plane y = 0)
        yo, yn = p[:, 1], pn[:, 1]
        cr = (yo * yn < 0)
        if cr.any():
            t = yo[cr] / (yo[cr] - yn[cr])
            hp = p[cr] + t[:, None] * (pn[cr] - p[cr])
            hv = vv[cr]
            rh = np.linalg.norm(hp, axis=1)
            ok = (rh > RIN) & (rh < ROUT)
            if ok.any():
                sel = np.nonzero(cr)[0][ok]
                hp = hp[ok]; hv = hv[ok]; rh = rh[ok]
                rho = np.maximum(np.hypot(hp[:, 0], hp[:, 2]), 1e-6)
                phi_hat = np.stack([-hp[:, 2] / rho, np.zeros_like(rho), hp[:, 0] / rho], 1)
                beta = np.sqrt(0.5 / rh)
                n_ph = -hv
                g = np.sqrt(1 - beta ** 2) / (1 - beta * np.sum(phi_hat * n_ph, 1))
                g *= np.sqrt(np.maximum(1 - 1 / rh, 0.05))
                phi = np.arctan2(hp[:, 2], hp[:, 0]) + 4.0 / np.sqrt(rh)
                nz = fbm(np.stack([rh * 4.5, np.cos(phi) * 1.7, np.sin(phi) * 1.7], 1).astype(np.float32), SEED + 900, 4).astype(np.float64)
                tex = 0.55 + 0.95 * ss(0.30, 0.75, nz)
                prof = (RIN / rh) ** 2.0 * ss(RIN, RIN * 1.12, rh) * (1 - ss(0.55 * ROUT, ROUT, rh))
                tt = ss(RIN, ROUT, rh)[:, None]
                col = hot[None] * (1 - tt) + cool[None] * tt
                E = col * (prof * tex * g ** 3 * 2.4)[:, None]
                gi = idx[sel]
                emit[gi] += trans[gi][:, None] * E
                trans[gi] *= 0.12          # the disc is nearly opaque
        pos[idx] = pn; v[idx] = vn
        cap = rn < 1.0
        esc = (rn > 40.0) & (np.sum(pn * vn, 1) > 0)
        done = cap | esc
        if done.any():
            di = idx[done]
            out[di] = vn[done]
            trans[di[cap[done]]] = 0.0
            idx = idx[~done]
    if idx.size:
        out[idx] = v[idx]
    # express the outgoing direction in the camera frame (f, R, U)
    comp = np.stack([out @ f, out @ R, out @ U], 1)
    return emit, trans, comp

def bilinear(img, u, v):
    """Wrap in u, clamp in v. img (H,W,C), u,v in 0..1."""
    h, w = img.shape[:2]
    x = u * w - 0.5; yy = np.clip(v * h - 0.5, 0, h - 1.0001)
    x0 = np.floor(x).astype(int); y0 = np.floor(yy).astype(int)
    fx = (x - x0)[..., None]; fy = (yy - y0)[..., None]
    x1 = (x0 + 1) % w; x0 = x0 % w; y1 = y0 + 1
    return img[y0, x0] * (1 - fx) * (1 - fy) + img[y0, x1] * fx * (1 - fy) + img[y1, x0] * (1 - fx) * fy + img[y1, x1] * fx * fy

def dir_to_uv(d):
    return np.arctan2(d[..., 0], d[..., 2]) / (2 * np.pi) + 0.5, np.arccos(np.clip(d[..., 1], -1, 1)) / np.pi

def composite_black_hole(base, cover):
    c, e1, e2 = bh_frame()
    # Patch bounds in equirect pixels.
    cu, cv = dir_to_uv(c)
    half_w = int(BH_THETA / (2 * np.pi) * W / max(math.cos(BH_EL), 0.2)) + 4
    half_h = int(BH_THETA / np.pi * H) + 4
    j_c = int(cv * H); i_c = int(cu * W)
    j0, j1 = max(j_c - half_h, 0), min(j_c + half_h, H)
    jj, ii = np.mgrid[j0 * SS:j1 * SS, (i_c - half_w) * SS:(i_c + half_w) * SS]
    u = ((ii + 0.5) / (W * SS)) % 1.0
    v = (jj + 0.5) / (H * SS)
    ang = (u - 0.5) * 2 * np.pi; th = v * np.pi
    d = np.stack([np.sin(ang) * np.sin(th), np.cos(th), np.cos(ang) * np.sin(th)], -1)
    dc = d @ c
    theta = np.arccos(np.clip(dc, -1, 1))
    inside = (dc > 0.2) & (theta < BH_THETA)
    gx = (d @ e1) / np.maximum(dc, 0.2); gy = (d @ e2) / np.maximum(dc, 0.2)
    sel = np.nonzero(inside.ravel())[0]
    print(f"tracing {sel.size} rays", flush=True)
    emit_all = np.zeros((sel.size, 3)); trans_all = np.zeros(sel.size); comp_all = np.zeros((sel.size, 3))
    chunk = 120000
    for k in range(0, sel.size, chunk):
        sl = slice(k, k + chunk)
        e_, t_, c_ = trace(gx.ravel()[sel][sl], gy.ravel()[sel][sl])
        emit_all[sl], trans_all[sl], comp_all[sl] = e_, t_, c_
        print(f"  {min(k + chunk, sel.size)}/{sel.size}", end="\r", flush=True)
    print()
    # Lensed background: look the sky up in the deflected direction, tapering to the
    # undeflected one toward the patch edge so there is no seam.
    d_out = comp_all[:, 0:1] * c[None] + comp_all[:, 1:2] * e1[None] + comp_all[:, 2:3] * e2[None]
    d_flat = d.reshape(-1, 3)[sel]
    th_flat = theta.ravel()[sel]
    wl = 1.0 - ss(0.40, BH_THETA, th_flat)
    d_use = d_flat * (1 - wl[:, None]) + d_out * wl[:, None]
    d_use /= np.linalg.norm(d_use, axis=1, keepdims=True)
    bu, bv = dir_to_uv(d_use)
    bg = bilinear(base, bu, bv)
    captured = trans_all <= 0.0
    # Pixels whose ray fell into the horizon are black; the lensed sky only counts when it escaped.
    color = emit_all + (trans_all * (1 - 0.0))[:, None] * bg
    color = np.where(captured[:, None], emit_all, color)
    edge = 1.0 - ss(0.55, BH_THETA, th_flat)          # emission fades out toward the patch edge
    emit_vis = emit_all * edge[:, None]
    soft = 1.0 - ss(0.62, BH_THETA, th_flat)          # the whole patch fades into the base sky
    color = np.where(captured[:, None], emit_vis, emit_vis + trans_all[:, None] * bg)

    patch = np.zeros((jj.shape[0] * jj.shape[1], 3)); emit_img = np.zeros_like(patch); mask = np.zeros(jj.shape[0] * jj.shape[1])
    patch[sel] = color; emit_img[sel] = emit_vis; mask[sel] = soft
    shape = jj.shape
    def down(a):
        a = a.reshape(shape + (-1,)) if a.ndim == 2 else a.reshape(shape)
        h, w = shape
        return a.reshape(h // SS, SS, w // SS, SS, *a.shape[2:]).mean(axis=(1, 3))
    patch = down(patch); emit_img = down(emit_img); mask = down(mask)
    # Soft bloom of the bright disc and ring.
    bloom = 0.55 * gaussian_filter(emit_img, (3.0, 3.0, 0)) + 0.40 * gaussian_filter(emit_img, (9.0, 9.0, 0)) \
        + 0.35 * gaussian_filter(emit_img, (28.0, 28.0, 0))
    cols = (np.arange((i_c - half_w), (i_c + half_w))) % W
    rows = np.arange(j0, j1)
    cov = cover[np.ix_(rows, cols)][..., None]
    vis = 1.0 - cov                                   # clouds in front hide the black hole
    cur = base[np.ix_(rows, cols)]
    m = mask[..., None]
    new = cur * (1 - m) + (cur * (1 - vis) + patch * vis) * m
    new = new + bloom * np.array([0.90, 0.97, 1.12]) * (1.0 - 0.55 * cov)
    out = base.copy()
    out[np.ix_(rows, cols)] = new
    return out

# ----------------------------------------------------------------------------- sky and clouds
def render_rows(j0, j1):
    jj, ii = np.mgrid[j0:j1, 0:W]
    u = (ii + 0.5) / W
    v = (jj + 0.5) / H
    ang = (u - 0.5) * 2.0 * np.pi
    th = v * np.pi
    d = np.stack([np.sin(ang) * np.sin(th), np.cos(th), np.cos(ang) * np.sin(th)], -1).astype(np.float32)
    y = d[..., 1]

    # --- sky above: navy gradient with an Earth atmosphere glow at the horizon
    ay = np.abs(y)
    base = np.array([0.008, 0.012, 0.040], np.float32) + np.exp(-ay / 0.20)[..., None] * np.array([0.030, 0.060, 0.150], np.float32)

    # --- faint galactic band and nebulae
    n = np.array([0.30, 0.86, 0.41], np.float32); n /= np.linalg.norm(n)
    lat = np.sum(d * n, -1)
    warp = 0.20 * (fbm(d * 2.2, SEED + 10, 3) - 0.5)
    band = np.exp(-((lat + warp) / 0.17) ** 2)
    mist = fbm(d * 3.0 + 4.0, SEED + 20, 5)
    dust = ss(0.52, 0.72, fbm(d * 5.5 + 9.0, SEED + 30, 5)) * np.exp(-((lat + warp * 0.5) / 0.12) ** 2)
    glow = band * (0.45 + 0.9 * ss(0.30, 0.80, mist)) * (1.0 - 0.8 * dust)
    band_rgb = glow[..., None] * np.array([0.42, 0.40, 0.66], np.float32) * 0.13
    n1 = ss(0.55, 0.82, fbm(d * 1.5 + 21.0, SEED + 40, 5)) ** 1.6
    n2 = ss(0.58, 0.84, fbm(d * 1.9 + 53.0, SEED + 50, 5)) ** 1.6
    neb = n1[..., None] * np.array([0.28, 0.12, 0.42], np.float32) * 0.24 \
        + n2[..., None] * np.array([0.06, 0.22, 0.32], np.float32) * 0.22

    # --- stars
    boost = 1.0 + 1.0 * band
    st = star_layer(d, 70.0, 0.012, 0.0020, 0.12, 0.55, 3.0, SEED + 100, boost)
    st += star_layer(d, 38.0, 0.007, 0.0020, 0.28, 0.95, 4.0, SEED + 200, boost)
    st += star_layer(d, 16.0, 0.004, 0.0024, 0.55, 1.70, 5.0, SEED + 300)

    # --- thin bright atmosphere line at the horizon, brighter on the black hole side
    ch = np.array([math.sin(BH_AZ), math.cos(BH_AZ)], np.float32)
    hh = np.stack([d[..., 0], d[..., 2]], -1)
    hh = hh / np.maximum(np.linalg.norm(hh, axis=-1, keepdims=True), 1e-4)
    near = np.clip(np.sum(hh * ch, -1), 0, 1)
    limb = (np.exp(-ay / 0.014) * (0.7 + 1.1 * near ** 2))[..., None] * np.array([0.09, 0.20, 0.50], np.float32) \
        + (np.exp(-ay / 0.06) * (0.5 + 1.0 * near ** 2))[..., None] * np.array([0.04, 0.09, 0.24], np.float32)

    # --- Earth's cloud layer below the horizon: bright cumulus, blue shadows, deep blue gaps
    below = y < 0.0
    ys = np.minimum(y, -0.035)
    t = 1.0 / (-ys)
    P = np.stack([d[..., 0] * t, d[..., 2] * t, np.zeros_like(t)], -1) * 1.4
    q = fbm(P * 0.5 + 7.0, SEED + 800, 3)
    Pw = P + np.stack([q - 0.5, (q - 0.5) * 0.7, np.zeros_like(q)], -1) * 1.3
    fine = ss(-0.30, -0.10, y)       # fine detail fades toward the horizon (it would only alias there)
    def cloud_height(Pq):
        return 0.50 * fbm(Pq, SEED + 810, 5) + 0.28 * fbm(Pq * 3.3 + 11.0, SEED + 815, 4) \
            + fine * (0.14 * fbm(Pq * 9.0 + 5.0, SEED + 818, 3) + 0.08 * fbm(Pq * 24.0 + 2.0, SEED + 819, 2)) + (1.0 - fine) * 0.11
    cn = cloud_height(Pw)
    Ldir = ch * 0.07
    cl_n = cloud_height(Pw + np.array([Ldir[0], Ldir[1], 0.0], np.float32))
    dens = ss(0.40, 0.54, cn)
    puff = fbm(Pw * 3.2 + 3.0, SEED + 820, 3)
    dens = np.clip(dens * (0.75 + 0.55 * puff), 0.0, 1.0)
    slope = np.clip(0.52 + (cn - cl_n) * 12.0, 0.0, 1.0)
    lit = np.clip(0.30 + 0.70 * slope, 0, 1) * (0.55 + 0.45 * dens)
    bright = 0.62 + 0.55 * near ** 2
    sea = np.array([0.060, 0.095, 0.205], np.float32)            # deep blue gaps
    c_shadow = np.array([0.30, 0.38, 0.56], np.float32)          # blue-grey cloud shadows
    c_lit = np.array([0.98, 0.95, 0.88], np.float32)             # sunlit cloud tops, warm white
    cloud = c_shadow + (c_lit - c_shadow) * (lit * bright)[..., None]
    cloud = sea + (cloud - sea) * dens[..., None]
    w = (1.0 - ss(-0.22, -0.015, y)) * below
    cover = w * dens

    sky = base + band_rgb + neb + st * 1.0 + limb
    haze = base + limb
    below_col = haze * (1.0 - w[..., None]) + cloud * w[..., None] + st * (1.0 - cover)[..., None] * 0.0
    below_col = below_col + st * (1.0 - cover)[..., None] * (1.0 - w)[..., None] * 0.0
    stars_below = st * (1.0 - cover)[..., None]
    below_col = haze * (1.0 - w[..., None]) + cloud * w[..., None] + stars_below + band_rgb * (1.0 - cover)[..., None] + neb * (1.0 - cover)[..., None]
    out = np.where(below[..., None], below_col, sky)
    return out + np.array([0.004, 0.004, 0.012], np.float32), cover.astype(np.float32)

def main():
    rows, covers = [], []
    step = 64
    for j in range(0, H, step):
        r, c = render_rows(j, min(j + step, H))
        rows.append(r); covers.append(c)
        print(f"row {j}/{H}", end="\r", flush=True)
    base = np.concatenate(rows, 0).astype(np.float64)
    cover = np.concatenate(covers, 0).astype(np.float64)
    print()
    img = composite_black_hole(base, cover)
    img = 1.0 - np.exp(-img * 1.25)
    img = np.clip(img, 0, 1) ** (1.0 / 1.15)
    Image.fromarray((img * 255 + 0.5).astype(np.uint8), "RGB").save(OUT, optimize=True)
    print(f"wrote {OUT}  mean={img.mean():.4f}  max={img.max():.3f}")

if __name__ == "__main__":
    main()
