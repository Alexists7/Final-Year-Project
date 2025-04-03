#! /usr/bin/python

from imutils import paths
import face_recognition
import cv2
import os
import csv

# Script that processes the images in the dataset directory and creates a CSV file with the encodings of each face.

print("[INFO] start processing faces...")
imagePaths = list(paths.list_images("dataset"))

knownEncodings = []
knownNames = []

for (i, imagePath) in enumerate(imagePaths):
    print("[INFO] processing image {}/{}".format(i + 1, len(imagePaths)))
    name = imagePath.split(os.path.sep)[-2]

    image = cv2.imread(imagePath)
    rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    boxes = face_recognition.face_locations(rgb, model="hog")

    encodings = face_recognition.face_encodings(rgb, boxes)

    for encoding in encodings:
        knownEncodings.append(encoding)
        knownNames.append(name)

print("[INFO] serializing encodings...")
csv_file_path = "encodings.csv"

with open(csv_file_path, mode='w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)

    header = ["name"] + [f"encoding_{i}" for i in range(len(knownEncodings[0]))]
    csv_writer.writerow(header)

    for name, encoding in zip(knownNames, knownEncodings):
        csv_writer.writerow([name] + encoding.tolist())

print("[INFO] Encodings saved to", csv_file_path)
