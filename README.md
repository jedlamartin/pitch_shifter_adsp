# ADSP-BF537 Pitch Shifter

## Overview

This project implements a **real-time pitch shifting** effect for the Analog Devices **ADSP-BF537 Blackfin DSP**. It processes audio via SPORT0/DMA, utilizing **fixed-point** implementation for performance.

The pitch shift relies on combining **Time Stretching** and **Resampling** methods.

## Algorithm Summary

| Technique | Function | Purpose |
| :--- | :--- | :--- |
| **Time Stretch** | OLA (`corr`, `apply_fade`) | Achieved by overlapping and adding audio blocks. **Cross-Correlation** (`corr`) finds the optimal phase alignment point, and a **linear cross-fade** (`apply_fade`) blends the segments. | 
| **Resampling** | Rate Conversion (`resample_spline`) | Interpolates or decimate the time-stretched signal to shift the pitch up or down. |

## Fixed-Point Optimization

All core processing is optimized for the Blackfin `int16_t` (`fract`) data type:

* **Q1.15 Format:** All I/O and interpolation factors are in Q1.15.
* **Precision:** Intermediate calculations use `int32_t` and 64-bit casting (`(int64_t)`) to maintain Q16.15 precision during multiplication.
* **Saturation:** All final results are explicitly **saturated/clipped** before being cast back to the 16-bit `fract` type to prevent audio distortion.

## Prerequisites

1.  Analog Devices **ADSP-BF537 EZ-KIT Lite** board.
2.  **CrossCore Embedded Studio** (CCES).
3.  JTAG emulator (e.g., ICE-1000/2000).
