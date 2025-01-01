#! /usr/bin/python3

import struct
import os
import sys

def put16le(value):
	return struct.pack('<H', value & 0xFFFF)

def put32le(value):
	return struct.pack('<I', value & 0xFFFFFFFF)

def write8(file, value):
	file.write(bytes([value & 0xFF]))

def write16le(file, value):
	file.write(put16le(value))

def write32le(file, value):
	file.write(put32le(value))

class ImageDirectory:
	def __init__(self, type):
		self.type = type
		self.images = []
		self.discardable = False

class Image:
	ICON = 1
	CURSOR = 2
	BITMAP = 3
	def __init__(self, imgdir, width, height, bpp):
		self.imgdir = imgdir
		imgdir.images.append(self)
		self.width = width
		self.height = height
		self.palette = [(0, 0, 0, 0)] * (1 << bpp) # Win3+ only
		self.bits_per_pixel = bpp # Win3+ only
		self.device_independent = True # Win1/2 only
		self.hotspot = (width // 2, height // 2) # cursors only, Win1/2 icons
		self.rowsize = (width + 7) // 8 # Win1/2 only
		self.mask = [[1] * width for i in range(height)]
		self.pixels = [[0] * width for i in range(height)]
		self.color_planes = 1 # should not be changed, Win3+ only

def write_win1(file, imgdir):
	write8(file, imgdir.type)
	devind = 0
	for img in imgdir.images:
		if img.device_independent:
			devind |= 2
		else:
			devind |= 1
	devind -= 1
	if imgdir.discardable:
		devind |= 0x80
	write8(file, devind)
	for img in sorted(imgdir.images, key = lambda img: not img.device_independent):
		if imgdir.type != Image.BITMAP:
			write16le(file, img.hotspot[0])
			write16le(file, img.hotspot[1])
		else:
			file.write(b'\0\0')
		write16le(file, img.width)
		write16le(file, img.height)
		write16le(file, img.rowsize)
		if imgdir.type == Image.BITMAP:
			file.write(b'\1\1\0\0')
		file.write(b'\0\0')
		if imgdir.type != Image.BITMAP:
			for row in img.mask:
				for i in range(img.rowsize):
					b = 0
					for j in range(8):
						if i * 8 + j >= img.width:
							pixel = 1
						else:
							pixel = row[i * 8 + j]
						b |= pixel << (7 - j)
					file.write(bytes([b]))
		for row in img.pixels:
			for i in range(img.rowsize):
				b = 0
				for j in range(8):
					if i * 8 + j >= img.width:
						pixel = 1
					else:
						pixel = row[i * 8 + j]
					b |= pixel << (7 - j)
				file.write(bytes([b]))

def write_win3(file, imgdir):
	image_sizes = []
	if imgdir.type == Image.BITMAP:
		file.write(b'BM')
		image_size = 0 # TODO
		write32le(file, image_size)
		file.write(b'\0\0\0\0')
		offset = 14 + 40
		write32le(file, offset)
		image_sizes.append(image_size)
	else:
		file.write(b'\0\0')
		write16le(file, imgdir.type)
		write16le(file, len(imgdir.images))
		offset = 6 + 16 * len(imgdir.images)
		image_sizes = []
		for img in imgdir.images:
			write8(file, img.width)
			write8(file, img.height)
			write8(file, len(img.palette))
			file.write(b'\0')
			if imgdir.type == Image.ICON:
				write16le(file, img.color_planes) # should be 1
				write16le(file, img.bits_per_pixel)
			else:
				write16le(file, img.hotspot[0])
				write16le(file, img.hotspot[1])
			image_size = 40 + 4 * len(img.palette) + (img.bits_per_pixel * img.width + 31) // 32 * 4 * img.height + (img.width + 31) // 32 * 4 * img.height
			write32le(file, image_size)
			write32le(file, offset)
			offset += image_size
			image_sizes.append(image_size)

	for img, image_size in zip(imgdir.images, image_sizes):
		write32le(file, 40) # header size
		write32le(file, img.width)
		write32le(file, img.height * 2)
		write16le(file, img.color_planes) # should be 1
		write16le(file, img.bits_per_pixel)
		write32le(file, 0) # no compression
		write32le(file, image_size)
		write32le(file, 0) # H resolution
		write32le(file, 0) # V resolution
		write32le(file, len(img.palette))
		write32le(file, 0) # important colors; ignored

		for r, g, b, a in img.palette:
			file.write(bytes([b, g, r, a]))

		pixels_per_byte = 8 // img.bits_per_pixel
		rowsize = (img.bits_per_pixel * img.width + 31) // 32 * 4
		#print(rowsize)
		for row in reversed(img.pixels):
			for i in range(rowsize):
				b = 0
				for j in range(pixels_per_byte):
					if i * pixels_per_byte + j >= img.width:
						pixel = 0
					else:
						pixel = row[i * pixels_per_byte + j]
					b |= pixel << (8 - img.bits_per_pixel - j * img.bits_per_pixel)
				file.write(bytes([b]))

		if imgdir.type != Image.BITMAP:
			rowsize = (img.width + 31) // 32 * 4
			for row in reversed(img.mask):
				for i in range(rowsize):
					b = 0
					for j in range(8):
						if i * 8 + j >= img.width:
							pixel = 1
						else:
							pixel = row[i * 8 + j]
						b |= pixel << (7 - j)
					file.write(bytes([b]))

ICON = """
...............FF...............
..............FFEF..............
.............FFFEEF.............
............FFFFEEEF............
...........FFFFFEEEEF...........
..........FFFFFFEEEEEF..........
.........FFFFFFFEEEEEEF.........
........FFFFFFFFEEEEEEEF........
.......FFFFFFFFFEEEEEEEEF.......
......FFFFFFFFFFEEEEEEEEEF......
.....FFFFFFFFFFFEEEEEEEEEEF.....
....FFFFFFFFFFFFEEEEEEEEEEEF....
...FFFFFFFFFFFFFEEEEEEEEEEEEF...
..FFFFFFFFFFFFFFEEEEEEEEEEEEEF..
.FFFFFFFFFFFFFFFEEEEEEEEEEEEEEF.
FFFFFFFFFFFFFFFFEEEEEEEEEEEEEEEF
FCCCCCCCCCCCCCCC^^^^^^^^^^^^^^^F
.FCCCCCCCCCCCCCC^^^^^^^^^^^^^^F.
..FCCCCCCCCCCCCC^^^^^^^^^^^^^F..
...FCCCCCCCCCCCC^^^^^^^^^^^^F...
....FCCCCCCCCCCC^^^^^^^^^^^F....
.....FCCCCCCCCCC^^^^^^^^^^F.....
......FCCCCCCCCC^^^^^^^^^F......
.......FCCCCCCCC^^^^^^^^F.......
........FCCCCCCC^^^^^^^F........
.........FCCCCCC^^^^^^F.........
..........FCCCCC^^^^^F..........
...........FCCCC^^^^F...........
............FCCC^^^F............
.............FCC^^F.............
..............FC^F..............
...............FF...............
"""

def generate_ico1(data):
	imgdir = ImageDirectory(Image.ICON)
	img = Image(imgdir, 64, 64, 1)
	for y, line in enumerate(line for line in data.split('\n') if line != ''):
		for x, c in enumerate(line):
			if c == '.':
				m = 1
				p = 0
			elif c == '^':
				m = 1
				p = 1
			else:
				c = int(c, 16)
				m = 0
				p = 1 if c == 15 else 0
			for i in range(2):
				for j in range(2):
					img.mask[2 * y + i][2 * x + j] = m
					img.pixels[2 * y + i][2 * x + j] = p
	return imgdir

win2 = [(0,0,0),(255,255,255)]
win16 = [(0,0,0),(128,0,0),(0,128,0),(128,128,0),(0,0,128),(128,0,128),(0,128,128),(192,192,192),(128,128,128),(255,0,0),(0,255,0),(255,255,0),(0,0,255),(255,0,255),(0,255,255),(255,255,255)]

def generate_ico3(data):
	imgdir = ImageDirectory(Image.ICON)
	img1 = Image(imgdir, 32, 32, 1)
	for i in range(2):
		img1.palette[i] = win2[i] + (0,)
	img2 = Image(imgdir, 32, 32, 4)
	for i in range(16):
		img2.palette[i] = win16[i] + (0,)
	for y, line in enumerate(line for line in data.split('\n') if line != ''):
		for x, c in enumerate(line):
			if c == '.':
				img1.mask[y][x] = 1
				img1.pixels[y][x] = 0

				img2.mask[y][x] = 1
				img2.pixels[y][x] = 0
			elif c == '^':
				img1.mask[y][x] = 1
				img1.pixels[y][x] = 1

				img2.mask[y][x] = 1
				img2.pixels[y][x] = 0xF
			else:
				c = int(c, 16)
				img1.mask[y][x] = 0
				img1.pixels[y][x] = 1 if c == 15 else 0

				img2.mask[y][x] = 0
				img2.pixels[y][x] = c
	return imgdir

def generate_ico4(data):
	imgdir = ImageDirectory(Image.ICON)
	img1 = Image(imgdir, 32, 32, 4)
	for i in range(16):
		img1.palette[i] = win16[i] + (0,)
	img2 = Image(imgdir, 16, 16, 4)
	for i in range(16):
		img2.palette[i] = win16[i] + (0,)
	for y, line in enumerate(line for line in data.split('\n') if line != ''):
		for x, c in enumerate(line):
			if c == '.':
				img1.mask[y][x] = 1
				img1.pixels[y][x] = 0
			elif c == '^':
				img1.mask[y][x] = 1
				img1.pixels[y][x] = 0xF
			else:
				c = int(c, 16)
				img1.mask[y][x] = 0
				img1.pixels[y][x] = c
	for y in range(0, 32, 2):
		for x in range(0, 32, 2):
			vset = {}
			for j in range(2):
				for i in range(2):
					v = (img1.mask[y + j][x + i], img1.pixels[y + j][x + i])
					if v not in vset:
						vset[v] = 1
					else:
						vset[v] += 1
			(m, p) = max(vset.keys(), key = lambda k: vset[k])
			img2.mask[y // 2][x // 2] = m
			img2.pixels[y // 2][x // 2] = p
	return imgdir

win1ico = generate_ico1(ICON)
win3ico = generate_ico3(ICON)
win4ico = generate_ico4(ICON)

with open('win1.ico', 'wb') as file:
	write_win1(file, win1ico)

with open('win3.ico', 'wb') as file:
	write_win3(file, win3ico)

with open('win4.ico', 'wb') as file:
	write_win3(file, win4ico)

