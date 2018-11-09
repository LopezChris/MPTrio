# 146 MP3 Project

Folder structure for applications, we'll be pulling the drivers from `/firmware/lib/L2_Drivers`.

~~~bash
firmware/mp3_player
├── controls
├── decoder
│   └── VS1053_decoder
├── display
│   └── nokia_5110_display
└── file_sys
~~~

## Parts used

1. [LCD Screen](https://www.sparkfun.com/products/10168)
    - [Data sheet](https://www.sparkfun.com/datasheets/LCD/Monochrome/Nokia5110.pdf)
2. [Decoder](https://www.adafruit.com/product/1381)
    - [Library for application](https://github.com/LopezChris/Adafruit_VS1053_Library)
