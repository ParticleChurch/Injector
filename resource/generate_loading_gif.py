from scipy import ndimage
import numpy as np
import cv2
import shutil, os
from pathlib import Path

LOCAL_DIR = Path(__file__).parent

OUTSIZE = 512
INSIZE = OUTSIZE * 20 # fake antialias

img = np.zeros((INSIZE, INSIZE, 4), dtype = 'uint8')

MIDDLE = INSIZE >> 1
RADIUS = round(INSIZE * 0.15)
cv2.circle(img, (MIDDLE, MIDDLE), MIDDLE - (RADIUS >> 1), (255,255,255,255), RADIUS, lineType=cv2.LINE_AA)
cv2.rectangle(img, (0, 0), (MIDDLE, MIDDLE), (0,0,0,0), -1)
cv2.rectangle(img, (MIDDLE, MIDDLE), (INSIZE, INSIZE), (0,0,0,0), -1)

img = cv2.resize(img, (OUTSIZE, OUTSIZE), interpolation = cv2.INTER_AREA)

frames = [
    ndimage.rotate(img, rot, reshape=False)
    for rot in range(0, 180, 10)
]

os.chdir(LOCAL_DIR)
def try_delete(path):
    try:
        os.remove(path)
    except OSError as e:
        return e
    return None

for i, frame in enumerate(frames):
    cv2.imwrite(f'frame{i:04d}.png', frame)

try:
    os.system("ffmpeg -y -i frame%04d.png -vf palettegen=reserve_transparent=1 palette.png")
    os.system("ffmpeg -y -framerate 30 -i frame%04d.png -i palette.png -lavfi paletteuse=alpha_threshold=128 -gifflags -offsetting loading.gif")
finally:
    for i, frame in enumerate(frames):
            try_delete(f'frame{i:04d}.png')
    try_delete(f'palette.png')