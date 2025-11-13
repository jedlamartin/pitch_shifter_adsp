# ADSP-BF537 Pitch Shifter

## Overview

This project implements a **real-time pitch shifting** effect for the Analog Devices **ADSP-BF537 Blackfin DSP**. It processes audio via SPORT0/DMA, utilizing **fixed-point** implementation for performance.

The pitch shift relies on combining **Time Stretching** and **Resampling** methods.

## Algorithm Summary

| Technique | Function | Purpose |
| :--- | :--- | :--- |
| **Time Stretch** | SOLA (`corr`, `apply_fade`) | Achieved by overlapping and adding audio blocks. **Cross-Correlation** (`corr`) finds the optimal phase alignment point, and a **linear cross-fade** (`apply_fade`) blends the segments. | 
| **Resampling** | Rate Conversion (`resample_spline`) | Interpolates or decimate the time-stretched signal to shift the pitch up or down. |

## Prerequisites

1.  Analog Devices **ADSP-BF537 EZ-KIT Lite** board.
2.  **CrossCore Embedded Studio** (CCES).
3.  JTAG emulator (e.g., ICE-1000/2000).
