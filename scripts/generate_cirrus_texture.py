#!/usr/bin/env python3
"""Generates assets/textures/environment/CirrusTexture.png for the Bedralis cirrus clouds.

The texture is 256x256, RGBA, and tiles seamlessly (every field is built in the
Fourier domain, so it is periodic by construction). Every channel is rank-transformed
to a uniform 0..1 distribution, so a shader threshold maps directly to a coverage fraction.
It is bound in the Sky material as the buffer "noisetexture" (sampler s_noisetexture).

  R : strands. Long, thin, gently curved filaments (the cirrus fibers)
  G : veil mass. Long soft bands that run along the strands, with some isotropic variation
  B : fine detail strands at another angle (sampled at a higher frequency)
  A : 255

The output is deterministic (fixed seed): rerunning gives the same pixels.

Run:  python3 scripts/generate_cirrus_texture.py [output.png]
Needs numpy and Pillow.
"""
import sys
import numpy as np
from PIL import Image

N = 256
rng = np.random.default_rng(20261102)


def spectrum_noise(sigma_u, sigma_v, angle_deg, low_cut=1.5):
    """Zero-mean periodic noise, stretched along the direction angle_deg.

    sigma_u : spectrum width along the stripe direction (small = long streaks)
    sigma_v : spectrum width across the stripe direction (large = thin streaks)
    """
    kx = np.fft.fftfreq(N) * N
    ky = np.fft.fftfreq(N) * N
    kx, ky = np.meshgrid(kx, ky)
    a = np.radians(angle_deg)
    u = kx * np.cos(a) + ky * np.sin(a)
    v = -kx * np.sin(a) + ky * np.cos(a)
    amp = np.exp(-(u**2 / (2 * sigma_u**2) + v**2 / (2 * sigma_v**2)))
    amp *= 1.0 - np.exp(-(kx**2 + ky**2) / (2 * low_cut**2))   # remove the mean and the lowest swell
    white = rng.standard_normal((N, N))
    field = np.real(np.fft.ifft2(np.fft.fft2(white) * amp))
    return (field - field.mean()) / field.std()


def thin_lines(field, width):
    """Bright thin lines where the field crosses zero (ridged look)."""
    return np.exp(-(field / width) ** 2)


def rank_uniform(a):
    """Rank transform to a uniform 0..1 distribution."""
    order = a.ravel().argsort().argsort()
    return (order / (order.size - 1)).reshape(a.shape)


def warp(a, amount, seed_k=(1, 2)):
    """Periodic domain warp (integer cycles keep it tileable), bilinear with wrap."""
    y, x = np.mgrid[0:N, 0:N].astype(float)
    wx = amount * np.sin(2 * np.pi * (seed_k[0] * x + seed_k[1] * y) / N + 0.7)
    wy = amount * np.cos(2 * np.pi * (seed_k[1] * x - seed_k[0] * y) / N + 1.9)
    xs, ys = (x + wx) % N, (y + wy) % N
    x0, y0 = np.floor(xs).astype(int), np.floor(ys).astype(int)
    fx, fy = xs - x0, ys - y0
    x1, y1 = (x0 + 1) % N, (y0 + 1) % N
    return (a[y0, x0] * (1 - fx) * (1 - fy) + a[y0, x1] * fx * (1 - fy)
            + a[y1, x0] * (1 - fx) * fy + a[y1, x1] * fx * fy)


def main(out):
    # R: strands. Long, thin and fairly sparse; two fields at slightly different
    # angles combined as thin lines, then warped so the filaments curve.
    f1 = thin_lines(spectrum_noise(1.7, 16.0, -22.0), 0.22)
    f2 = thin_lines(spectrum_noise(2.4, 22.0, -8.0), 0.22)
    fibers = warp(np.maximum(f1, 0.8 * f2), 3.5, (1, 2))
    fibers = rank_uniform(fibers)

    # G: veil mass. Long soft bands that run along the strands, with a little
    # isotropic variation so the veils do not look combed.
    mass = spectrum_noise(2.0, 6.5, -20.0, low_cut=0.7)
    mass += 0.40 * spectrum_noise(5.0, 5.0, 0.0, low_cut=0.7)
    mass = rank_uniform(warp(mass, 5.0, (1, 1)))

    # B: fine detail strands at another angle.
    detail = thin_lines(spectrum_noise(3.5, 40.0, 14.0), 0.40)
    detail = rank_uniform(warp(detail, 2.0, (2, 1)))

    rgba = np.stack([fibers, mass, detail, np.ones((N, N))], axis=-1)
    Image.fromarray((np.clip(rgba, 0, 1) * 255 + 0.5).astype(np.uint8), "RGBA").save(out, optimize=True)
    print("wrote", out)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "assets/textures/environment/CirrusTexture.png")
