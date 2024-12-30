#! /usr/bin/python

# Import the necessary packages
import face_recognition
import cv2
import csv
import argparse
import sys

# Initialize 'currentname' to trigger only when a new person is identified.
currentname = "unknown"
# Path to the encodings CSV file
encodings_csv = "encodings.csv"

# Construct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("image", help="path to the input image")
args = ap.parse_args()

# Load the known faces and embeddings from the CSV file
data = {"encodings": [], "names": []}

# Read the CSV file and load encodings and names
with open(encodings_csv, newline='') as csvfile:
    reader = csv.reader(csvfile)
    next(reader)  # Skip the header row
    for row in reader:
        # First column is the name, the rest are encodings
        name = row[0]
        try:
            encoding = list(map(float, row[1:]))  # Convert encoding back to float
        except ValueError as e:
            print(f"[ERROR] Could not convert encodings for {name}: {e}")
            continue  # Skip this row if there's an error
        data["names"].append(name)
        data["encodings"].append(encoding)

# Load the image from the provided path
image = cv2.imread(args.image)
if image is None:
    print("[ERROR] Could not read image from the provided path:", args.image)
    sys.exit(1)

# Resize the image to width of 500 pixels (optional, depending on image size)
# image = cv2.resize(image, (500, int(image.shape[0] * 500 / image.shape[1])))

# Detect the face locations in the image
boxes = face_recognition.face_locations(image)

# Compute the facial embeddings for each face bounding box
encodings = face_recognition.face_encodings(image, boxes)

if not encodings:
    print("Unknown person - sound alarm!")  # No encoding found
else:
    # Assume we are only processing one person in the image
    encoding = encodings[0]  # Taking only the first encoding

    # Attempt to match the face in the input image to our known encodings
    matches = face_recognition.compare_faces(data["encodings"], encoding)
    name = "Unknown"  # Default to Unknown if no match is found

    # Check if we found a match
    if True in matches:
        # Find the indexes of all matched faces and maintain a count for each recognized face
        matchedIdxs = [i for (i, b) in enumerate(matches) if b]
        counts = {}

        for i in matchedIdxs:
            name = data["names"][i]
            counts[name] = counts.get(name, 0) + 1

        # Determine the recognized face with the largest number of votes
        name = max(counts, key=counts.get)

    if name == "Unknown":
        print("Unknown person - sound alarm!")
    else:
        print(f"Person Detected - {name}")
