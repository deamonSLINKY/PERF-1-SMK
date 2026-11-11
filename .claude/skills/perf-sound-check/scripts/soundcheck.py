#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""PERF sound check: measure a rendered WAV (and its MIDI) and judge it against genre targets.

Needs only Python 3.9+ and numpy (no scipy, librosa or ffmpeg), so it runs wherever PERF's tests run.

    python soundcheck.py analyze render.wav [--midi render.mid] [--genre techno] [--bpm 128] [--block 256] [--json out.json]
    python soundcheck.py midi render.mid [--bpm 128] [--json out.json]
    python soundcheck.py compare old.json new.json            # or two WAVs: adds a null test
    python soundcheck.py report results/ --out report.md      # every *.json in a folder, one Markdown report
    python soundcheck.py selftest                             # synthetic signals with known answers

Measurements follow ITU-R BS.1770-4 / EBU R128 for loudness (K-weighting, 400 ms momentary, 3 s short-term, gated
integrated, Tech 3342 LRA) and BS.1770 Annex 2 style 4x oversampling for true peak. Thresholds live in
targets.json next to the skill's references; each carries a source tag ("sourced" or "heuristic").
"""
import argparse, json, math, os, struct, sys
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent
TARGETS = HERE.parent / "references" / "targets.json"
EPS = 1e-12


# ----------------------------------------------------------------------------------------------------------------
# WAV reading (PCM 8/16/24/32 and IEEE float 32/64), no dependencies
# ----------------------------------------------------------------------------------------------------------------
def read_wav(path):
    data = Path(path).read_bytes()
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError(f"{path}: not a RIFF/WAVE file")
    pos, fmt, raw = 12, None, None
    while pos + 8 <= len(data):
        cid, size = data[pos:pos + 4], struct.unpack("<I", data[pos + 4:pos + 8])[0]
        body = data[pos + 8:pos + 8 + size]
        if cid == b"fmt ":
            tag, ch, sr, _, _, bits = struct.unpack("<HHIIHH", body[:16])
            if tag == 0xFFFE and len(body) >= 26:  # WAVE_FORMAT_EXTENSIBLE: the subformat's first two bytes
                tag = struct.unpack("<H", body[24:26])[0]
            fmt = (tag, ch, sr, bits)
        elif cid == b"data":
            raw = body
        pos += 8 + size + (size & 1)
    if fmt is None or raw is None:
        raise ValueError(f"{path}: missing fmt or data chunk")
    tag, ch, sr, bits = fmt
    if tag == 3:
        x = np.frombuffer(raw[: len(raw) // (bits // 8) * (bits // 8)], dtype="<f4" if bits == 32 else "<f8").astype(np.float64)
    elif tag == 1:
        if bits == 8:
            x = (np.frombuffer(raw, dtype=np.uint8).astype(np.float64) - 128.0) / 128.0
        elif bits == 16:
            x = np.frombuffer(raw[: len(raw) // 2 * 2], dtype="<i2").astype(np.float64) / 32768.0
        elif bits == 24:
            b = np.frombuffer(raw[: len(raw) // 3 * 3], dtype=np.uint8).reshape(-1, 3).astype(np.int32)
            v = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
            v = np.where(v & 0x800000, v - 0x1000000, v)
            x = v.astype(np.float64) / 8388608.0
        elif bits == 32:
            x = np.frombuffer(raw[: len(raw) // 4 * 4], dtype="<i4").astype(np.float64) / 2147483648.0
        else:
            raise ValueError(f"{path}: {bits}-bit PCM not supported")
    else:
        raise ValueError(f"{path}: WAV format tag {tag} not supported (PCM or float only)")
    x = x[: len(x) // ch * ch].reshape(-1, ch)
    return x, sr


def write_wav(path, x, sr):
    """16-bit PCM writer, used by selftest."""
    x = np.atleast_2d(np.asarray(x, dtype=np.float64))
    if x.shape[0] < x.shape[1]:
        x = x.T
    pcm = (np.clip(x, -1, 1 - 1 / 32768) * 32768).astype("<i2").tobytes()
    ch = x.shape[1]
    hdr = b"RIFF" + struct.pack("<I", 36 + len(pcm)) + b"WAVEfmt " + struct.pack("<IHHIIHH", 16, 1, ch, sr, sr * ch * 2, ch * 2, 16)
    Path(path).write_bytes(hdr + b"data" + struct.pack("<I", len(pcm)) + pcm)


# ----------------------------------------------------------------------------------------------------------------
# DSP helpers (numpy only)
# ----------------------------------------------------------------------------------------------------------------
def fftconv(x, h):
    """Linear convolution along axis 0, same length as x."""
    n = len(x) + len(h) - 1
    nfft = 1 << (n - 1).bit_length()
    H = np.fft.rfft(h, nfft)
    if x.ndim == 1:
        return np.fft.irfft(np.fft.rfft(x, nfft) * H, nfft)[: len(x)]
    return np.fft.irfft(np.fft.rfft(x, nfft, axis=0) * H[:, None], nfft, axis=0)[: len(x)]


def iir_impulse(b, a, n=8192):
    y, x1, x2, y1, y2 = np.zeros(n), 0.0, 0.0, 0.0, 0.0
    for i in range(n):
        xi = 1.0 if i == 0 else 0.0
        yi = b[0] * xi + b[1] * x1 + b[2] * x2 - a[1] * y1 - a[2] * y2
        x2, x1, y2, y1 = x1, xi, y1, yi
        y[i] = yi
    return y


def kweight_ir(sr):
    """BS.1770 K-weighting (shelf + RLB high-pass) for any rate, coefficients derived as libebur128 does."""
    f0, G, Q = 1681.974450955533, 3.999843853973347, 0.7071752369554196
    K = math.tan(math.pi * f0 / sr)
    Vh, Vb = 10 ** (G / 20), (10 ** (G / 20)) ** 0.4996667741545416
    a0 = 1 + K / Q + K * K
    b1 = [(Vh + Vb * K / Q + K * K) / a0, 2 * (K * K - Vh) / a0, (Vh - Vb * K / Q + K * K) / a0]
    a1 = [1, 2 * (K * K - 1) / a0, (1 - K / Q + K * K) / a0]
    f0, Q = 38.13547087602444, 0.5003270373238773
    K = math.tan(math.pi * f0 / sr)
    a0 = 1 + K / Q + K * K
    b2, a2 = [1, -2, 1], [1, 2 * (K * K - 1) / a0, (1 - K / Q + K * K) / a0]
    return fftconv(iir_impulse(b1, a1), iir_impulse(b2, a2))


def sinc_lowpass(cut_hz, sr, taps=4095):
    n = np.arange(taps) - (taps - 1) / 2
    h = 2 * cut_hz / sr * np.sinc(2 * cut_hz / sr * n) * np.blackman(taps)
    return h / h.sum()


def true_peak(x, sr):
    """4x (2x at >= 96 kHz) oversampled absolute peak, windowed-sinc interpolation."""
    L = 4 if sr < 96000 else 2
    taps = 12 * L * 4 + 1
    n = np.arange(taps) - (taps - 1) / 2
    h = np.sinc(n / L) * np.kaiser(taps, 8.0)
    up = np.zeros((len(x) * L, x.shape[1]))
    up[::L] = x
    y = fftconv(up, h)
    return float(np.max(np.abs(y))) if len(y) else 0.0


def db(v):
    return 20 * math.log10(max(float(v), EPS))


def frames(x, size, hop):
    if len(x) < size:
        return np.zeros((0, size) + x.shape[1:])
    idx = np.arange(0, len(x) - size + 1, hop)
    return np.stack([x[i:i + size] for i in idx])


# ----------------------------------------------------------------------------------------------------------------
# Loudness (BS.1770-4 / EBU R128)
# ----------------------------------------------------------------------------------------------------------------
def channel_weights(ch):
    return np.array([1.0, 1.0, 1.0, 0.0, 1.41, 1.41][:ch]) if ch > 2 else np.ones(ch)


def loudness(x, sr):
    y = fftconv(x, kweight_ir(sr))
    w = channel_weights(x.shape[1])
    hop = int(round(0.1 * sr))
    if len(y) < 4 * hop:
        return {"integrated": None, "momentary_max": None, "short_term_max": None, "lra": None, "short_term": []}
    seg = np.array([np.mean(y[i:i + hop] ** 2, axis=0) for i in range(0, len(y) - hop + 1, hop)])  # 100 ms mean squares

    def block(nseg):
        if len(seg) < nseg:
            return np.zeros(0)
        z = np.array([seg[i:i + nseg].mean(axis=0) for i in range(len(seg) - nseg + 1)])
        return -0.691 + 10 * np.log10(np.maximum((z * w).sum(axis=1), EPS))

    m = block(4)      # 400 ms, 75 % overlap
    s = block(30)     # 3 s, 10 Hz update
    # integrated: absolute gate -70, relative gate -10 LU
    zb = np.array([seg[i:i + 4].mean(axis=0) for i in range(len(seg) - 3)])
    pw = (zb * w).sum(axis=1)
    lk = -0.691 + 10 * np.log10(np.maximum(pw, EPS))
    g = lk > -70
    I = None
    if g.any():
        rel = -0.691 + 10 * math.log10(max(pw[g].mean(), EPS)) - 10
        g2 = g & (lk > rel)
        if g2.any():
            I = -0.691 + 10 * math.log10(max(pw[g2].mean(), EPS))
    lra = None
    if len(s):
        sg = s[s > -70]
        if len(sg):
            rel = 10 * math.log10(max(np.mean(10 ** (sg / 10)), EPS)) - 20
            sg = sg[sg > rel]
            if len(sg) > 1:
                lra = float(np.percentile(sg, 95) - np.percentile(sg, 10))
    # PSR the way Ian Shepherd reads it: (peak in the 3 s window - its short-term loudness), P10 over the loud
    # windows (within 3 LU of the loudest). Window peaks are sample peaks; the caller adds the true-peak overshoot.
    psr = None
    if len(s):
        pk = np.array([np.max(np.abs(x[i * hop:(i + 30) * hop])) for i in range(len(s))])
        loud = s >= s.max() - 3
        vals = 20 * np.log10(np.maximum(pk[loud], EPS)) - s[loud]
        psr = float(np.percentile(vals, 10)) if len(vals) else None
    jumps = 0
    if len(s) > 20:
        sec = s[::10]
        jumps = int((np.abs(np.diff(sec)) > 3).sum())
    return {"integrated": I, "momentary_max": float(m.max()) if len(m) else None,
            "short_term_max": float(s.max()) if len(s) else None, "lra": lra, "psr_sample_db": psr,
            "short_term_jumps_3lu": jumps,
            "short_term": [round(float(v), 2) for v in s[::10]]}  # one value a second


# ----------------------------------------------------------------------------------------------------------------
# Spectrum, stereo, artefacts
# ----------------------------------------------------------------------------------------------------------------
BANDS = {"sub_20_60": (20, 60), "low_60_250": (60, 250), "lowmid_250_2k": (250, 2000),
         "presence_2k_8k": (2000, 8000), "air_8k_20k": (8000, 20000)}
BANDS_TBC = {"tbc_low_20_250": (20, 250), "tbc_lowmid_250_2k": (250, 2000),
             "tbc_highmid_2k_8k": (2000, 8000), "tbc_high_8k_20k": (8000, 20000)}


def psd(mono, sr, n=8192):
    if len(mono) < n:
        n = 1 << max(8, (len(mono) - 1).bit_length() - 1)
    f = frames(mono, n, n // 2)
    if not len(f):
        return np.zeros(1), np.zeros(1)
    win = np.hanning(n)
    P = (np.abs(np.fft.rfft(f * win, axis=1)) ** 2).mean(axis=0) / (win ** 2).sum()
    return np.fft.rfftfreq(n, 1 / sr), P


def spectrum(x, sr):
    mono = x.mean(axis=1)
    fr, P = psd(mono, sr)
    tot = P[(fr >= 20) & (fr <= 20000)].sum() + EPS
    out = {k: float(P[(fr >= lo) & (fr < hi)].sum() / tot) for k, (lo, hi) in {**BANDS, **BANDS_TBC}.items()}
    out["infra_below_20"] = float(P[(fr > 0) & (fr < 20)].sum() / tot)
    out["below_250"] = out["sub_20_60"] + out["low_60_250"]
    out["sub_to_low_ratio"] = out["sub_20_60"] / max(out["low_60_250"], EPS)
    centres = 1000 * 10 ** (np.arange(-17, 14) / 10)
    third = []
    for c in centres:
        sel = (fr >= c * 10 ** (-1 / 20)) & (fr < c * 10 ** (1 / 20))
        third.append(round(10 * math.log10(P[sel].sum() + EPS), 2) if sel.any() else None)
    ok = [(c, v) for c, v in zip(centres, third) if v is not None and 100 <= c <= 10000]
    slope = None
    if len(ok) > 4:
        lc, lv = np.log2([c for c, _ in ok]), np.array([v for _, v in ok])
        slope = float(np.polyfit(lc, lv, 1)[0])  # dB per octave of 1/3-octave band power
    sel = (fr > 20) & (fr < 20000)
    centroid = float((fr[sel] * P[sel]).sum() / (P[sel].sum() + EPS))
    lowsel = (fr >= 30) & (fr <= 120)
    kick_f0 = float(fr[lowsel][np.argmax(P[lowsel])]) if lowsel.any() else None
    return {"bands": out, "third_octave_db": dict(zip([round(float(c), 1) for c in centres], third)),
            "tilt_db_per_oct": slope, "centroid_hz": centroid, "low_peak_hz": kick_f0}


def stereo(x, sr):
    if x.shape[1] < 2:
        return {"channels": 1}
    L, R = x[:, 0], x[:, 1]

    def corr(a, b):
        sa, sb = a.std(), b.std()
        return float(np.mean((a - a.mean()) * (b - b.mean())) / (sa * sb)) if sa > EPS and sb > EPS else None

    w = int(0.4 * sr)
    wins = [corr(L[i:i + w], R[i:i + w]) for i in range(0, len(L) - w + 1, w)]
    wins = [v for v in wins if v is not None]
    lp = sinc_lowpass(120, sr)
    Ll, Rl = fftconv(L, lp), fftconv(R, lp)
    M, S = (L + R) / 2, (L - R) / 2
    return {"channels": 2, "correlation": corr(L, R), "correlation_min_400ms": min(wins) if wins else None,
            "correlation_below_120hz": corr(Ll, Rl),
            "side_to_mid_db": db(np.sqrt(np.mean(S ** 2))) - db(np.sqrt(np.mean(M ** 2))),
            "identical_channels": bool(np.allclose(L, R))}


def artefacts(x, sr, block):
    out = {"nan_or_inf": int((~np.isfinite(x)).sum())}
    x = np.nan_to_num(x)
    out["dc_dbfs"] = [db(abs(x[:, c].mean())) for c in range(x.shape[1])]
    runs, longest, where = 0, 0, []
    for c in range(x.shape[1]):
        hot = np.abs(x[:, c]) >= 0.999
        if hot.any():
            d = np.diff(np.concatenate([[0], hot.astype(np.int8), [0]]))
            st, en = np.where(d == 1)[0], np.where(d == -1)[0]
            ln = en - st
            big = ln >= 3
            runs += int(big.sum())
            if big.any():
                longest = max(longest, int(ln.max()))
                where += [round(float(t) / sr, 3) for t in st[big][:10]]
    out.update(clip_runs=runs, clip_longest=longest, clip_times_s=where[:10])
    mono = x.mean(axis=1)
    d2 = np.abs(np.diff(mono, 2))
    if len(d2) > sr // 10:
        # robust local level: the median of |second difference| in 10 ms blocks (a click barely moves a median),
        # interpolated back to every sample; a click is a sample far above it (heuristic threshold: calibrate)
        k = max(16, int(0.01 * sr))
        nb = len(d2) // k
        meds = np.median(d2[: nb * k].reshape(nb, k), axis=1)
        local = np.interp(np.arange(len(d2)), np.arange(nb) * k + k / 2, meds)
        z = d2 / (local * 1.4826 + 1e-7)
        hits = np.where(z > 60)[0]
        # a click is a jump in the *value*: an isolated spike in the first difference, with smooth samples around it.
        # A drum's hard attack changes the slope and stays steep (not a spike); a noise burst is steep everywhere.
        d1 = np.abs(np.diff(mono))
        keep, last = [], -sr
        for h in hits:
            if h - last > sr // 50:
                spike = max(d1[h], d1[h + 1]) if h + 1 < len(d1) else d1[h]
                nb = np.concatenate([d1[max(0, h - 40):max(0, h - 2)], d1[h + 4:h + 42]])
                if len(nb) and spike > 0.005 and spike > 8 * np.percentile(nb, 90):
                    keep.append(h)
            last = h
        out["clicks"] = len(keep)
        out["click_times_s"] = [round(float(h) / sr, 3) for h in keep[:10]]
        if block and keep:
            # by chance 5 of every `block` positions are within 2 samples of an edge; only a clear excess means a
            # buffer bug (clicks locked to the beat drift through every block phase and land on edges by chance)
            mods = np.array(keep) % block
            edge = int(((mods <= 2) | (mods >= block - 2)).sum())
            share = edge / len(keep)
            out["clicks_edge_share"] = round(share, 3)
            out["clicks_at_block_edge"] = edge if (edge >= 3 and share >= 0.5) else 0
        if len(keep) >= 4:
            iv = np.diff(np.array(keep)) / sr
            med = float(np.median(iv))
            if med > 0 and float(np.std(iv)) / med < 0.05:
                out["clicks_period_s"] = round(med, 4)   # clicks at a steady period: a voice event, not random
    w = int(0.05 * sr)
    rms = np.sqrt(np.mean(frames(mono, w, w) ** 2, axis=1)) if len(mono) >= w else np.zeros(1)
    out["noise_floor_dbfs"] = db(np.percentile(rms, 5)) if len(rms) else None
    out["tail_dbfs"] = db(rms[-10:].max()) if len(rms) >= 10 else None
    out["digital_silence_share"] = float((rms < 1e-6).mean()) if len(rms) else None
    return out


def onset_tempo(x, sr):
    """Rough tempo from the spectral-flux onset envelope (60-200 BPM). The MIDI tempo is better when you have it."""
    mono = x.mean(axis=1)
    n, hop = 1024, 256
    f = frames(mono, n, hop)
    if len(f) < 64:
        return None
    S = np.abs(np.fft.rfft(f * np.hanning(n), axis=1))
    flux = np.maximum(np.diff(np.log1p(S * 100), axis=0), 0).sum(axis=1)
    flux -= flux.mean()
    ac = np.correlate(flux, flux, mode="full")[len(flux) - 1:]
    fps = sr / hop
    lags = np.arange(len(ac))
    bpm = 60 * fps / np.maximum(lags, 1)
    sel = (bpm >= 60) & (bpm <= 200)
    if not sel.any():
        return None
    return float(bpm[sel][np.argmax(ac[sel])])


def bar_energy(x, sr, bpm, beats_per_bar=4):
    """Low-band and full-band level per bar, and drop candidates (a bar whose low band jumps >= 6 dB)."""
    if not bpm:
        return None
    bar = int(round(sr * 60 / bpm * beats_per_bar))
    if len(x) < 2 * bar:
        return None
    mono = x.mean(axis=1)
    low = fftconv(mono, sinc_lowpass(150, sr))
    full_db = [db(np.sqrt(np.mean(mono[i:i + bar] ** 2))) for i in range(0, len(mono) - bar + 1, bar)]
    low_db = [db(np.sqrt(np.mean(low[i:i + bar] ** 2))) for i in range(0, len(low) - bar + 1, bar)]
    drops = [i for i in range(1, len(low_db)) if low_db[i] - low_db[i - 1] >= 6]
    jumps = [i for i in range(1, len(full_db)) if abs(full_db[i] - full_db[i - 1]) >= 6]
    return {"bars": len(full_db), "full_db": [round(v, 1) for v in full_db], "low_db": [round(v, 1) for v in low_db],
            "drop_bars": drops, "level_jump_bars": jumps}


def analyze_audio(path, bpm=None, block=256):
    x, sr = read_wav(path)
    pk = float(np.max(np.abs(np.nan_to_num(x)))) if len(x) else 0.0
    tp = true_peak(np.nan_to_num(x), sr)
    L = loudness(np.nan_to_num(x), sr)
    mono_l = loudness(np.nan_to_num(x).mean(axis=1, keepdims=True) * math.sqrt(2) if x.shape[1] == 2 else x, sr)["integrated"] if x.shape[1] == 2 else None
    res = {"file": str(path), "sr": sr, "channels": x.shape[1], "seconds": round(len(x) / sr, 3),
           "sample_peak_dbfs": db(pk), "true_peak_dbtp": db(tp), "loudness": L,
           "spectrum": spectrum(np.nan_to_num(x), sr), "stereo": stereo(np.nan_to_num(x), sr),
           "artefacts": artefacts(x, sr, block), "tempo_audio_bpm": onset_tempo(np.nan_to_num(x), sr)}
    I = L["integrated"]
    res["plr_db"] = res["true_peak_dbtp"] - I if I is not None else None
    # PSR: loud-window P10 of (window peak - short-term), plus the file's true-peak overshoot over its sample peak
    res["psr_db"] = (L["psr_sample_db"] + (res["true_peak_dbtp"] - res["sample_peak_dbfs"])) if L["psr_sample_db"] is not None else None
    res["mono_sum_drop_lu"] = (I - mono_l) if (I is not None and mono_l is not None) else None
    res["bars"] = bar_energy(np.nan_to_num(x), sr, bpm or res["tempo_audio_bpm"])
    a, tb = res["artefacts"], bpm or res["tempo_audio_bpm"]
    if a.get("clicks_period_s") and tb and a.get("click_times_s"):
        beat = 60 / tb
        a["clicks_per_beat"] = round(beat / a["clicks_period_s"], 3)  # 1.0: once a beat, 0.25: once a bar...
        a["clicks_phase_in_beat_s"] = round(a["click_times_s"][0] % beat, 4)  # how long after the beat: e.g. a voice's length
    return res


# ----------------------------------------------------------------------------------------------------------------
# MIDI (Standard MIDI File reader, no dependencies)
# ----------------------------------------------------------------------------------------------------------------
def read_midi(path):
    d = Path(path).read_bytes()
    if d[:4] != b"MThd":
        raise ValueError(f"{path}: not a MIDI file")
    fmt, ntrk, div = struct.unpack(">HHH", d[8:14])
    if div & 0x8000:
        raise ValueError(f"{path}: SMPTE time division not supported")
    pos, events = 8 + struct.unpack(">I", d[4:8])[0], []
    for _ in range(ntrk):
        if d[pos:pos + 4] != b"MTrk":
            break
        ln = struct.unpack(">I", d[pos + 4:pos + 8])[0]
        p, end, tick, run = pos + 8, pos + 8 + ln, 0, 0
        while p < end:
            v = 0
            while True:
                c = d[p]; p += 1
                v = (v << 7) | (c & 0x7F)
                if not c & 0x80:
                    break
            tick += v
            st = d[p]
            if st == 0xFF:
                typ, p = d[p + 1], p + 2
                v = 0
                while True:
                    c = d[p]; p += 1
                    v = (v << 7) | (c & 0x7F)
                    if not c & 0x80:
                        break
                body = d[p:p + v]; p += v
                if typ == 0x51 and len(body) == 3:
                    events.append((tick, "tempo", int.from_bytes(body, "big")))
                elif typ == 0x58 and len(body) >= 2:
                    events.append((tick, "timesig", body[0]))
                continue
            if st in (0xF0, 0xF7):
                p += 1
                v = 0
                while True:
                    c = d[p]; p += 1
                    v = (v << 7) | (c & 0x7F)
                    if not c & 0x80:
                        break
                p += v
                continue
            if st & 0x80:
                run, p = st, p + 1
            kind, ch = run & 0xF0, run & 0x0F
            nd = 1 if kind in (0xC0, 0xD0) else 2
            a = d[p:p + nd]; p += nd
            if kind == 0x90 and a[1] > 0:
                events.append((tick, "on", ch, a[0], a[1]))
            elif kind == 0x80 or (kind == 0x90 and a[1] == 0):
                events.append((tick, "off", ch, a[0]))
            elif kind == 0xB0 and a[0] in (120, 123):   # all notes (or sound) off: every note sounding on the channel ends
                events.append((tick, "alloff", ch))
        pos = end
    # by tick, the file's own order kept within a tick (a stable sort): a drum's note-on and its note-off on the same
    # tick pair up (sorting offs first left one note per drum open), and a re-struck note's off-then-on stays in order
    events.sort(key=lambda e: (e[0], 0 if e[1] in ("tempo", "timesig") else 1))
    # ticks to seconds through the tempo map
    tempo, last_tick, last_s, out, tempos = 500000, 0, 0.0, [], []
    for e in events:
        s = last_s + (e[0] - last_tick) * tempo / 1e6 / div
        last_tick, last_s = e[0], s
        if e[1] == "tempo":
            tempo = e[2]
            tempos.append(60e6 / tempo)
        else:
            out.append((s,) + e[1:])
    notes, open_ = [], {}
    for e in out:
        if e[1] == "on":
            open_.setdefault((e[2], e[3]), []).append((e[0], e[4]))
        elif e[1] == "off" and open_.get((e[2], e[3])):
            t0, vel = open_[(e[2], e[3])].pop(0)
            notes.append({"t": t0, "dur": e[0] - t0, "ch": e[2], "pitch": e[3], "vel": vel})
        elif e[1] == "alloff":
            for key in [k for k in open_ if k[0] == e[2]]:
                for t0, vel in open_.pop(key):
                    notes.append({"t": t0, "dur": e[0] - t0, "ch": e[2], "pitch": key[1], "vel": vel})
    stuck = sum(len(v) for v in open_.values())
    return notes, (tempos[0] if tempos else 120.0), stuck


KK_MAJ = [6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88]
KK_MIN = [6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17]
EDMA_MAJ = [1.00, .29, .50, .40, .60, .56, .32, .80, .32, .45, .42, .39]   # Faraldo 2017, EDM-specific profiles
EDMA_MIN = [1.00, .31, .44, .58, .33, .49, .29, .78, .43, .29, .53, .32]
NAMES = ["C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]
DRUM_LANES = {36: "kick", 35: "kick", 38: "snare", 40: "snare", 37: "snare", 39: "clap", 42: "hat_closed", 44: "hat_closed",
              22: "hat_closed", 46: "hat_open", 26: "hat_open", 49: "crash", 57: "crash", 51: "ride", 59: "ride"}


def key_estimate(pcw, maj, minr):
    v = np.asarray(pcw, dtype=float)
    if v.sum() <= 0:
        return None
    scores = []
    for t in range(12):
        for mode, prof in (("major", maj), ("minor", minr)):
            p = np.roll(prof, t)
            scores.append((float(np.corrcoef(v, p)[0, 1]), t, mode))
    scores.sort(reverse=True)
    best, second = scores[0], scores[1]
    return {"key": f"{NAMES[best[1]]} {best[2]}", "tonic": best[1], "mode": best[2], "r": round(best[0], 3),
            "margin": round(best[0] - second[0], 3), "ambiguous": best[0] - second[0] < 0.05}


CHORDS = {"": [0, 4, 7], "m": [0, 3, 7], "dim": [0, 3, 6], "sus4": [0, 5, 7], "sus2": [0, 2, 7],
          "7": [0, 4, 7, 10], "maj7": [0, 4, 7, 11], "m7": [0, 3, 7, 10], "m9": [0, 3, 7, 10, 2]}
ROMAN = ["I", "bII", "II", "bIII", "III", "IV", "#IV", "V", "bVI", "VI", "bVII", "VII"]


def chord_of(pcw):
    v = np.asarray(pcw, dtype=float)
    if v.sum() <= 0:
        return None
    best = None
    for root in range(12):
        for q, iv in CHORDS.items():
            mask = np.zeros(12); mask[[(root + i) % 12 for i in iv]] = 1
            score = (v * mask).sum() / v.sum() - 0.02 * len(iv) + 0.05 * (v[root] / v.max())
            if best is None or score > best[0]:
                best = (score, root, q)
    return best[1], best[2]


def analyze_midi(path, bpm=None, steps=16):
    notes, midi_bpm, stuck = read_midi(path)
    bpm = bpm or midi_bpm
    step_s = 60 / bpm / 4
    res = {"file": str(path), "bpm": round(bpm, 3), "notes": len(notes), "stuck_notes": stuck}
    drums = [n for n in notes if n["ch"] == 9]
    tonal = [n for n in notes if n["ch"] != 9]
    nbars = max(1, int(math.ceil(max([n["t"] for n in notes] or [0]) / (step_s * steps))))
    res["bars"] = nbars
    # drums: per lane per step probability, offset (ms and twelfths of a step), velocity, ghosts, swing
    lanes = {}
    for n in drums:
        lane = DRUM_LANES.get(n["pitch"], f"perc_{n['pitch']}")
        q = n["t"] / step_s
        st = int(round(q))
        off = (n["t"] - st * step_s)
        L = lanes.setdefault(lane, {"hits": np.zeros(steps), "off": [[] for _ in range(steps)], "vel": [[] for _ in range(steps)]})
        L["hits"][st % steps] += 1
        L["off"][st % steps].append(off)
        L["vel"][st % steps].append(n["vel"])
    dr = {}
    for lane, L in lanes.items():
        vels = [v for vs in L["vel"] for v in vs]
        dr[lane] = {"prob": [round(min(1.0, h / nbars), 2) for h in L["hits"]],
                    "push_ms": [round(float(np.mean(o)) * 1000, 1) if o else None for o in L["off"]],
                    "push_twelfths": [round(float(np.mean(o)) / step_s * 12, 2) if o else None for o in L["off"]],
                    "vel_mean": [round(float(np.mean(v)), 1) if v else None for v in L["vel"]],
                    "ghost_share": round(float(np.mean([v < 50 for v in vels])), 3) if vels else None,
                    "hits": int(L["hits"].sum())}
    res["drums"] = dr
    offbeat = [n["t"] / step_s for n in drums if int(round(n["t"] / step_s)) % 2 == 1]
    if offbeat:
        # position of the off-beat 16th inside its 8th, as a drum machine's swing %: 50 straight, 66.7 triplet
        pos = [((q % 2) / 2) for q in offbeat]
        res["swing_pct"] = round(float(np.median(pos)) * 100, 1)
    # timing jitter of grid-locked hits (should be ~0 for a sequencer unless a feel is on)
    offs = [(n["t"] / step_s - round(n["t"] / step_s)) * step_s * 1000 for n in drums]
    res["drum_offset_sd_ms"] = round(float(np.std(offs)), 2) if offs else None
    # groove consistency: 1 - mean Hamming distance between neighbouring bars' onset vectors (MusPy's definition)
    if drums and nbars > 1:
        bars = np.zeros((nbars, steps))
        for n in drums:
            st = int(round(n["t"] / step_s))
            if st // steps < nbars:
                bars[st // steps, st % steps] = 1
        hd = [np.mean(bars[i] != bars[i + 1]) for i in range(nbars - 1)]
        res["groove_consistency"] = round(1 - float(np.mean(hd)), 3)
    # harmony
    if tonal:
        pcw = np.zeros(12)
        for n in tonal:
            pcw[n["pitch"] % 12] += n["dur"]
        res["key_kk"] = key_estimate(pcw, KK_MAJ, KK_MIN)
        res["key_edma"] = key_estimate(pcw, EDMA_MAJ, EDMA_MIN)
        p = pcw / pcw.sum()
        res["pitch_class_entropy"] = round(float(-(p[p > 0] * np.log2(p[p > 0])).sum()), 3)
        k = res["key_edma"] or res["key_kk"]
        scale = [0, 2, 4, 5, 7, 9, 11] if k["mode"] == "major" else [0, 2, 3, 5, 7, 8, 10]
        ins = sum(n["dur"] for n in tonal if (n["pitch"] - k["tonic"]) % 12 in scale)
        res["in_scale_share"] = round(ins / max(sum(n["dur"] for n in tonal), EPS), 3)
        # chord per bar from the pad and bass channels' sounding notes (everything tonal)
        bar_s = step_s * steps
        prog = []
        for b in range(nbars):
            w = np.zeros(12)
            t0, t1 = b * bar_s, (b + 1) * bar_s
            for n in tonal:
                ov = min(t1, n["t"] + n["dur"]) - max(t0, n["t"])
                if ov > 0:
                    w[n["pitch"] % 12] += ov
            c = chord_of(w)
            if c:
                rel = (c[0] - k["tonic"]) % 12
                numeral = ROMAN[rel]
                if c[1] in ("m", "m7", "m9", "dim"):
                    numeral = numeral.lower()
                prog.append(numeral + {"": "", "m": "", "dim": "°", "sus4": "sus4", "sus2": "sus2", "7": "7", "maj7": "maj7", "m7": "7", "m9": "9"}[c[1]])
        res["progression"] = prog
        trans = {}
        for a, b in zip(prog, prog[1:]):
            if a != b:
                trans[f"{a}>{b}"] = trans.get(f"{a}>{b}", 0) + 1
        res["chord_moves"] = dict(sorted(trans.items(), key=lambda kv: -kv[1])[:12])
        # per channel register (helps spot masking between bass and other parts)
        reg = {}
        for n in tonal:
            reg.setdefault(n["ch"] + 1, []).append(n["pitch"])
        res["register_by_channel"] = {str(c): {"low": int(min(v)), "high": int(max(v)), "median": float(np.median(v))} for c, v in reg.items()}
    return res


# ----------------------------------------------------------------------------------------------------------------
# Judging against targets
# ----------------------------------------------------------------------------------------------------------------
def load_targets(path=None):
    p = Path(path) if path else TARGETS
    return json.loads(p.read_text(encoding="utf-8")) if p.exists() else {"default": {}, "genres": {}}


def get(d, dotted):
    for k in dotted.split("."):
        if isinstance(d, dict) and k in d:
            d = d[k]
        else:
            return None
    return d


def judge(res, genre, targets):
    rules = dict(targets.get("default", {}))
    g = targets.get("genres", {}).get((genre or "").lower().replace("-", "_").replace(" ", "_"), {})
    rules.update(targets.get("profiles", {}).get(g.get("profile", ""), {}))
    rules.update(g.get("rules", {}))
    findings = []
    for metric, r in rules.items():
        v = get(res, metric)
        if isinstance(v, list):
            v = max(v) if v else None
        if v is None:
            continue
        status = "pass"
        for level in ("fail", "warn"):
            lo, hi = r.get(level + "_below"), r.get(level + "_above")
            if (lo is not None and v < lo) or (hi is not None and v > hi):
                status = level
                break
        findings.append({"metric": metric, "value": round(float(v), 3), "status": status, "say": r.get("say", ""),
                         "source": r.get("source", "heuristic"), "rule": {k: r[k] for k in r if k.endswith(("_below", "_above"))}})
    bpm = res.get("bpm") or res.get("tempo_audio_bpm")
    if g.get("bpm") and bpm:
        lo, hi = g["bpm"]
        ok = lo <= bpm <= hi or lo <= bpm * 2 <= hi or lo <= bpm / 2 <= hi
        findings.append({"metric": "bpm", "value": round(float(bpm), 1), "status": "pass" if ok else "warn",
                         "say": f"genre tempo range {lo}-{hi} BPM", "source": g.get("bpm_source", "sourced"), "rule": {"range": [lo, hi]}})
    sw = res.get("swing_pct")
    if g.get("swing") and sw is not None:
        lo, hi = g["swing"]
        findings.append({"metric": "swing_pct", "value": sw, "status": "pass" if lo - 2 <= sw <= hi + 2 else "warn",
                         "say": f"typical swing for the genre {lo}-{hi}% (50 straight, 54 loose, 66 triplet) (#25)",
                         "source": "sourced: Linn swing scale; per-genre range heuristic", "rule": {"range": [lo, hi]}})
    drums = res.get("drums") or {}
    for lane, mask in (g.get("signature") or {}).items():
        d = drums.get(lane)
        if not d:
            continue
        want = [i for i, ch in enumerate(mask) if ch == "x"]
        share = float(np.mean([d["prob"][i] for i in want])) if want else 0.0
        findings.append({"metric": f"signature.{lane}", "value": round(share, 2), "status": "pass" if share >= 0.75 else "warn",
                         "say": f"the genre's signature {lane} hits ({mask}) are present in this share of bars (#27)",
                         "source": "sourced pattern; 75 % ratio heuristic", "rule": {"warn_below": 0.75}})
    order = {"fail": 0, "warn": 1, "pass": 2}
    findings.sort(key=lambda f: order[f["status"]])
    return findings


def calibrate(results, out_path=None):
    """Median, P10 and P90 of every judged metric over a set of renders the maker likes, per genre: the numbers to
    put into targets.json in place of heuristic thresholds."""
    keys = sorted({f["metric"] for r in results for f in r.get("findings", [])})
    table = {}
    for r in results:
        gen = r.get("genre") or "all"
        flat = {**r.get("audio", {}), **(r.get("midi") or {})}
        for k in keys:
            v = get(flat, k)
            if isinstance(v, list):
                v = max(v) if v else None
            if isinstance(v, (int, float)) and v is not None and math.isfinite(v):
                table.setdefault(gen, {}).setdefault(k, []).append(float(v))
    out = {g: {k: {"n": len(v), "p10": round(float(np.percentile(v, 10)), 3), "median": round(float(np.median(v)), 3),
                   "p90": round(float(np.percentile(v, 90)), 3)} for k, v in m.items()} for g, m in table.items()}
    if out_path:
        Path(out_path).write_text(json.dumps(out, indent=1), encoding="utf-8")
    return out


# ----------------------------------------------------------------------------------------------------------------
# Comparing two results (build A vs build B) and the null test
# ----------------------------------------------------------------------------------------------------------------
COMPARE_KEYS = ["loudness.integrated", "true_peak_dbtp", "psr_db", "plr_db", "loudness.lra", "spectrum.tilt_db_per_oct",
                "spectrum.centroid_hz", "spectrum.bands.sub_20_60", "spectrum.bands.low_60_250",
                "spectrum.bands.lowmid_250_2k", "spectrum.bands.presence_2k_8k", "spectrum.bands.air_8k_20k",
                "stereo.correlation", "stereo.side_to_mid_db", "artefacts.clicks", "artefacts.clip_runs",
                "drum_offset_sd_ms", "swing_pct", "in_scale_share", "groove_consistency"]


def null_test(a_path, b_path):
    a, sra = read_wav(a_path)
    b, srb = read_wav(b_path)
    if sra != srb or a.shape[1] != b.shape[1]:
        return {"comparable": False}
    n = min(len(a), len(b))
    a, b = a[:n].mean(axis=1), b[:n].mean(axis=1)
    if np.array_equal(a, b):
        return {"comparable": True, "bit_identical": True}
    m = min(n, sra * 10)
    xc = np.fft.irfft(np.fft.rfft(a[:m], 2 * m) * np.conj(np.fft.rfft(b[:m], 2 * m)), 2 * m)
    lag = int(np.argmax(np.abs(xc)))
    lag = lag - 2 * m if lag > m else lag
    if lag > 0:
        a2, b2 = a[lag:], b[: n - lag]
    else:
        a2, b2 = a[: n + lag], b[-lag:]
    g = float(np.dot(a2, b2) / (np.dot(b2, b2) + EPS))
    r = a2 - g * b2
    return {"comparable": True, "bit_identical": False, "lag_samples": lag, "gain_db": db(abs(g)),
            "residual_peak_dbfs": db(np.max(np.abs(r))), "null_depth_db": db(np.sqrt(np.mean(a2 ** 2))) - db(np.sqrt(np.mean(r ** 2)))}


def compare(a, b):
    out = []
    for k in COMPARE_KEYS:
        va, vb = get(a, k), get(b, k)
        if va is None or vb is None:
            continue
        d = vb - va
        if k.startswith("spectrum.bands."):
            rel = 10 * math.log10(max(vb, EPS) / max(va, EPS))
            if abs(rel) >= 1.0:
                out.append({"metric": k, "old": round(va, 4), "new": round(vb, 4), "change_db": round(rel, 2)})
        elif abs(d) >= {"loudness.integrated": 0.5, "true_peak_dbtp": 0.3, "spectrum.centroid_hz": 100}.get(k, 0.1 if "corr" in k else 0.5):
            out.append({"metric": k, "old": round(va, 3), "new": round(vb, 3), "change": round(d, 3)})
    return out


# ----------------------------------------------------------------------------------------------------------------
# Markdown report
# ----------------------------------------------------------------------------------------------------------------
def fmt(v):
    return "-" if v is None else (f"{v:.2f}" if isinstance(v, float) else str(v))


def report(results, out_path):
    L = ["# PERF sound check", ""]
    L.append("| Render | Genre | LUFS | dBTP | PSR | Tilt dB/oct | Corr | Clicks | Fails | Warns |")
    L.append("|---|---|---|---|---|---|---|---|---|---|")
    for r in results:
        a, f = r.get("audio", {}), r.get("findings", [])
        L.append(f"| {Path(a.get('file', r.get('midi', {}).get('file', '?'))).name} | {r.get('genre') or '-'} | "
                 f"{fmt(get(a, 'loudness.integrated'))} | {fmt(a.get('true_peak_dbtp'))} | {fmt(a.get('psr_db'))} | "
                 f"{fmt(get(a, 'spectrum.tilt_db_per_oct'))} | {fmt(get(a, 'stereo.correlation'))} | {fmt(get(a, 'artefacts.clicks'))} | "
                 f"{sum(x['status'] == 'fail' for x in f)} | {sum(x['status'] == 'warn' for x in f)} |")
    for r in results:
        name = Path(r.get("audio", {}).get("file", r.get("midi", {}).get("file", "?"))).name
        L += ["", f"## {name}" + (f" ({r['genre']})" if r.get("genre") else "")]
        for f in r.get("findings", []):
            if f["status"] != "pass":
                L.append(f"- **{f['status'].upper()}** `{f['metric']}` = {f['value']}: {f['say']} ({f['source']})")
        m = r.get("midi")
        if m:
            L.append(f"- MIDI: {m.get('bpm')} BPM, key {get(m, 'key_edma.key')} (EDM profile, margin {get(m, 'key_edma.margin')}), "
                     f"in scale {m.get('in_scale_share')}, swing {m.get('swing_pct')}%, groove consistency {m.get('groove_consistency')}, "
                     f"stuck notes {m.get('stuck_notes')}")
            if m.get("progression"):
                L.append(f"- Progression (one chord a bar): {' '.join(m['progression'][:16])}")
    Path(out_path).write_text("\n".join(L) + "\n", encoding="utf-8")


# ----------------------------------------------------------------------------------------------------------------
# Self-test: synthetic signals with known answers
# ----------------------------------------------------------------------------------------------------------------
def selftest():
    import tempfile
    sr, ok = 48000, True
    t = np.arange(sr * 20) / sr
    rng0 = np.random.default_rng(7)

    def check(name, cond, detail):
        nonlocal ok
        print(("PASS " if cond else "FAIL ") + name + ": " + detail)
        ok &= bool(cond)

    # EBU Tech 3341 case 1 scaled: a 1 kHz sine at -20 dBFS (peak) in both channels reads -20.0 LUFS (+-0.1)
    s = 0.1 * np.sin(2 * np.pi * 997 * t)
    x = np.stack([s, s], axis=1)
    L = loudness(x, sr)
    check("loudness 997 Hz -20 dBFS stereo (Tech 3341)", abs(L["integrated"] - (-20.0)) < 0.1, f"{L['integrated']:.2f} LUFS")
    check("LRA of a steady tone", L["lra"] is not None and L["lra"] < 0.1, f"{L['lra']}")
    tp = db(true_peak(x, sr))
    check("true peak of a sine", abs(tp - (-20)) < 0.1, f"{tp:.2f} dBTP")
    # inter-sample peak: fs/4 sine at 45 degrees phase: samples at 0.707, true peak 1.0 (+3 dB)
    q = np.sin(2 * np.pi * (sr / 4) * np.arange(sr) / sr + np.pi / 4) * 0.5
    tq = db(true_peak(np.stack([q, q], 1), sr)) - db(np.max(np.abs(q)))
    check("inter-sample overshoot", 2.5 < tq < 3.3, f"+{tq:.2f} dB over sample peak")
    st = stereo(x, sr)
    check("correlation of identical channels", st["correlation"] > 0.999, f"{st['correlation']:.4f}")
    xs = np.stack([s, -s], axis=1)
    check("correlation of inverted channels", stereo(xs, sr)["correlation"] < -0.999, "anti-phase")
    sp = spectrum(np.stack([0.3 * np.sin(2 * np.pi * 50 * t)] * 2, 1), sr)
    check("50 Hz tone lands in the sub band", sp["bands"]["sub_20_60"] > 0.95, f"{sp['bands']['sub_20_60']:.3f}")
    c = s.copy(); c[sr * 5] += 0.5
    a = artefacts(np.stack([c, c], 1), sr, 256)
    check("one click found", a["clicks"] == 1, f"{a['clicks']} at {a.get('click_times_s')}")
    d = np.clip(20 * s, -1, 1)  # a sine driven to twice full scale
    cr = artefacts(np.stack([d] * 2, 1), sr, 0)["clip_runs"]
    check("clipping runs found", cr > 10, f"{cr} runs in a clipped sine")
    check("no clicks in a clean sine", artefacts(x, sr, 256)["clicks"] == 0, "clean")
    hn = 0.3 * rng0.standard_normal(sr * 5)  # dense noise: no clicks either
    check("no clicks in noise", artefacts(np.stack([hn, hn], 1), sr, 256)["clicks"] == 0, "noise")
    # pink noise tilt ~ -3 dB/oct of 1/3-octave band power? pink = equal energy per octave -> 1/3-oct band power flat (0 dB/oct)
    rng = np.random.default_rng(1)
    w = rng.standard_normal(sr * 20)
    W = np.fft.rfft(w); f = np.fft.rfftfreq(len(w), 1 / sr); W[1:] /= np.sqrt(f[1:]); W[0] = 0
    pink = np.fft.irfft(W, len(w)); pink /= np.max(np.abs(pink)) * 4
    tl = spectrum(np.stack([pink, pink], 1), sr)["tilt_db_per_oct"]
    check("pink noise is flat per 1/3 octave", abs(tl) < 0.5, f"{tl:+.2f} dB/oct")
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "a.wav"
        write_wav(p, x, sr)
        y, sr2 = read_wav(p)
        check("WAV round trip", sr2 == sr and np.max(np.abs(y - x)) < 1e-4, f"{y.shape}")
        nt = null_test(p, p)
        check("null test of a file with itself", nt.get("bit_identical"), str(nt))
        # MIDI: 4 bars of 16th hats with 60 % swing at 120 BPM, kick on the beats, a C minor pad
        def vlq(n):
            b = [n & 0x7F]
            while n > 127:
                n >>= 7; b.insert(0, (n & 0x7F) | 0x80)
            return bytes(b)
        ppq, evs = 480, []
        for i in range(64):
            tk = i * 120 + (int(round((0.60 - 0.5) * 2 * 120)) if i % 2 else 0)
            evs += [(tk, bytes([0x99, 42, 90 if i % 2 == 0 else 40])), (tk + 30, bytes([0x89, 42, 0]))]
            if i % 4 == 0:
                evs += [(i * 120, bytes([0x99, 36, 120])), (i * 120 + 60, bytes([0x89, 36, 0]))]
        for bar in range(4):
            for p_ in (60, 63, 67):
                evs += [(bar * 1920, bytes([0x90, p_, 80])), (bar * 1920 + 1900, bytes([0x80, p_, 0]))]
        evs.sort(key=lambda e: e[0])
        trk, last = bytearray(b"\x00\xff\x51\x03" + (500000).to_bytes(3, "big")), 0
        for tk, msg in evs:
            trk += vlq(tk - last) + msg; last = tk
        trk += b"\x00\xff\x2f\x00"
        mp = Path(td) / "t.mid"
        mp.write_bytes(b"MThd" + struct.pack(">IHHH", 6, 0, 1, ppq) + b"MTrk" + struct.pack(">I", len(trk)) + bytes(trk))
        m = analyze_midi(mp)
        check("MIDI tempo", abs(m["bpm"] - 120) < 0.01, f"{m['bpm']}")
        check("swing measured", abs(m.get("swing_pct", 0) - 60) < 1.0, f"{m.get('swing_pct')}%")
        check("key C minor", get(m, "key_edma.key") == "C minor", f"{get(m, 'key_edma.key')}")
        check("progression i", m["progression"][:1] == ["i"], f"{m['progression']}")
        check("kick on the beats", m["drums"]["kick"]["prob"][0] == 1.0 and m["drums"]["kick"]["prob"][1] == 0, "kick grid")
        check("ghost hats", 0.4 < m["drums"]["hat_closed"]["ghost_share"] < 0.6, f"{m['drums']['hat_closed']['ghost_share']}")
    print("selftest:", "all passed" if ok else "FAILURES")
    return 0 if ok else 1


# ----------------------------------------------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    a = sub.add_parser("analyze"); a.add_argument("wav"); a.add_argument("--midi"); a.add_argument("--genre")
    a.add_argument("--bpm", type=float); a.add_argument("--block", type=int, default=256); a.add_argument("--json")
    a.add_argument("--targets")
    m = sub.add_parser("midi"); m.add_argument("mid"); m.add_argument("--bpm", type=float); m.add_argument("--json")
    c = sub.add_parser("compare"); c.add_argument("old"); c.add_argument("new"); c.add_argument("--json")
    r = sub.add_parser("report"); r.add_argument("folder"); r.add_argument("--out", default="soundcheck_report.md")
    k = sub.add_parser("calibrate"); k.add_argument("folder"); k.add_argument("--out", default="calibration.json")
    sub.add_parser("selftest")
    g = ap.parse_args()
    if g.cmd == "selftest":
        return selftest()
    if g.cmd == "calibrate":
        rs = [json.loads(p.read_text(encoding="utf-8")) for p in sorted(Path(g.folder).glob("*.json"))]
        out = calibrate(rs, g.out)
        print(f"calibration: {g.out} ({len(rs)} renders, {len(out)} genres)")
        return 0
    if g.cmd == "analyze":
        res = {"genre": g.genre, "audio": analyze_audio(g.wav, g.bpm, g.block)}
        if g.midi:
            res["midi"] = analyze_midi(g.midi, g.bpm)
        flat = {**res["audio"], **(res.get("midi") or {})}
        res["findings"] = judge(flat, g.genre, load_targets(g.targets))
    elif g.cmd == "midi":
        res = {"midi": analyze_midi(g.mid, g.bpm)}
    elif g.cmd == "compare":
        if g.old.lower().endswith(".wav") and g.new.lower().endswith(".wav"):
            A, B = {"audio": analyze_audio(g.old)}, {"audio": analyze_audio(g.new)}
            res = {"null_test": null_test(g.old, g.new)}
        else:
            A, B = json.loads(Path(g.old).read_text()), json.loads(Path(g.new).read_text())
            res = {}
        flatA = {**A.get("audio", {}), **(A.get("midi") or {})}
        flatB = {**B.get("audio", {}), **(B.get("midi") or {})}
        res["changes"] = compare(flatA, flatB)
    elif g.cmd == "report":
        rs = [json.loads(p.read_text(encoding="utf-8")) for p in sorted(Path(g.folder).glob("*.json"))]
        report(rs, g.out)
        print(f"report: {g.out} ({len(rs)} renders)")
        return 0
    text = json.dumps(res, indent=1, default=lambda o: o.tolist() if hasattr(o, "tolist") else str(o))
    if getattr(g, "json", None):
        Path(g.json).write_text(text, encoding="utf-8")
        print(f"wrote {g.json}")
    else:
        print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
