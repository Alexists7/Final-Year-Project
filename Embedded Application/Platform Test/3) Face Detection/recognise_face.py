#! /usr/bin/python

import face_recognition
import cv2
import csv
import argparse
import sys

# Script that detects a face in an image and compares it to known faces, if person found, returns name else unknown

currentname = "unknown"
encodings_csv = "encodings.csv"

ap = argparse.ArgumentParser()
ap.add_argument("image", help="path to the input image")
args = ap.parse_args()

data = {"encodings": [], "names": []}

with open(encodings_csv, newline='') as csvfile:
    reader = csv.reader(csvfile)
    next(reader)
    for row in reader:
        name = row[0]
        try:
            encoding = list(map(float, row[1:]))
        except ValueError as e:
            print(f"[ERROR] Could not convert encodings for {name}: {e}")
            continue
        data["names"].append(name)
        data["encodings"].append(encoding)

image = cv2.imread(args.image)
if image is None:
    print("[ERROR] Could not read image from the provided path:", args.image)
    sys.exit(1)

# image = cv2.resize(image, (500, int(image.shape[0] * 500 / image.shape[1])))

boxes = face_recognition.face_locations(image)

encodings = face_recognition.face_encodings(image, boxes)

if not encodings:
    print("Unknown person - sound alarm!")
else:
    encoding = encodings[0]

    matches = face_recognition.compare_faces(data["encodings"], encoding)
    name = "Unknown"

    if True in matches:
        matchedIdxs = [i for (i, b) in enumerate(matches) if b]
        counts = {}

        for i in matchedIdxs:
            name = data["names"][i]
            counts[name] = counts.get(name, 0) + 1

        name = max(counts, key=counts.get)

    if name == "Unknown":
        print("Unknown person - sound alarm!")
    else:
        print(f"Person Detected - {name}")
