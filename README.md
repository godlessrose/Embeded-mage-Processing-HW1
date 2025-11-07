#Embedded Digital Image Processing HW1

### Group 6

**Members:**
| Name               | Student ID |
|--------------------|--------------|
| **Eren Karagül**   | 150722007     |
| **Mehmet Akif Takcı** | 150721058   |

---

## Project Description
This project demonstrates several fundamental grayscale image processing operations implemented on an embedded system (ARM Cortex-M MCU). The raw grayscale image is stored in memory, processed on the microcontroller, and the output buffers are made viewable through the memory window.

The following transformations are implemented in C:

- **Negative Transformation:** Inverts pixel intensity as `255 - pixel`.
- **Thresholding:** Converts the image into a binary representation using a user-defined threshold.
- **Gamma Correction:** Uses lookup tables (LUTs) to apply nonlinear intensity adjustments for γ = 3 and γ = 1/3.
- **Piecewise Linear Transformation:** Adjusts image contrast by mapping pixel ranges across three linear segments.


---

## Techniques Implemented

### 1. Gamma Correction
![Gamma 3](gamma3.png)

### 2. Gamma Correction
![Gamma 0.33](gamma033.png)

### 3. Negative Transformation
![Negative Image](negative.png)

### 4. Thresholding
![Threshold](threshold.png)

---

