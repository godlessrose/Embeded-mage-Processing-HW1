#Embedded Digital Image Processing HW1

**Members:**
| Name               | Student ID |
|--------------------|--------------|
| **Eren Karagül**   | 150722007     |
| **Mehmet Akif Takcı** | 150721058   |

---

## Project Description
This project demonstrates several fundamental grayscale image processing operations implemented on an embedded system (STM32F446RET6). The raw grayscale image is stored in memory, processed on the microcontroller, and the output buffers are made viewable through the memory window.

The following questions are implemented in STM32 CUBE IDE:

- **Storing an image:** We store an 64 by 64 image on the memory of stm32f4 board.
- **Negative Transformation:** Inverts pixel intensity as `255 - pixel`.
- **Thresholding:** Converts the image into a binary representation using a user-defined threshold.
- **Gamma Correction:** Uses lookup tables (LUTs) to apply nonlinear intensity adjustments for γ = 3 and γ = 1/3.
- **Piecewise Linear Transformation:** Adjusts image contrast by mapping pixel ranges across three linear segments.


---

## Answers

### Q2-a) Negative Transformation
<br/>
<div align="center">
  <img src="images\negative.png" width="80%">
</div>

### Q2-b) Thresholding
<br/>
<div align="center">
  <img src="images\threshold.png" width="80%">
</div>

### Q2-c) Gamma Correction (γ = 3)
<br/>

<div align="center">
  <img src="images\gamma3.png" width="80%">
</div>


### Q2-c. Gamma Correction (γ = 1/3)
<br/>
<div align="center">
  <img src="images\gamma033.png" width="80%">
</div>

### Q2-d) Piecewise Linear Transformation
<br/>
<div align="center">
  <img src="images\pwlt.png" width="80%">
</div>


