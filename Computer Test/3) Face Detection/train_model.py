#! /usr/bin/python

# Import the necessary packages
from imutils import paths
import face_recognition
import cv2
import os
import csv

# Our images are located in the dataset folder
print("[INFO] start processing faces...")
imagePaths = list(paths.list_images("dataset"))

# Initialize the list of known encodings and known names
knownEncodings = []
knownNames = []

# Loop over the image paths
for (i, imagePath) in enumerate(imagePaths):
    # Extract the person name from the image path
    print("[INFO] processing image {}/{}".format(i + 1, len(imagePaths)))
    name = imagePath.split(os.path.sep)[-2]

    # Load the input image and convert it from RGB (OpenCV ordering) to dlib ordering (RGB)
    image = cv2.imread(imagePath)
    rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # Detect the (x, y)-coordinates of the bounding boxes corresponding to each face in the input image
    boxes = face_recognition.face_locations(rgb, model="hog")

    # Compute the facial embedding for the face
    encodings = face_recognition.face_encodings(rgb, boxes)

    # Loop over the encodings
    for encoding in encodings:
        # Add each encoding + name to our set of known names and encodings
        knownEncodings.append(encoding)
        knownNames.append(name)

# Dump the facial encodings + names to CSV
print("[INFO] serializing encodings...")
csv_file_path = "encodings.csv"

# Open the CSV file for writing
with open(csv_file_path, mode='w', newline='') as csv_file:
    # Create a CSV writer object
    csv_writer = csv.writer(csv_file)

    # Write the header
    header = ["name"] + [f"encoding_{i}" for i in range(len(knownEncodings[0]))]
    csv_writer.writerow(header)

    # Write the encodings and names to the CSV file
    for name, encoding in zip(knownNames, knownEncodings):
        csv_writer.writerow([name] + encoding.tolist())  # Convert the encoding array to a list

print("[INFO] Encodings saved to", csv_file_path)
