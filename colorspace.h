typedef struct {
  double h; // Hue (0-360 degrees)
  double s; // Saturation (0-1)
  double i; // Intensity (0-1)
} HSI;

typedef struct {
  unsigned int r; // Red (0-255)
  unsigned int g; // Green (0-255)
  unsigned int b; // Blue (0-255)
} RGB;

RGB HSItoRGB(HSI hsi);
HSI RGBtoHSI(RGB rgb);